#include "tasks_controller.h"
#include "validator.h"

static int showTagNamesCallback(void *data, int argc, char **argv, char **azColName) {

    for (int i = 0; i < argc; i++) {
        cJSON_AddStringToObject(data, "", argv[i] ? argv[i] : "NULL");
    }

    return 0;
}


return_t updateTaskWithTags(sqlite3 *db, size_t task_id, cJSON *task, char **zErrMsg) {

    // Free Me !
    char *sql_query = NULL;
    cJSON *resp2 = cJSON_CreateObject();
    char *tmp_tag_name = NULL;
    char *final_tags = NULL;

    return_t ret = RET_OK;

    // prepare query
    asprintf(&sql_query, "SELECT name FROM task_tags tt" \
             " INNER JOIN tags t ON t.id = tt.tag_id" \
             " WHERE tt.task_id = %zu" \
             " ORDER BY t.name ASC;", task_id);
    if (!sql_query) {
        ret = RET_OOM;
        goto clean_up;
    }

    // get all tags
    int rc = sqlite3_exec(db, sql_query, showTagNamesCallback, resp2, zErrMsg);
    if (SQLITE_OK != rc) {
        ret = RET_FAIL;
        goto clean_up;
    }

    // No Tags? No Work :D
    if (cJSON_GetArraySize(resp2) == 0) {
        cJSON_AddStringToObject(task, "tags", "");
        goto clean_up;
    }

    // ToDo Refactor -> priority low: already works!
    // Find out how much space we have to allocate!
    cJSON *element = NULL;
    size_t elems = 0;
    size_t length = 0;
    cJSON_ArrayForEach(element, resp2) {
        tmp_tag_name = cJSON_Print(element);
        elems++;
        char *s = tmp_tag_name;
        s++;
        s[strlen(s)-1] = '\0';
        for (size_t i = 0; i < strlen(s); i++) {
            char c = s[i];
            if ('%' == c || ',' == c) {
                length++; // to add escape sign
            }
        }
        length += strlen(s);
        free(tmp_tag_name);
        tmp_tag_name = NULL;
    }
    length += (elems - 1); //  to append ',' delimiters

    // now we know the total length we can allocate enough space !
    final_tags = calloc(1, length + 1);
    if (!final_tags) {
        ret = RET_OOM;
        goto clean_up;
    }

    size_t index = 0;
    cJSON_ArrayForEach(element, resp2) {
        if (index != 0) {
            final_tags[index] = ',';
            index++;
        }
        tmp_tag_name = cJSON_Print(element);
        char * s = tmp_tag_name;
        s++;
        s[strlen(s)-1] = '\0';
        for (size_t i = 0; i < strlen(s); i++) {
            char c = s[i];
            if ('%' == c || ',' == c) {
                // escape sign
                final_tags[index] = '%';
                index++;
            }
            final_tags[index] = c;
            index++;
        }
        free(tmp_tag_name);
        tmp_tag_name = NULL;
    }
    final_tags[index] = '\0';

    cJSON_AddStringToObject(task, "tags", final_tags);

    clean_up:
    free(sql_query);
    cJSON_Delete(resp2);
    free(tmp_tag_name);
    free(final_tags);

    return ret;
}

return_t insertAllTags(sqlite3 *db, char** input_tags, size_t amount_of_tags, size_t task_id, char **zErrMsg) {


    // Free Me
    char *sql_tag_task = NULL;
    sqlite3_stmt* check_stmt = NULL;
    sqlite3_stmt* insert_stmt = NULL;

    return_t ret = RET_OK;

    for (size_t i = 0; i < amount_of_tags; i++) {

        if (input_tags == NULL || input_tags[i] == NULL) {
            ret = RET_FAIL;
            goto clean_up;
        }

        // clean_up for new iteration
        free(sql_tag_task);
        sql_tag_task = NULL;

        // store tag id
        size_t tag_id = 0;

        // check if tag exists? -> Use prepared stmt to check if tag exists!
        char *sql_check_tag_exists = "SELECT id FROM tags WHERE name = ?;";
        if (SQLITE_OK != sqlite3_prepare_v2(db, sql_check_tag_exists, -1, &check_stmt, 0)) {
            ret = RET_FAIL;
            goto clean_up;
        }
        if (SQLITE_OK != sqlite3_bind_text(check_stmt, 1, input_tags[i], (int)strlen(input_tags[i]), SQLITE_STATIC)) {
            // Bind first parameter.
            ret = RET_FAIL;
            goto clean_up;
        }
        int step_ret = sqlite3_step(check_stmt);
        if (SQLITE_ROW == step_ret) {
            tag_id = (size_t) sqlite3_column_int(check_stmt, 0);
        }
        else if (SQLITE_DONE != step_ret) {
            ret = RET_FAIL;
            goto clean_up;
        }
        sqlite3_finalize(check_stmt);
        check_stmt = NULL;

        if (0 == tag_id) {
            //  create tag if not exists -> Use prepared stmt to check if tag exists while preventing sql injection!
            char *sql_insert_tag = "INSERT INTO tags (name, description) VALUES(?, '');";
            if (SQLITE_OK != sqlite3_prepare_v2(db, sql_insert_tag, -1, &insert_stmt, 0)) {
                ret = RET_FAIL;
                goto clean_up;
            }

            if (SQLITE_OK != sqlite3_bind_text(insert_stmt, 1, input_tags[i], (int)strlen(input_tags[i]), SQLITE_STATIC)) {
                // Bind first parameter.
                ret = RET_FAIL;
                goto clean_up;
            }
            if (SQLITE_DONE != sqlite3_step(insert_stmt)) {
                ret = RET_FAIL;
                goto clean_up;
            }
            tag_id = (size_t) sqlite3_last_insert_rowid(db);

            sqlite3_finalize(insert_stmt);
            insert_stmt = NULL;
        }

        // now that we know the id, create a task tag connection if needed
        // prepare stmt
        asprintf(&sql_tag_task, "INSERT OR IGNORE INTO task_tags (tag_id, task_id) VALUES(%zu, %zu);", tag_id, task_id);
        if (!sql_tag_task) {
            ret = RET_FAIL;
            goto clean_up;
        }
        // execute stmt
        if (SQLITE_OK != sqlite3_exec(db, sql_tag_task, 0, 0, zErrMsg)) {
            ret = RET_FAIL;
            goto clean_up;
        }
    }

    clean_up:
    free(sql_tag_task);
    sqlite3_finalize(check_stmt);
    sqlite3_finalize(insert_stmt);
    return ret;
}



void showAllTasks(int c_fd, sqlite3 * db) {

    // Free ME
    char *zErrMsg = NULL;
    cJSON *tasks = NULL;

    // try to get all tasks
    switch ((size_t) getAllEntries(db, "tasks", &zErrMsg, &tasks)) {
        case RET_OOM:
            handleError(c_fd, OOM);
            goto clean_up;
        case RET_FAIL:
            handleError(c_fd, zErrMsg);
            goto clean_up;
        default:
            break;
    }

    // update tasks with tags!
    cJSON *task = NULL;
    cJSON_ArrayForEach(task, tasks) {

        cJSON *tmp = cJSON_GetObjectItem(task, "id");
        char *id_as_string = cJSON_Print(tmp);
        size_t id = (size_t) strtol(id_as_string, 0, 10);
        free(id_as_string);

        switch ((size_t) updateTaskWithTags(db, id, task, &zErrMsg)) {
            case RET_OOM:
                handleError(c_fd, OOM);
                goto clean_up;
            case RET_FAIL:
                handleError(c_fd, zErrMsg);
                goto clean_up;
            default:
                break;
        }
    }

    sendJsonResponse(c_fd, STATUS_SUCCESSFUL, tasks);

    clean_up:
    sqlite3_free(zErrMsg);
    cJSON_Delete(tasks);
}


void showTask(int c_fd, sqlite3 * db, size_t id) {

    // Free ME
    char *zErrMsg = NULL;
    char *sql_query =  NULL;
    cJSON *resp = NULL;

    // try to get the task based on the id
    switch ((size_t) getEntry(db, id, "tasks", &zErrMsg, &resp)) {
        case RET_OOM:
            handleError(c_fd, OOM);
            goto clean_up;
        case RET_FAIL:
            handleError(c_fd, zErrMsg);
            goto clean_up;
        case RET_ABORT:
            handleTaskNotFound(c_fd);
            goto clean_up;
        default:
            break;
    }

    // add the tags to the task
    switch ((size_t) updateTaskWithTags(db, id, resp, &zErrMsg)) {
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
    free(sql_query);
    cJSON_Delete(resp);
}


void addTask(int c_fd, sqlite3 * db, cJSON *params) {

    // Free me!
    char *zErrMsg = NULL;
    char *sql_insert_task = NULL;
    cJSON *resp = NULL;
    char *name = NULL;
    char *description = NULL;
    char **input_tags = NULL;
    char *all_tags = NULL;
    sqlite3_stmt* stmt = NULL;

    // add task should roll back all db changes if something failed
    size_t rollback = 0;

    // validate body!
    if (parseTaskBody(params, &name, &description, &all_tags)) {
        handleInvalidRequest(c_fd);
        goto clean_up;
    }

    // parseTags
    size_t amount_of_tags = 0;
    if (strlen(all_tags) > 0) {
        // only parse tags if there are some -> no tags is totally valid
        input_tags = parseTags(all_tags, &amount_of_tags);
        if (!input_tags) {
            handleBadTags(c_fd);
            goto clean_up;
        }
    }

    // start sqlite transaction block, -> now errors should do a rollback
    sqlite3_exec(db, "BEGIN", 0, 0, 0);
    rollback = 1;

    // Use prepared stmt to prevent sql injections when adding tasks
    sql_insert_task = "INSERT INTO tasks (name, description) VALUES(?, ?);";
    if (SQLITE_OK != sqlite3_prepare_v2(db, sql_insert_task, -1, &stmt, 0)) {
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
    if (SQLITE_DONE != sqlite3_step(stmt)) {
        handleError(c_fd, sqlite3_errmsg(db));
        goto clean_up;
    }

    // instant free -> else false positives in valgrind -> possible leaks!
    sqlite3_finalize(stmt);
    stmt = NULL;

    // save the task id
    sqlite3_int64 si64 = sqlite3_last_insert_rowid(db);  // Overflow protection!
    if (si64 > INT32_MAX) {
        handleError(c_fd, "DB is full!");
        goto clean_up;
    }
    size_t task_id = (size_t) si64;

    // insert all tags into the db
    switch ((size_t) insertAllTags(db, input_tags, amount_of_tags, task_id, &zErrMsg)) {
        case RET_FAIL:
            handleError(c_fd, zErrMsg);
            goto clean_up;
        default:
            break;
    }

    // Now get the task that was added!->  try to get the task based on the id
    switch ((size_t) getEntry(db, task_id, "tasks", &zErrMsg, &resp)) {
        case RET_OOM:
            handleError(c_fd, OOM);
            goto clean_up;
        case RET_FAIL:
            handleError(c_fd, zErrMsg);
            goto clean_up;
        case RET_ABORT:
            handleTaskNotFound(c_fd);
            goto clean_up;
        default:
            break;
    }

    // add the tags to the task
    switch ((size_t) updateTaskWithTags(db, task_id, resp, &zErrMsg)) {
        case RET_OOM:
            handleError(c_fd, OOM);
            goto clean_up;
        case RET_FAIL:
            handleError(c_fd, zErrMsg);
            goto clean_up;
        default:
            break;
    }

    // commit all to sqlite db
    sqlite3_exec(db, "COMMIT", 0, 0, 0);

    // everything worked -> no rollback needed
    rollback = 0;

    // send creation response
    sendJsonResponse(c_fd, STATUS_SUCCESSFULLY_CREATED, resp);


    clean_up:
    if (rollback) {
        sqlite3_exec(db, "ROLLBACK", 0, 0, 0);
    }
    sqlite3_free(zErrMsg);
    cJSON_Delete(resp);
    free(name);
    free(description);
    free(input_tags);
    free(all_tags);
    sqlite3_finalize(stmt);
}


void updateTask(int c_fd, sqlite3 *db, size_t id, cJSON *params) {

    // Free me!
    char *zErrMsg = NULL;
    char *sql_id_exists = NULL;
    char *sql_delete_tag_tasks = NULL;
    cJSON *resp = NULL;
    char *name = NULL;
    char *description = NULL;
    char *all_tags = NULL;
    char **input_tags = NULL;
    sqlite3_stmt* update_stmt = NULL;

    // add task should roll back all db changes if something failed
    size_t rollback = 0;

    // validate body!
    if (parseTaskBody(params, &name, &description, &all_tags)) {
        handleInvalidRequest(c_fd);
        goto clean_up;
    }

    // parseTags
    size_t amount_of_tags = 0;
    if (strlen(all_tags) > 0) {
        // only parse tags if there are some -> no tags is totally valid
        input_tags = parseTags(all_tags, &amount_of_tags);
        if (!input_tags) {
            handleBadTags(c_fd);
            goto clean_up;
        }
    }

    //prepare queries
    asprintf(&sql_id_exists, "SELECT id FROM tasks WHERE id = %zu;", id);
    asprintf(&sql_delete_tag_tasks, "DELETE FROM task_tags WHERE task_id = %zu;", id);
    if (!sql_id_exists || !sql_delete_tag_tasks) {
        handleError(c_fd, OOM);
        goto clean_up;
    }

    // Begin transaction (atomic commit logic) -> now errors should do a rollback
    sqlite3_exec(db, "BEGIN", 0, 0, 0);
    rollback = 1;

    // check if task exists
    if (SQLITE_OK != sqlite3_exec(db, sql_id_exists, checkIfDataExistsCallback, (void *) &resp, &zErrMsg)) {
        handleError(c_fd, zErrMsg);
        goto clean_up;
    }
    if (NULL == resp) {
        handleTagNotFound(c_fd);
        goto clean_up;
    }

    // Use prepared stmt to update tasks and prevent sql injections!
    char *sql_update_task = "UPDATE tasks SET name = ?, description = ? WHERE id = ?;";
    if (SQLITE_OK != sqlite3_prepare_v2(db, sql_update_task, -1, &update_stmt, 0)) {
        handleError(c_fd, sqlite3_errmsg(db));
        goto clean_up;
    }
    if (SQLITE_OK != sqlite3_bind_text(update_stmt, 1, name, (int)strlen(name), SQLITE_STATIC)) {
        // Bind first parameter.
        handleError(c_fd, sqlite3_errmsg(db));
        goto clean_up;
    }
    if (SQLITE_OK != sqlite3_bind_text(update_stmt, 2, description, (int)strlen(description), SQLITE_STATIC)) {
        // Bind second parameter.
        handleError(c_fd, sqlite3_errmsg(db));
        goto clean_up;
    }
    if (SQLITE_OK != sqlite3_bind_int(update_stmt, 3, (int)id)) {
        // Bind third parameter.
        handleError(c_fd, sqlite3_errmsg(db));
        goto clean_up;
    }
    if (SQLITE_DONE != sqlite3_step(update_stmt)) {
        handleError(c_fd, sqlite3_errmsg(db));
        goto clean_up;
    }
    sqlite3_finalize(update_stmt);
    update_stmt = NULL;

    // delete all tag_task connection
    if (SQLITE_OK != sqlite3_exec(db, sql_delete_tag_tasks, 0, 0, &zErrMsg)) {
        handleError(c_fd, zErrMsg);
        goto clean_up;
    }

    // insert all tags into the db
    cJSON_Delete(resp); // free old resp!
    resp = NULL;
    switch ((size_t) insertAllTags(db, input_tags, amount_of_tags, id, &zErrMsg)) {
        case RET_FAIL:
            handleError(c_fd, zErrMsg);
            goto clean_up;
        default:
            break;
    }

    // Now get the task that was added!->  try to get the task based on the id
    switch ((size_t) getEntry(db, id, "tasks", &zErrMsg, &resp)) {
        case RET_OOM:
            handleError(c_fd, OOM);
            goto clean_up;
        case RET_FAIL:
            handleError(c_fd, zErrMsg);
            goto clean_up;
        case RET_ABORT:
            handleTaskNotFound(c_fd);
            goto clean_up;
        default:
            break;
    }

    // add the tags to the task
    switch ((size_t) updateTaskWithTags(db, id, resp, &zErrMsg)) {
        case RET_OOM:
            handleError(c_fd, OOM);
            goto clean_up;
        case RET_FAIL:
            handleError(c_fd, zErrMsg);
            goto clean_up;
        default:
            break;
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
    free(sql_delete_tag_tasks);
    cJSON_Delete(resp);
    free(input_tags);
    free(all_tags);
    sqlite3_finalize(update_stmt);
}

void deleteTask(int c_fd, sqlite3 *db, size_t id) {

    deleteEntry(c_fd, db, id, "tasks", handleTaskNotFound);
}