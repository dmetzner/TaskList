#include <sys/poll.h>
#include "request.h"

void request_free(request_t **request) {
    if (request && *request) {
        if ((*request)->method) {
            free((*request)->method);
            (*request)->method = NULL; /* clear to avoid double free */
        }
        if ((*request)->path) {
            free((*request)->path);
            (*request)->path = NULL; /* clear to avoid double free */
        }
        if ((*request)->params) {
            cJSON_Delete((*request)->params);
            (*request)->params = NULL; /* clear to avoid double free */
        }
        if ((*request)->content_type) {
            free((*request)->content_type);
            (*request)->content_type = NULL; /* clear to avoid double free */
        }
        free(*request);
        *request = NULL; /* clear to avoid double free */
    }
}

return_t recvWrapper(int server_fd, char *data) {
    struct pollfd fd;
    fd.fd = server_fd;
    fd.events = POLLIN;
                                    // 15 second timeout
    switch (poll(&fd, 1, 15000)) {
        case -1:
            return RET_FAIL;
        case 0:
            return RET_TIME_OUT;
        default:
            if (1 != recv(server_fd, data, 1, 0)) {
                return  RET_FAIL;
            }
            return RET_OK;
    }
}


return_t recvHttpHeader(int server_fd, char *recv_buffer, size_t size) {

    return_t ret = RET_FAIL;

    size_t i = 0;
    for (i = 0; i < size - 1; i++) {

        if (RET_OK != (ret = recvWrapper(server_fd, &recv_buffer[i]))) {
            break;
        }

        if (i >= 3 && '\n' == recv_buffer[i] && '\r' == recv_buffer[i - 1] &&
            '\n' == recv_buffer[i - 2] && '\r' == recv_buffer[i - 3]) {
            recv_buffer[i] = '\0';
            recv_buffer[i - 1] = '\0';
            recv_buffer[i - 2] = '\0';
            recv_buffer[i - 3] = '\0';
            ret = RET_OK;
            break;
        }
        if (i >= 1 && '\n' == recv_buffer[i] && '\n' == recv_buffer[i - 1]) {
            recv_buffer[i] = '\0';
            recv_buffer[i - 1] = '\0';
            ret = RET_OK;
            break;
        }
    }

    return ret;
}

return_t recvHttpBody(int server_fd, char *recv_buffer, size_t size) {

    return_t ret = RET_OK;

    size_t i = 0;
    for (i = 0; i < size - 1; i++) {
        if (RET_OK != (ret = recvWrapper(server_fd, &recv_buffer[i]))) {
            break;
        }
    }
    recv_buffer[i] = '\0';

    return ret;
}


return_t parseMethodPath(request_t **request, char *method_path) {

    // method path must contain: "method, path, http_version"
    char *method = strtok(method_path, " ");
    char *path = strtok(NULL, " ");
    char *http = strtok(NULL, " ");
    if (!method || !path || !http) {
        return RET_PARSER;
    }

    // case sensitive!
    if (strncmp(http, "HTTP/", 5) != 0) {
        return RET_PARSER;
    }

    // here we split the paths and also save the id to allow easy routing
    // this can be done because all our supported routes are either "/xy" or "/xy/:id"
    size_t i;
    for (i = 0; i < strlen(path); i++) {
        if ((path[i]) == '/') {
            (*request)->path_parts++;
        }
    }
    if ((*request)->path_parts == 2) {
        char *path_cpy = strdup(path);
        strtok(path_cpy, "/");
        char *id = strtok(NULL, "/");
        if (id != NULL) {
            /* reset errno to 0 before call */
            errno = 0;
            const char *string_to_read = id;
            char *endptr = NULL;
            long number = strtol(string_to_read, &endptr, 10);
            if (errno == 0 && !*endptr && number > 0 && number < INT32_MAX) {
                // valid  (and represents all characters read
                (*request)->id = (size_t) number;
            }
            else {
                // set path to invalid
                (*request)->id = 0;
            }
        }
        free(path_cpy);
    }
    // we have to copy the string into the request struct
    (*request)->method = strdup(method);
    (*request)->path = strdup(path);
    if (!(*request)->path || !(*request)->method) {
        return RET_OOM;
    }
    return RET_OK;
}

return_t parseContent(request_t **request, char *content_length_line, char *content_type_line, char *cl, char *ct) {

    char *content_length = NULL;

    if (content_length_line) {
        content_length = content_length_line + strlen(cl);

        // trim whitespace
        while (content_length != NULL && content_length[0] == ' ') {
            content_length++;
        }
    }
    if (content_length) {

        char *content_type = NULL;

        /* reset errno to 0 before call */
        errno = 0;
        const char *string_to_read = content_length;
        char *endptr = NULL;
        long number = strtol(string_to_read, &endptr, 10);

        if (errno == 0 && !*endptr && number > 0 && number < INT32_MAX) {
            // valid  (and represents all characters read
            (*request)->content_length = (size_t) number;
        }
        else {
            return RET_FAIL;
        }

        // type only needed when their is a length!
        if (content_type_line) {
            content_type = content_type_line + strlen(ct);

            // trim whitespace
            while (content_type != NULL && content_type[0] == ' ') {
                content_type++;
            }
        }

        if (content_type) {
            (*request)->content_type = strdup(content_type);
            if (!(*request)->content_type) {
                return RET_OOM;
            }
        }
    }

    return RET_OK;

}


return_t request_parse(int fd, request_t **request) {

    return_t ret;

    // Invalid request
    if (NULL == request) {
        return RET_INV;
    }

    // create and set all heap data structure we need to NULL
    *request = calloc(1, sizeof(request_t));
    if (NULL == *request) {
        return RET_OOM;
    }

    // Free ME!
    char *header_buffer = NULL;
    char *method_path = NULL;
    char *content_length_line = NULL;
    char *content_type_line = NULL;
    char *body_buffer = NULL;

    // Let read the HTTP Request and parse it:

    // create buffer for the header
    const size_t buffer_size = 8192;
    header_buffer = calloc(1, buffer_size + 1);
    if (!header_buffer) {
        ret = RET_OOM;
        goto cleanup;
    }

    // read the HTTP header and store it in the buffer
    if (RET_OK != (ret = recvHttpHeader(fd, header_buffer, buffer_size))) {
        goto cleanup;
    }

    // parse the header
    char delimiter[] = "\r\n";
    // the first line has to be the method path -> ex.: GET /tasks HTTP/1.0
    char *header_ptr = strtok(header_buffer, delimiter);
    if (NULL == header_ptr) {
        ret = RET_INV;
        goto cleanup;
    }
    method_path = strdup(header_ptr);
    if (NULL == method_path) {
        ret = RET_OOM;
        goto cleanup;
    }

    char *cl = "Content-Length:";
    char *ct = "Content-Type:";
    // the next lines can be arbitrary header stuff -> lets just search for the stuff we could need
    while(NULL != header_ptr) {
        // we need the content length to recv the body (if there is one!)
        if (strlen(header_ptr) > strlen(cl) &&
            // content length
            strncasecmp(header_ptr, cl, strlen(cl)) == 0) {
            content_length_line = strdup(header_ptr);
            if (NULL == content_length_line) {
                ret = RET_OOM;
                goto cleanup;
            }
        }
        if (strlen(header_ptr) > strlen(ct) && strncasecmp(header_ptr, ct, strlen(ct)) == 0) {
            // content type
            content_type_line = strdup(header_ptr);
            if (NULL == content_type_line) {
                ret = RET_OOM;
                goto cleanup;
            }
        }
        header_ptr = strtok(NULL, delimiter);
    }

    if (RET_OK != (ret = parseMethodPath(request, method_path))) {
        goto cleanup;
    }

    printf("DEBUG > Method: %s, path: %s\n", (*request)->method, (*request)->path);


    char *post = "post";
    char *put = "put";
    // only interested in body if we have a POST or PUT request!
    if ((strlen((*request)->method) >= strlen(post) && strncasecmp((*request)->method, post, strlen(post)) == 0) ||
            (strlen((*request)->method) >= strlen(put) && strncasecmp((*request)->method, put, strlen(put)) == 0)) {
        if (RET_OK != (ret = parseContent(request, content_length_line, content_type_line, cl, ct))) {
            goto cleanup;
        }
    }


    // if there is a request body
    char *app_json = "application/json";
    if ((*request)->content_length > 0 && (*request)->content_type &&
        strlen((*request)->content_type) >= strlen(app_json) &&
        strncasecmp((*request)->content_type, app_json, strlen(app_json)) == 0) {

        // create buffer
        body_buffer = calloc(1, (*request)->content_length + 1);
        if (!body_buffer) {
            ret = RET_OOM;
            goto cleanup;
        }
        // read the HTTP body and store it in the buffer
        if (RET_OK != (ret = recvHttpBody(fd, body_buffer, (*request)->content_length + 1))) {
            goto cleanup;
        }

        // parse json body
        ((*request)->params) = cJSON_Parse(body_buffer);
        if ( !((*request)->params) ) {
            ret = RET_INV;
            goto cleanup;
        }
    }
    else  {
        if (strlen((*request)->method) >= strlen(post) &&
                 strncasecmp((*request)->method, post, strlen(post)) == 0) {
            // post needs body!
            ret = RET_INV;
            goto cleanup;
        } else if (strlen((*request)->method) >= strlen(put) &&
                   strncasecmp((*request)->method, put, strlen(put)) == 0) {
            // put needs body!
            ret = RET_INV;
            goto cleanup;
        }
        // else we ignore the fact that the body is incorrect since we don't need the body
    }

    cleanup:
    free(header_buffer);
    free(method_path);
    free(content_length_line);
    free(content_type_line);
    free(body_buffer);
    if (RET_OK != ret) {
        request_free(request);
    }
    return ret;
}