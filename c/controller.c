#include "controller.h"

// Callbacks

int checkIfDataExistsCallback(void *data, int argc, char **argv, char **azColName) {

    (*(void **) data) = cJSON_CreateArray();

    return 0;
}

int getEntryCallback(void *data, int argc, char **argv, char **azColName) {

    (*(void **) data) = cJSON_CreateObject();

    for (int i = 0; i < argc; i++) {
        if (strlen(azColName[i]) == strlen("id") && !strcmp(azColName[i], "id")) {
            cJSON_AddNumberToObject((*(void **) data), azColName[i], strtol(argv[i], 0, 10));
        } else {
            cJSON_AddStringToObject((*(void **) data), azColName[i], argv[i] ? argv[i] : "NULL");
        }
    }

    return 0;
}

int getAllEntriesCallback(void *data, int argc, char **argv, char **azColName) {

    cJSON *tags = (cJSON *) data;
    cJSON *tag = cJSON_CreateObject();

    cJSON_AddItemToArray(tags, tag);

    for (int i = 0; i < argc; i++) {
        if (strlen(azColName[i]) == strlen("id") && !strcmp(azColName[i], "id")) {
            cJSON_AddNumberToObject(tag, azColName[i], strtol(argv[i], 0, 10));
        } else {
            cJSON_AddStringToObject(tag, azColName[i], argv[i] ? argv[i] : "NULL");
        }
    }

    return 0;
}

//

return_t showRoot(int c_fd) {

    // Free ME
    char *file_buffer = NULL;

    return_t ret;

    if (RET_OK != readFile("index.html", &file_buffer)) {
        const char * const data[] = {ERROR};
        ret = sendErrorResponse(c_fd, STATUS_ERROR, data, 1);
    }
    else {
        ret = sendHtmlResponse(c_fd, STATUS_SUCCESSFUL, file_buffer);
    }

    // clean up
    free(file_buffer);

    return ret;
}

return_t urlNotFound(int c_fd) {

    const char *const data[] = {NOT_FOUND, URL_NOT_FOUND};
    return sendErrorResponse(c_fd, STATUS_NOT_FOUND, data, 2);
}

return_t methodNotAllowed(int c_fd) {

    const char *const data[] = {METHOD_NOT_ALLOWED, METHOD_NOT_ALLOWED_URL};
    return sendErrorResponse(c_fd, STATUS_METHOD_NOT_ALLOWED, data, 2);
}

return_t handleError(int c_fd, const char *msg) {

    if (msg) {
        const char *const data[] = {ERROR, msg};
        return sendErrorResponse(c_fd, STATUS_INVALID_REQUEST_DATA, data, 2);
    }
    const char *const data[] = {ERROR};
    return sendErrorResponse(c_fd, STATUS_INVALID_REQUEST_DATA, data, 1);
}

return_t handleTagNotFound(int c_fd) {

    const char *const data[] = {NOT_FOUND, TAG_ID_NOT_FOUND};
    return sendErrorResponse(c_fd, STATUS_NOT_FOUND, data, 2);
}

return_t handleTaskNotFound(int c_fd) {

    const char *const data[] = {NOT_FOUND, TASK_ID_NOT_FOUND};
    return sendErrorResponse(c_fd, STATUS_NOT_FOUND, data, 2);
}

return_t handleInvalidRequest(int c_fd) {

    const char *const data[] = {INVALID_REQUEST_DATA, WELL_FORMED};
    return sendErrorResponse(c_fd, STATUS_INVALID_REQUEST_DATA, data, 2);
}

return_t handleBadTags(int c_fd) {
    const char *const data[] = {INVALID_REQUEST_DATA, WELL_FORMED, BAD_TAGS};
    return sendErrorResponse(c_fd, STATUS_INVALID_REQUEST_DATA, data, 3);
}




return_t deleteEntry(int c_fd, sqlite3 *db, size_t id, char *table, return_t (*idNotFound_functionPtr)(int)) {

    // Free me!
    char *zErrMsg = NULL;
    char *sql_id_exists = NULL;
    char *sql_delete_tag = NULL;
    cJSON *resp = NULL;

    return_t ret = RET_FAIL;

    //prepare queries
    asprintf(&sql_id_exists, "SELECT id FROM %s WHERE id = %zu", table, id);
    asprintf(&sql_delete_tag, "DELETE FROM %s WHERE id = %zu", table, id);
    if (!sql_delete_tag || !sql_id_exists) {
        handleError(c_fd, OOM);
        goto clean_up;
    }

    // start sqlite transaction block (atomic)
    sqlite3_exec(db, "BEGIN", 0, 0, 0);

    // check if tag exists
    int rc = sqlite3_exec(db, sql_id_exists, checkIfDataExistsCallback, (void *) &resp, &zErrMsg);
    if (SQLITE_OK != rc) {
        handleError(c_fd, zErrMsg);
        goto clean_up;
    }
    if (NULL == resp) {
        (*idNotFound_functionPtr)(c_fd);
        goto clean_up;
    }
    // delete tag
    rc = sqlite3_exec(db, sql_delete_tag, 0, 0, &zErrMsg);
    if (SQLITE_OK != rc) {
        handleError(c_fd, zErrMsg);
        goto clean_up;
    }

    // commit all to sqlite db
    sqlite3_exec(db, "COMMIT", 0, 0, 0);

    ret = sendResponse(c_fd, STATUS_SUCCESSFULLY_DELETED, "");

    clean_up:
    sqlite3_free(zErrMsg);
    free(sql_id_exists);
    free(sql_delete_tag);
    cJSON_Delete(resp);

    return ret;
}

return_t getEntry(sqlite3 *db, size_t id, char *table, char **zErrMsg, cJSON **resp) {

    // Free me!
    char *sql_query = NULL;

    return_t ret = RET_OK;

    // prepare query
    asprintf(&sql_query, "SELECT creation_date, description, id, name FROM %s WHERE id = %zu;", table, id);
    if (!sql_query) {
        ret = RET_OOM;
        goto clean_up;
    }

    // execute query
    int rc = sqlite3_exec(db, sql_query, getEntryCallback, (void*)resp, zErrMsg);
    if (SQLITE_OK != rc) {
        ret = RET_FAIL;
        goto clean_up;
    }

    // Not Found
    if (NULL == *resp) {
        ret = RET_ABORT;
        goto clean_up;
    }

    clean_up:
    free(sql_query);

    return ret;
}

return_t getAllEntries(sqlite3 *db, char *table, char **zErrMsg, cJSON **resp) {

    // Free me!
    char *sql_query = NULL;

    return_t ret = RET_OK;

    // prepare query
    asprintf(&sql_query, "SELECT creation_date, description, id, name FROM %s;", table);
    if (!sql_query) {
        ret = RET_OOM;
        goto clean_up;
    }

    // init resp array
    *resp = cJSON_CreateArray();

    // execute query
    int rc = sqlite3_exec(db, sql_query, getAllEntriesCallback, *resp, zErrMsg);
    if (SQLITE_OK != rc) {
        ret = RET_FAIL;
        goto clean_up;
    }

    clean_up:
    free(sql_query);

    return ret;
}
