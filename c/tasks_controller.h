#ifndef C_TASKS_CONTROLLER_H
#define C_TASKS_CONTROLLER_H

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

void showAllTasks(int c_fd, sqlite3 *db);

void addTask(int c_fd, sqlite3 *db, cJSON *params);

void showTask(int c_fd, sqlite3 *db, size_t id);

void updateTask(int c_fd, sqlite3 *db, size_t id, cJSON *params);

void deleteTask(int c_fd, sqlite3 *db, size_t id);

#endif //C_TASKS_CONTROLLER_H
