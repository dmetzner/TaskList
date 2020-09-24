#ifndef C_REQUEST_T_H
#define C_REQUEST_T_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "return_t.h"
#include "request.h"
#include "cJSON.h"
#include <errno.h>


typedef struct {
    char *method;
    char *path;
    size_t path_parts;
    size_t id;
    size_t content_length;
    char *content_type;
    cJSON *params;
} request_t;

void request_free(request_t **request);

return_t request_parse(int fd, request_t **request);

#endif //C_REQUEST_T_H
