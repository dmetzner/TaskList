#ifndef PROJECT_RESPONSES_H
#define PROJECT_RESPONSES_H

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "cJSON.h"
#include "return_t.h"

return_t sendResponse(int c_fd, size_t status_code, char *body);

return_t sendErrorResponse(int c_fd, size_t status_code, const char *const data[], int data_len);

return_t sendJsonResponse(int c_fd, size_t status_code, cJSON *json);

return_t sendHtmlResponse(int c_fd, size_t status_code, char *body);

cJSON* createErrorResponse(cJSON *resp, const char * const data[], int len);

static const char MESSAGES[] = "messages";

static const char INVALID_REQUEST[] = "Invalid Request";
static const char TIMEOUT[] = "Request timeout";
static const char WELL_FORMED[] = "The request was well-formed but was unable to be followed due to semantic errors.";
static const char BAD_TAGS[] =  "Invalid escape characters in tags";

static const char OOM[] = "Server out of memory";

static const char TASK_ID_NOT_FOUND[] = "Task ID not found";
static const char TAG_ID_NOT_FOUND[] = "Tag ID not found";

static const char URL_NOT_FOUND[] = "The requested URL was not found on the server.  " \
                                    "If you entered the URL manually please check your spelling and try again.";

static const char METHOD_NOT_ALLOWED_URL[] = "The method is not allowed for the requested URL.";

static const char ERROR[] = "Error";

// Status Messages
static const char SUCCESSFUL[] = "OK";
static const char CREATED[] = "CREATED";
static const char NO_CONTENT[] = "NO CONTENT";
static const char NOT_FOUND[] = "Not Found";
static const char METHOD_NOT_ALLOWED[] = "Method Not Allowed";
static const char INVALID_REQUEST_DATA[] = "Unprocessable Entity";

// Status codes
#define STATUS_SUCCESSFUL 200
#define STATUS_SUCCESSFULLY_CREATED 201
#define STATUS_SUCCESSFULLY_DELETED 204
#define STATUS_NOT_FOUND 404
#define STATUS_METHOD_NOT_ALLOWED 405
#define STATUS_INVALID_REQUEST_DATA 422
#define STATUS_ERROR 500


#endif //PROJECT_RESPONSES_H
