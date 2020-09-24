#include "dbconfig.h"

int initDatabase(sqlite3 *db) {

    char *zErrMsg = 0;

    char *sql_tasks = "CREATE TABLE if not exists tasks (" \
           "id INTEGER PRIMARY KEY AUTOINCREMENT," \
           " name VARCHAR(30) NOT NULL," \
           " description VARCHAR(100) NOT NULL," \
           " creation_date DATETIME DEFAULT CURRENT_TIMESTAMP" \
           ")";
    int rc = sqlite3_exec(db, sql_tasks, 0, 0, &zErrMsg);
    if (SQLITE_OK != rc) goto error;


    char *sql_tags = "CREATE TABLE if not exists tags (" \
           "id INTEGER PRIMARY KEY AUTOINCREMENT," \
           " name VARCHAR(30) UNIQUE NOT NULL," \
           " description VARCHAR(100) NOT NULL," \
           " creation_date DATETIME DEFAULT CURRENT_TIMESTAMP" \
           ")";
    rc = sqlite3_exec(db, sql_tags, 0, 0, &zErrMsg);
    if (SQLITE_OK != rc) goto error;


    char *sql = "PRAGMA foreign_keys = ON";
    rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    if (SQLITE_OK != rc) goto error;


    char *sql_task_tags = "CREATE TABLE if not exists task_tags (" \
           "task_id INTEGER NOT NULL," \
           " tag_id INTEGER NOT NULL," \
           " PRIMARY KEY (task_id, tag_id)," \
           " FOREIGN KEY(task_id) REFERENCES tasks(id) ON DELETE CASCADE," \
           " FOREIGN KEY(tag_id) REFERENCES tags(id) ON DELETE CASCADE" \
           ")";

    rc = sqlite3_exec(db, sql_task_tags, 0, 0, &zErrMsg);
    if (SQLITE_OK != rc) goto error;

    return 0;


    error:
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);

    return -1;
}