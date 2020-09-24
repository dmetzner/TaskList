#include "routes.h"

void handleRouting(int c_fd, sqlite3 *db, request_t *request) {

    // paths case sensitive, methods not!

    if (strcmp(request->path, "/") == 0) {
        if (strcasecmp(request->method, "GET") == 0) {
            showRoot(c_fd);
        }
        else {
            methodNotAllowed(c_fd);
        }
    }
    else if (strcmp(request->path, "/tasks") == 0) {

        if (strcasecmp(request->method, "GET") == 0) {
            showAllTasks(c_fd, db);
        }
        else if (strcasecmp(request->method, "POST") == 0) {
            addTask(c_fd, db, request->params);
        }
        else {
            methodNotAllowed(c_fd);
        }
    }
    else if (2 == request->path_parts && strncmp(request->path, "/tasks/", strlen("/tasks/")) == 0) {

        if (request->id == 0) {
            handleTaskNotFound(c_fd);
        }
        else if (strcasecmp(request->method, "GET") == 0) {
            showTask(c_fd, db, request->id);
        }
        else if (strcasecmp(request->method, "PUT") == 0) {
            updateTask(c_fd, db, request->id, request->params);
        }
        else if (strcasecmp(request->method, "DELETE") == 0) {
            deleteTask(c_fd, db, request->id);
        }
        else {
            methodNotAllowed(c_fd);
        }
    }
    else if (strcmp(request->path, "/tags") == 0) {
        if (strcasecmp(request->method, "GET") == 0) {
            showAllTags(c_fd, db);
        }
        else {
            methodNotAllowed(c_fd);
        }
    }
    else if (2 == request->path_parts && strncmp(request->path, "/tags/", strlen("/tags/")) == 0) {

        if (request->id == 0) {
            handleTagNotFound(c_fd);
        }
        else if (strcasecmp(request->method, "GET") == 0) {
            showTag(c_fd, db, request->id);
        }
        else if (strcasecmp(request->method, "PUT") == 0) {
            updateTag(c_fd, db, request->id, request->params);
        }
        else if (strcasecmp(request->method, "DELETE") == 0) {
            deleteTag(c_fd, db, request->id);
        }
        else {
            methodNotAllowed(c_fd);
        }
    }
    else {
        urlNotFound(c_fd);
    }
}
