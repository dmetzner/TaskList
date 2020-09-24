import {runAsync} from "../controller/QueryHelper";

const sqlite3 = require("sqlite3").verbose();


let init = async function (db_path) {

    // Load database file (Creates file if not exists)
    let db = new sqlite3.Database(db_path, sqlite3.OPEN_READWRITE | sqlite3.OPEN_CREATE, (err) => {
        if (err) {
            console.error(err);
        } else {
            console.log("Connected to '", db_path, "' sqlite3 database");
        }
    });

    db.serialize(async function () {
        // Init tags and task tables if they don't exist
        await runAsync("CREATE TABLE if not exists tasks (" +
            "id INTEGER PRIMARY KEY AUTOINCREMENT," +
            " name VARCHAR(30) NOT NULL," +
            " description VARCHAR(100) NOT NULL," +
            " creation_date DATETIME DEFAULT CURRENT_TIMESTAMP" +
            ")", [], db);

        await runAsync("CREATE TABLE if not exists tags (" +
            "id INTEGER PRIMARY KEY AUTOINCREMENT," +
            " name VARCHAR(30) UNIQUE NOT NULL," +
            " description VARCHAR(100) NOT NULL," +
            " creation_date DATETIME DEFAULT CURRENT_TIMESTAMP" +
            ")", [], db);

        await runAsync("PRAGMA foreign_keys = ON", [], db);

        await runAsync("CREATE TABLE if not exists task_tags (" +
            "task_id INTEGER NOT NULL," +
            " tag_id INTEGER NOT NULL," +
            " PRIMARY KEY (task_id, tag_id)," +
            " FOREIGN KEY(task_id) REFERENCES tasks(id) ON DELETE CASCADE," +
            " FOREIGN KEY(tag_id) REFERENCES tags(id) ON DELETE CASCADE" +
            ")", [], db);
    });
    console.log("database initialized");
    return db;
};

module.exports = init;
