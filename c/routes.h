#ifndef C_ROUTES_H
#define C_ROUTES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sqlite3.h>
#include "controller.h"
#include "tasks_controller.h"
#include "tags_controller.h"
#include "request.h"

void handleRouting(int c_fd, sqlite3 *db, request_t *request);

#endif //C_ROUTES_H
