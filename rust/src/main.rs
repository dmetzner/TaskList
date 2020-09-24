#![feature(proc_macro_hygiene, decl_macro)]

#[macro_use]
extern crate rocket;
extern crate rusqlite;
extern crate rusq;
#[macro_use]
extern crate rocket_contrib;
extern crate serde;
#[macro_use]
extern crate serde_derive;
extern crate time;
extern crate chrono;

// command line args and exit program
use std::env;
use std::process;

// Rocket configuration stuff
use rocket::config::{Config, Environment};
use rocket_contrib::templates::Template;
use std::collections::HashMap; // needed for html pages!

// Json Api -> make responses look like json responses with custom response values
use rocket::http::Status;
use rocket_contrib::json::Json;
use rocket_contrib::json::JsonValue;
use rocket::response::status::Custom;
use serde::ser::{Serialize, Serializer, SerializeStruct};

// database -> Mutex, and sate for synchronisation,
//          -> rusqlite has everything we need to work with the db
use std::sync::{Mutex, MutexGuard};
use rocket::State;
use rusqlite::types::ToSql;
use rusqlite::{Connection, NO_PARAMS};
use time::Timespec;
use chrono::prelude::*;
use chrono::TimeZone;


//--------------------------------------------------------------------------------------------------
// Entry Point!
//
fn main() {

    // validate cmd line args
    let args: Vec<String> = env::args().collect();

    if args.len() != 3 {
        println!("Provide db path and port as arguments");
        process::exit(-1)
    }

    let db_path = &args[1];
    let res = &args[2].parse::<u16>();
    let port: u16;

    match res {
        Ok(v) => {
            port = *v;
        }
        Err(e) => {
            println!("provide a valid port -> {:?}", e);
            process::exit(-1)
        }
    }

    //Open SQLite database.
    let conn = Connection::open(db_path).expect("db");
    init_database(&conn);

    //init rocket
    let config = Config::build(Environment::Development)
        .port(port)
        .workers(12)
        .unwrap();

    // launch rocket ;)
    let e = rocket::custom(config)
        .manage(Mutex::new(conn))
        .attach(Template::fairing())
        .mount("/", routes![
            index,
            tags_get_all, tag_get, tag_update, tag_delete,
            tasks_get_all, task_get, task_update, task_delete, task_add
        ])
        .register(catchers![
            not_found, bad_request, inv_request, server_error, method_not_allowed
        ])
        .launch();

    println!("Something went wrong {:?}", e);
    process::exit(-1)
}



//--------------------------------------------------------------------------------------------------
// type and struct definitions
//

type DbConn = Mutex<Connection>;

#[derive(Debug)]
struct Task {
    id: i64,
    name: String,
    description: String,
    creation_date: Timespec,
    tags: String,
}

#[derive(Debug)]
struct Tag {
    id: i64,
    name: String,
    description: String,
    creation_date: Timespec,
}

#[derive(Deserialize)]
struct TaskReq {
    name: String,
    description: String,
    tags: String,
}

#[derive(Deserialize)]
struct TagReq {
    name: String,
    description: String,
}

#[derive(Debug)]
struct JustIdPlease {
    id: i64,
}

#[derive(Debug)]
struct JustNamePlease {
    name: String,
}

// some of 'em need to be serializable! so make 'em
impl Serialize for Task {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
        where
            S: Serializer,
    {
        // 5 is the number of fields in the struct.
        let mut state = serializer.serialize_struct("Task", 5)?;
        state.serialize_field("id", &self.id)?;
        state.serialize_field("name", &self.name)?;
        state.serialize_field("description", &self.description)?;
        state.serialize_field("creation_date", &Utc.timestamp(self.creation_date.sec, self.creation_date.nsec as u32).to_rfc3339())?;
        state.serialize_field("tags", &self.tags)?;
        state.end()
    }
}

impl Serialize for Tag {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
        where
            S: Serializer,
    {
        // 4 is the number of fields in the struct.
        let mut state = serializer.serialize_struct("Tag", 4)?;
        state.serialize_field("id", &self.id)?;
        state.serialize_field("name", &self.name)?;
        state.serialize_field("description", &self.description)?;
        state.serialize_field("creation_date", &Utc.timestamp(self.creation_date.sec, self.creation_date.nsec as u32).to_rfc3339())?;
        state.end()
    }
}

impl Serialize for JustIdPlease {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
        where
            S: Serializer,
    {
        let mut state = serializer.serialize_struct("JustIdPlease", 1)?;
        state.serialize_field("id", &self.id)?;
        state.end()
    }
}


//--------------------------------------------------------------------------------------------------
// Defining all routes with their logic
//

#[get("/")]
fn index() -> Template {
    let context = HashMap::<String, String>::new();
    Template::render("index", context)
}

// Tags
#[get("/tags")]
fn tags_get_all(conn_mutex: State<DbConn>) -> Custom<JsonValue> {
    let conn = conn_mutex
        .lock()
        .expect("");

    let mut stmt = conn
        .prepare("SELECT id, name, description, creation_date FROM tags")
        .expect("");

    let tag_iter = stmt
        .query_map(NO_PARAMS, |row| Tag {
            id: row.get(0),
            name: row.get(1),
            description: row.get(2),
            creation_date: row.get(3),
        }).expect("");

    let mut vec = Vec::new();
    for tag in tag_iter {
        let t = tag.expect("");
        vec.push(t);
    }
    return Custom(Status::Ok, json!(vec));
}

#[get("/tags/<_id>")]
fn tag_get(conn_mutex: State<DbConn>, _id: i64) -> Custom<JsonValue> {
    let conn = conn_mutex
        .lock()
        .expect("");

    let mut stmt = conn
        .prepare("SELECT id, name, description, creation_date FROM tags WHERE id = ?").expect("");

    let tag_iter = stmt
        .query_map(&[&_id], |row| Tag {
            id: row.get(0),
            name: row.get(1),
            description: row.get(2),
            creation_date: row.get(3),
        }).expect("");

    for tag in tag_iter {
        let t = tag.expect("");
        return Custom(Status::Ok, json!(t));
    }
    return send404()
}

#[put("/tags/<_id>", format = "json", data = "<tag_req>")]
fn tag_update(conn_mutex: State<DbConn>, tag_req: Json<TagReq>, _id: i64) -> Custom<JsonValue> {
    if validate_name(&tag_req.name) || validate_description(&tag_req.description) {
        return send422()
    }

    let conn = conn_mutex
        .lock()
        .unwrap();

    let mut stmt = conn
        .prepare("SELECT id FROM tags WHERE id = ?")
        .unwrap();

    let tag_iter = stmt
        .query_map(&[&_id], |row| JustIdPlease { id: row.get(0) })
        .unwrap();

    let mut tag_exists = false;
    for _tag in tag_iter {
        tag_exists = true;
    }
    if !tag_exists {
        return send404()
    }

    // can fail -> tag names are unique
    let failed = match conn.execute("UPDATE tags SET name = ?, description = ? WHERE id = ?",
                       &[&tag_req.name as &dyn ToSql, &tag_req.description, &_id]){
        Ok(_) => false,
        Err(_) => true,
    };
    if failed {
        return send422();
    }


    let mut stmt = conn
        .prepare("SELECT id, name, description, creation_date FROM tags WHERE id = ?")
        .unwrap();

    let tag_iter = stmt
        .query_map(&[&_id], |row| Tag {
            id: row.get(0),
            name: row.get(1),
            description: row.get(2),
            creation_date: row.get(3),
        }).unwrap();

    for tag in tag_iter {
        let t = tag.unwrap();
        return Custom(Status::Ok, json!(t));
    }
    return send404()
}

#[delete("/tags/<_id>")]
fn tag_delete(conn_mutex: State<DbConn>, _id: i64) -> Custom<JsonValue> {
    let conn = conn_mutex
        .lock()
        .unwrap();

    let mut stmt = conn
        .prepare("SELECT id FROM tags WHERE id = ?")
        .unwrap();

    let tag_iter = stmt
        .query_map(&[&_id], |row| JustIdPlease { id: row.get(0) })
        .unwrap();

    for _tag in tag_iter {
        conn.execute("DELETE FROM tags WHERE id = ?", &[&_id])
            .unwrap();
        return Custom(Status::NoContent, json!([]));
    }
    return send404()
}

// tasks
#[get("/tasks")]
fn tasks_get_all(conn_mutex: State<DbConn>) -> Custom<JsonValue> {
    let conn = conn_mutex
        .lock()
        .expect("");

    let mut stmt = conn
        .prepare("SELECT id, name, description, creation_date FROM tasks")
        .expect("");

    let task_iter = stmt
        .query_map(NO_PARAMS, |row| Task {
            id: row.get(0),
            name: row.get(1),
            description: row.get(2),
            creation_date: row.get(3),
            tags: "".to_string(),
        }).expect("");

    let mut vec = Vec::new();
    for task in task_iter {
        let t = task.expect("");
        let resp = get_task_with_tags(t, &conn);
        vec.push(resp);
    }

    return Custom(Status::Ok, json!(vec));
}

#[get("/tasks/<_id>")]
fn task_get(conn_mutex: State<DbConn>, _id: i64) -> Custom<JsonValue> {
    let conn = conn_mutex
        .lock()
        .expect("");

    let mut stmt = conn
        .prepare("SELECT id, name, description, creation_date FROM tasks WHERE id = ?")
        .expect("");

    let task_iter = stmt
        .query_map(&[&_id], |row| Task {
            id: row.get(0),
            name: row.get(1),
            description: row.get(2),
            creation_date: row.get(3),
            tags: "".to_string(),
        }).expect("");

    for task in task_iter {
        let t = task.expect("");
        let resp = get_task_with_tags(t, &conn);
        return Custom(Status::Ok, json!(resp))
    }

    return send404()
}

#[post("/tasks", format = "json", data = "<task_req>")]
fn task_add(conn_mutex: State<DbConn>, task_req: Json<TaskReq>) -> Custom<JsonValue> {
    if validate_name(&task_req.name) || validate_description(&task_req.description) {
        return send422()
    }

    let mut tags = Vec::new();
    if task_req.tags.chars().count() > 0 {
        // parse tags, but only if there is something to parse ;)
        tags = parse_and_unescape_tags(&task_req.tags);
        if tags.len() == 0 || validate_parsed_tags(&tags) {
            return send422()
        }
    }

    let conn = conn_mutex.lock().unwrap();

    conn.execute("INSERT INTO tasks (name, description) VALUES(?, ?)", &[&task_req.name, &task_req.description]).unwrap();

    // save task id!
    let task_id = &conn.last_insert_rowid();

    if tags.len() > 0 {
        add_task_tags(task_id, tags, &conn);
    }

    let mut stmt = conn.prepare("SELECT id, name, description, creation_date FROM tasks WHERE id = ?").unwrap();

    let task_iter = stmt
        .query_map(&[&task_id], |row| Task {
            id: row.get(0),
            name: row.get(1),
            description: row.get(2),
            creation_date: row.get(3),
            tags: "".to_string(),
        }).unwrap();

    for task in task_iter {
        let t = task.unwrap();
        let resp = get_task_with_tags(t, &conn);
        return Custom(Status::Created, json!(resp));
    }

    return send422()
}

#[put("/tasks/<_id>", format = "json", data = "<task_req>")]
fn task_update(conn_mutex: State<DbConn>, task_req: Json<TaskReq>, _id: i64) -> Custom<JsonValue> {
    if validate_name(&task_req.name) || validate_description(&task_req.description) {
        return send422()
    }

    let mut tags = Vec::new();
    if task_req.tags.chars().count() > 0 {
        // parse tags but only if there is something to parse ;)
        tags = parse_and_unescape_tags(&task_req.tags);
        if tags.len() == 0 || validate_parsed_tags(&tags) {
            return send422()
        }
    }

    let conn = conn_mutex
        .lock()
        .unwrap();

    let mut stmt = conn
        .prepare("SELECT id FROM tasks WHERE id = ?")
        .unwrap();

    let task_iter = stmt
        .query_map(&[&_id], |row| JustIdPlease { id: row.get(0) })
        .unwrap();

    let mut task_exists = false;
    for _task in task_iter {
        task_exists = true;
    }
    if !task_exists {
        return send404()
    }

    conn.execute("UPDATE tasks SET name = ?, description = ? WHERE id = ?",
                 &[&task_req.name as &dyn ToSql, &task_req.description, &_id])
        .unwrap();

    // delete old task_tag entries if there are some
    conn.execute("DELETE FROM task_tags WHERE task_id = ?",
                 &[&_id])
        .unwrap();

    if tags.len() > 0 {
        add_task_tags(&_id, tags, &conn);
    }

    let mut stmt = conn.prepare("SELECT id, name, description, creation_date FROM tasks WHERE id = ?").unwrap();

    let task_iter = stmt
        .query_map(&[&_id], |row| Task {
            id: row.get(0),
            name: row.get(1),
            description: row.get(2),
            creation_date: row.get(3),
            tags: "".to_string(),
        }).unwrap();

    for task in task_iter {
        let t = task.unwrap();
        let resp = get_task_with_tags(t, &conn);
        return Custom(Status::Ok, json!(resp))
    }
    return send422()
}

#[delete("/tasks/<_id>")]
fn task_delete(conn_mutex: State<DbConn>, _id: i64) -> Custom<JsonValue> {
    let conn = conn_mutex
        .lock()
        .unwrap();

    let mut stmt = conn
        .prepare("SELECT id FROM tasks WHERE id = ?")
        .unwrap();

    let task_iter = stmt
        .query_map(&[&_id], |row| JustIdPlease { id: row.get(0) })
        .unwrap();

    for _task in task_iter {
        conn.execute("DELETE FROM tasks WHERE id = ?", &[&_id])
            .unwrap();
        return Custom(Status::NoContent, json!([]))
    }
    return send404()
}


//--------------------------------------------------------------------------------------------------
// Defining all Error catcher
//
#[catch(400)]
fn bad_request(_req: &rocket::Request) -> Custom<JsonValue> {
    send400()
}

#[catch(404)]
fn not_found(_req: &rocket::Request) -> Custom<JsonValue> {
    send404()
}

#[catch(405)]
fn method_not_allowed(_req: &rocket::Request) -> Custom<JsonValue> {
    send405()
}

#[catch(422)]
fn inv_request(_req: &rocket::Request) -> Custom<JsonValue> {
    send422()
}

#[catch(500)]
fn server_error(_req: &rocket::Request) -> Custom<JsonValue> {
    send500()
}


//--------------------------------------------------------------------------------------------------
// Helper functions
//
fn get_task_with_tags(task: Task, conn: &MutexGuard<Connection>) -> Task {
    let mut stmt = conn
        .prepare("SELECT name FROM task_tags tt \
        INNER JOIN tags t ON t.id = tt.tag_id \
        WHERE tt.task_id = ? \
        ORDER BY t.name ASC")
        .unwrap();

    let task_iter = stmt
        .query_map(&[&task.id], |row| JustNamePlease { name: row.get(0) })
        .unwrap();

    let mut all_tags: String = String::from("");
    let mut i = 0;
    for task_name in task_iter {
        if i != 0 {
            all_tags.push(DELIMITER);
        }
        let name = task_name.unwrap().name;
        all_tags.push_str(&escape_tag(&name));
        i += 1;
    }

    let task_with_tags = Task {
        id: task.id,
        name: task.name,
        description: task.description,
        creation_date: task.creation_date,
        tags: all_tags,
    };

    return task_with_tags;
}

fn add_task_tags(task_id: &i64, tags: Vec<String>, conn: &MutexGuard<Connection>) {
    for tag in tags {
        let mut stmt = conn.prepare("SELECT id FROM tags WHERE name = ?").unwrap();

        let tag_iter = stmt
            .query_map(&[&tag], |row| JustIdPlease {
                id: row.get(0),
            }).unwrap();

        let mut tag_id: i64 = 0;
        let mut tag_exists = false;
        for row_tag_id in tag_iter {
            tag_exists = true;
            tag_id = row_tag_id.unwrap().id;
        }

        if !tag_exists {
            // create tag
            conn.execute("INSERT INTO tags (name, description) VALUES(?, ?)", &[&tag, &String::from("")]).unwrap();
            tag_id = conn.last_insert_rowid();
        }

        conn.execute("INSERT OR IGNORE INTO task_tags (tag_id, task_id) VALUES(?, ?)", &[&tag_id, &task_id]).unwrap();
    }
}


//--------------------------------------------------------------------------------------------------
// Database stuff
//
fn init_database(conn: &Connection) {
    conn.execute("PRAGMA foreign_keys = ON;", NO_PARAMS).expect("init db");

    conn.execute("CREATE TABLE if not exists tasks (\
        id INTEGER PRIMARY KEY AUTOINCREMENT,\
        name VARCHAR(30) NOT NULL,description VARCHAR(100) NOT NULL,\
        creation_date DATETIME DEFAULT CURRENT_TIMESTAMP\
    );", NO_PARAMS).expect("init db");

    conn.execute("CREATE TABLE if not exists tags (\
        id INTEGER PRIMARY KEY AUTOINCREMENT,\
        name VARCHAR(30) UNIQUE NOT NULL,description VARCHAR(100) NOT NULL,\
        creation_date DATETIME DEFAULT CURRENT_TIMESTAMP\
    );", NO_PARAMS).expect("init db");

    conn.execute("CREATE TABLE if not exists task_tags (\
        task_id INTEGER NOT NULL,\
        tag_id INTEGER NOT NULL,\
        PRIMARY KEY (task_id, tag_id),\
        FOREIGN KEY(task_id) REFERENCES tasks(id) ON DELETE CASCADE,\
        FOREIGN KEY(tag_id) REFERENCES tags(id) ON DELETE CASCADE\
    );", NO_PARAMS).expect("init db");
}


//--------------------------------------------------------------------------------------------------
// validation stuff
//
fn validate_name(name: &String) -> bool {
    return name.chars().count() < 1 || name.chars().count() > 30;
}

fn validate_description(name: &String) -> bool {
    return name.chars().count() > 100;
}

fn validate_parsed_tags(tags: &Vec<String>) -> bool {
    for tag in tags {
        if tag.chars().count() < 1 || tag.chars().count() > 30 {
            return true;
        }
    }
    return false;
}

static ESCAPE_SIGN: char = '%';
static DELIMITER: char = ',';

fn unescape_tag(tag: &String) -> String {
    let mut escaped = false;
    let mut unescaped_tag: String = String::from("");

    for c in tag.chars() {
        if !escaped && c == ESCAPE_SIGN {
            escaped = true;
        } else {
            unescaped_tag.push(c);
            escaped = false;
        }
    }
    return unescaped_tag;
}


fn escape_tag(tag: &String) -> String {
    let mut escaped_tag: String = String::from("");

    for c in tag.chars() {
        if c == ESCAPE_SIGN || c == DELIMITER {
            escaped_tag.push(ESCAPE_SIGN);
            escaped_tag.push(c);
        } else {
            escaped_tag.push(c);
        }
    }
    return escaped_tag;
}

fn parse_and_unescape_tags(tags_as_string: &String) -> Vec<String> {
    let mut tags: Vec<String> = Vec::new();
    let mut tag: String = String::from("");
    let mut escaped = false;

    for c in tags_as_string.chars() {
        if escaped {
            if c == ESCAPE_SIGN || c == DELIMITER {
                // if escaped -> only allow escaped chars!
                tag.push(ESCAPE_SIGN);
                tag.push(c);
            } else {
                // error!
                break;
            }
            escaped = false;
        } else {
            if c == ESCAPE_SIGN {
                escaped = true;
            } else if c == DELIMITER {
                // add tag and start a new one
                tags.push(unescape_tag(&tag));
                tag = String::from("");
            } else {
                tag.push(c);
            }
        }
    }
    if escaped {
        // still escaped? we had an error!
        return Vec::new();
    }

    // also add the last tag!
    tags.push(unescape_tag(&tag));

    return tags;
}


//--------------------------------------------------------------------------------------------------
// Responses
//
fn send400() -> Custom<JsonValue> {
    Custom(Status::BadRequest, json!({ "messages": [ "Bad Request" ]}))
}

fn send404() -> Custom<JsonValue> {
    Custom(Status::NotFound, json!({ "messages": [ "Not Found" ]}))
}

fn send405() -> Custom<JsonValue> {
    Custom(Status::MethodNotAllowed, json!({ "messages": [ "Method not allowed" ]}))
}

fn send422() -> Custom<JsonValue> {
    Custom(Status::UnprocessableEntity, json!({ "messages": [ "Unprocessable Entity" ]}))
}

fn send500() -> Custom<JsonValue> {
    Custom(Status::InternalServerError, json!({ "messages": [ "No risk no fun :)" ]}))
}

