#include "tags_controller.h"

void showAllTags(int c_fd, sqlite3 *db) {

    // Free ME
    char *zErrMsg = NULL;
    cJSON *resp = NULL;

    // try to get all tags
    switch ((size_t) getAllEntries(db, "tags", &zErrMsg, &resp)) {
        case RET_OOM:
            handleError(c_fd, OOM);
            goto clean_up;
        case RET_FAIL:
            handleError(c_fd, zErrMsg);
            goto clean_up;
        default:
            break;
    }

    sendJsonResponse(c_fd, STATUS_SUCCESSFUL, resp);

    clean_up:
    sqlite3_free(zErrMsg);
    cJSON_Delete(resp);
}


void showTag(int c_fd, sqlite3 *db, size_t id) {

    // Free ME
    char *zErrMsg = NULL;
    char *sql_query = NULL;
    cJSON *resp = NULL;

    // try to get the tag based on the id
    switch ((size_t) getEntry(db, id, "tags", &zErrMsg, &resp)) {
        case RET_OOM:
            handleError(c_fd, OOM);
            goto clean_up;
        case RET_FAIL:
            handleError(c_fd, zErrMsg);
            goto clean_up;
        case RET_ABORT:
            handleTagNotFound(c_fd);
            goto clean_up;
        default:
            break;
    }

    sendJsonResponse(c_fd, STATUS_SUCCESSFUL, resp);

    clean_up:
    sqlite3_free(zErrMsg);
    free(sql_query);
    cJSON_Delete(resp);
}


void updateTag(int c_fd, sqlite3 *db, size_t id, cJSON *params) {

    // Free me!
    char *zErrMsg = NULL;
    char *sql_id_exists = NULL;
    char *sql_get_tag = NULL;
    cJSON *resp = NULL;
    char *name = NULL;
    char *description = NULL;
    sqlite3_stmt* stmt = NULL; // used against sql injection

    // add task should roll back all db changes if something failed
    size_t rollback = 0;

    // validate body!
    if (parseTagBody(params, &name, &description)) {
        handleInvalidRequest(c_fd);
        goto clean_up;
    }

    //prepare queries
    asprintf(&sql_id_exists, "SELECT id FROM tags WHERE id = %zu;", id);
    asprintf(&sql_get_tag, "SELECT creation_date, description, id, name FROM tags WHERE id = %zu;", id);
    if (!sql_id_exists || !sql_get_tag) {
        handleError(c_fd, OOM);
        goto clean_up;
    }

    // Begin transaction (atomic commit logic) -> now errors should do a rollback
    sqlite3_exec(db, "BEGIN", 0, 0, 0);
    rollback = 1;

    // check if tag exists
    if (SQLITE_OK != sqlite3_exec(db, sql_id_exists, checkIfDataExistsCallback, (void *) &resp, &zErrMsg)) {
        handleError(c_fd, zErrMsg);
        goto clean_up;
    }
    if (NULL == resp) {
        handleTagNotFound(c_fd);
        goto clean_up;
    }

    // update tag -> Use prepared stmt to prevent sql injections!
    char *sql_update_tag = "UPDATE tags SET name=?, description=? WHERE id=?;";
    if (SQLITE_OK != sqlite3_prepare_v2(db, sql_update_tag, -1, &stmt, 0)) {
        handleError(c_fd, sqlite3_errmsg(db));
        goto clean_up;
    }
    if (SQLITE_OK != sqlite3_bind_text( stmt, 1, name, (int)strlen(name), SQLITE_STATIC)) {
        // Bind first parameter.
        handleError(c_fd, sqlite3_errmsg(db));
        goto clean_up;
    }
    if (SQLITE_OK != sqlite3_bind_text( stmt, 2, description, (int)strlen(description), SQLITE_STATIC)) {
        // Bind second parameter.
        handleError(c_fd, sqlite3_errmsg(db));
        goto clean_up;
    }
    if (SQLITE_OK != sqlite3_bind_int( stmt, 3, (int)id)) {
        // Bind third parameter.
        handleError(c_fd, sqlite3_errmsg(db));
        goto clean_up;
    }
    if (SQLITE_DONE != sqlite3_step(stmt)) {
        handleError(c_fd, sqlite3_errmsg(db));
        goto clean_up;
    }

    // get updated tag
    cJSON_Delete(resp); // free old resp!
    if (SQLITE_OK != sqlite3_exec(db, sql_get_tag, getEntryCallback, (void *) &resp, &zErrMsg)) {
        handleError(c_fd, zErrMsg);
        goto clean_up;
    }

    // commit all to sqlite db
    sqlite3_exec(db, "COMMIT", 0, 0, 0);

    // everything worked -> no rollback needed
    rollback = 0;

    sendJsonResponse(c_fd, STATUS_SUCCESSFUL, resp);


    clean_up:
    if (rollback) {
        sqlite3_exec(db, "ROLLBACK", 0, 0, 0);
    }
    free(name);
    free(description);
    sqlite3_free(zErrMsg);
    free(sql_id_exists);
    free(sql_get_tag);
    cJSON_Delete(resp);
    sqlite3_finalize(stmt);
}


void deleteTag(int c_fd, sqlite3 *db, size_t id) {

    deleteEntry(c_fd, db, id, "tags", handleTagNotFound);
}
