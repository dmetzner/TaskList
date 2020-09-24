#include "responses.h"

const char* getStatusName(size_t status_code) {
    switch (status_code) {

        case STATUS_SUCCESSFUL:
            return SUCCESSFUL;

        case STATUS_SUCCESSFULLY_CREATED:
            return CREATED;

        case STATUS_SUCCESSFULLY_DELETED:
            return NO_CONTENT;

        case STATUS_NOT_FOUND:
            return NOT_FOUND;

        case STATUS_METHOD_NOT_ALLOWED:
            return METHOD_NOT_ALLOWED;

        case STATUS_INVALID_REQUEST_DATA:
            return INVALID_REQUEST_DATA;

        default:
            return "Internal Server Error";
    }
}

cJSON* createErrorResponse(cJSON *resp, const char * const data[], int len) {

    if (resp) {
        cJSON_Delete(resp);
    }
    resp = cJSON_CreateObject();
    cJSON *msg = cJSON_CreateStringArray((const char **) (data), len);
    cJSON_AddItemToObject(resp, MESSAGES, msg);

    return resp;
}

return_t sendResponse(int c_fd, size_t status_code, char *body) {

    const char *status_name = getStatusName(status_code);

    char *response = NULL;

    if (STATUS_SUCCESSFULLY_DELETED == status_code) {
        // No body stuff in delete
        asprintf(&response, "HTTP/1.0 %zu %s\n\n", status_code, status_name);
    }
    else {
        asprintf(&response, "HTTP/1.0 %zu %s\n" \
                        "Content-Length: %zu\n" \
                        "Content-Type: application/json; charset=utf-8\n" \
                        "\n" \
                        "%s", status_code, status_name, strlen(body), body);
    }


    if (!response) {
        return RET_OOM;
    }
    send(c_fd, response, strlen(response), 0);

    // clean up
    free(response);
    return RET_OK;
}


return_t sendErrorResponse(int c_fd, size_t status_code, const char *const data[], int data_len) {

    // Free me
    cJSON *resp = NULL;

    resp = createErrorResponse(resp, data, data_len);
    sendJsonResponse(c_fd, status_code, resp);

    // clean_up
    cJSON_Delete(resp);

    return RET_OK;
}

return_t sendJsonResponse(int c_fd, size_t status_code, cJSON *json) {

    // Free me
    char *response = NULL;

    response = cJSON_Print(json);
    sendResponse(c_fd, status_code, response);

    // clean_up
    free(response);

    return RET_OK;
}

return_t sendHtmlResponse(int c_fd, size_t status_code, char *body) {

    const char *status_name = getStatusName(status_code);

    char *response = NULL;
    asprintf(&response, "HTTP/1.0 %zu %s\n" \
                        "Content-Length: %zu\n" \
                        "Content-Type: text/html; charset=utf-8\n" \
                        "\n" \
                        "%s", status_code, status_name, strlen(body), body);
    if (!response) {
        return RET_OOM;
    }
    send(c_fd, response, strlen(response), 0);

    // clean up
    free(response);
    return RET_OK;
}

