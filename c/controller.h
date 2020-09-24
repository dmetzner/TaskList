#ifndef PROJECT_CONTROLLER_H
#define PROJECT_CONTROLLER_H

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
#include "return_t.h"

return_t handleError(int c_fd, const char *msg);

return_t handleTagNotFound(int c_fd);

return_t handleTaskNotFound(int c_fd);

return_t showRoot(int c_fd);

return_t methodNotAllowed(int c_fd);

return_t urlNotFound(int c_fd);

return_t handleInvalidRequest(int c_fd);

return_t handleBadTags(int c_fd);

return_t deleteEntry(int c_fd, sqlite3 *db, size_t id, char *table, return_t (*idNotFound_functionPtr)(int));

return_t getEntry(sqlite3 *db, size_t id, char *table, char **zErrMsg, cJSON **resp);

return_t getAllEntries(sqlite3 *db, char *table, char **zErrMsg, cJSON **resp);


// "public" callbacks

int checkIfDataExistsCallback(void *data, int argc, char **argv, char **azColName);

int getEntryCallback(void *data, int argc, char **argv, char **azColName);

int getAllEntriesCallback(void *data, int argc, char **argv, char **azColName);


#endif //PROJECT_CONTROLLER_H
