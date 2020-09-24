#ifndef C_TAGS_CONTROLLER_H
#define C_TAGS_CONTROLLER_H

#define _GNU_SOURCE

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "request.h"
#include <sys/types.h>
#include <sys/socket.h>
#include "cJSON.h"
#include "responses.h"
#include "utils.h"
#include "controller.h"
#include "return_t.h"
#include "validator.h"

void showAllTags(int c_fd, sqlite3 *db);

void showTag(int c_fd, sqlite3 *db, size_t id);

void updateTag(int c_fd, sqlite3 *db, size_t id, cJSON *params);

void deleteTag(int c_fd, sqlite3 *db, size_t id);

#endif //C_TAGS_CONTROLLER_H
