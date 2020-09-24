import {
    BAD_TAGS, ERROR, getErrorResponse, INVALID_REQUEST_DATA,
    NOT_FOUND,
    STATUS_INVALID_REQUEST_DATA,
    STATUS_NOT_FOUND, STATUS_SUCCESSFUL, STATUS_SUCCESSFULLY_CREATED,
    STATUS_SUCCESSFULLY_DELETED,
    TASK_ID_NOT_FOUND, WELL_FORMED
} from "./Responses";
import {allAsync, getAsync, runAsync} from "./QueryHelper";
import {
    escapeTag,
    parseAndUnEscapeTags,
    validateDescription,
    validateName, validateParsedTags,
    validateTags
} from "./CustomValidator";

class TaskController {

    async showAll(db, res) {
        try {
            let stmt = "SELECT creation_date, description, id, name FROM tasks";
            let tasks = await allAsync(stmt, [], db);
            for (let j = 0; j < tasks.length; j++) {
                tasks[j] = await updateTaskWithTags(tasks[j], db)
            }
            res.status(STATUS_SUCCESSFUL)
                .send(tasks)
        } catch (e) {
            res.status(ERROR)
                .send(getErrorResponse([e.message]));
        }
    }


    async show(db, req, res) {
        try {
            let task_id = req.params.id;

            let stmt = "SELECT creation_date, description, id, name FROM tasks WHERE id = ?";
            let task = await getAsync(stmt, [task_id], db);
            let task_exists = task != null;
            if (task_exists) {
                task = await updateTaskWithTags(task, db);
                res.status(STATUS_SUCCESSFUL)
                    .send(task)
            } else {
                res.status(STATUS_NOT_FOUND)
                    .send(getErrorResponse([NOT_FOUND, TASK_ID_NOT_FOUND]));
            }
        } catch (e) {
            res.status(ERROR)
                .send(getErrorResponse([e.message]));
        }
    }


    async add(db, req, res) {
        try {
            let task_name = req.body.name;
            let task_description = req.body.description;
            let task_tags = req.body.tags;

            if (validateName(task_name) || validateDescription(task_description) ||
                validateTags(task_tags)) {
                res.status(STATUS_INVALID_REQUEST_DATA)
                    .send(getErrorResponse([INVALID_REQUEST_DATA, WELL_FORMED]));
                return
            }

            // only parse/validate tags if there are some --> no tags is valid!
            let tags = null;
            if (task_tags.length > 0) {
                tags = parseAndUnEscapeTags(task_tags);
                if (!tags || validateParsedTags(tags)) {
                    res.status(STATUS_INVALID_REQUEST_DATA)
                        .send(getErrorResponse([INVALID_REQUEST_DATA, BAD_TAGS]));
                    return
                }
            }

            await runAsync("Begin", [], db);
            let stmt = "INSERT INTO tasks (name, description) VALUES(?, ?)";
            let task_id = await runAsync(stmt, [task_name, task_description], db);

            if (tags != null) {
                // create new tags and tag_task entries on if there are some ;)
                await addTaskTags(task_id, tags, db);
            }

            stmt = "SELECT creation_date, description, id, name FROM tasks WHERE id = ?";
            let task = await getAsync(stmt, [task_id], db);
            task = await updateTaskWithTags(task, db);

            await runAsync("COMMIT", [], db);

            res.status(STATUS_SUCCESSFULLY_CREATED)
                .send(task)
        } catch (e) {
            await runAsync("ROLLBACK", [], db);
            res.status(STATUS_INVALID_REQUEST_DATA) // constraint error
                .send(getErrorResponse([INVALID_REQUEST_DATA, WELL_FORMED, e.message]))
        }
    };


    async update(db, req, res) {
        try {
            let task_id = req.params.id;
            let task_name = req.body.name;
            let task_description = req.body.description;
            let task_tags = req.body.tags;

            if (validateName(task_name) || validateDescription(task_description) ||
                validateTags(task_tags)) {
                res.status(STATUS_INVALID_REQUEST_DATA)
                    .send(getErrorResponse([INVALID_REQUEST_DATA, WELL_FORMED]));
                return
            }

            // only parse/validate tags if there are some --> no tags is valid!
            let tags = null;
            if (task_tags.length > 0) {
                tags = parseAndUnEscapeTags(task_tags);
                if (!tags || validateParsedTags(tags)) {
                    res.status(STATUS_INVALID_REQUEST_DATA)
                        .send(getErrorResponse([INVALID_REQUEST_DATA, BAD_TAGS]));
                    return
                }
            }

            await runAsync("Begin", [], db);
            let stmt = "SELECT id FROM tasks WHERE id = ?";
            let response = await getAsync(stmt, [task_id], db);
            let task_exists = response != null;
            if (!task_exists) {
                await runAsync("ROLLBACK", [], db);
                res.status(STATUS_NOT_FOUND)
                    .send(getErrorResponse([NOT_FOUND, TASK_ID_NOT_FOUND]));
                return
            }

            stmt = "UPDATE tasks SET name = ?, description = ? WHERE id = ?";
            await runAsync(stmt, [task_name, task_description, task_id], db);

            // delete old tag_tag entries only if there are some
            stmt = "DELETE FROM task_tags WHERE task_id = ?";
            await runAsync(stmt, [task_id], db);

            if (tags != null) {
                // create new tags and tag_task entries only if there are some
                await addTaskTags(task_id, tags, db);
            }

            stmt = "SELECT creation_date, description, id, name FROM tasks WHERE id = ?";
            let updated_task = await getAsync(stmt, [task_id], db);
            updated_task = await updateTaskWithTags(updated_task, db);

            await runAsync("COMMIT", [], db);
            res.status(STATUS_SUCCESSFUL)
                .send(updated_task);
        } catch (e) {
            await runAsync("ROLLBACK", [], db);
            res.status(STATUS_INVALID_REQUEST_DATA)
                .send(getErrorResponse([INVALID_REQUEST_DATA, WELL_FORMED, e.message]));
        }
    }


    async delete(db, req, res) {
        try {
            let task_id = req.params.id;

            let stmt = "SELECT id FROM tasks WHERE id = ?";
            let response = await getAsync(stmt, [task_id], db);
            let task_exists = response != null;
            if (task_exists) {
                stmt = "DELETE FROM tasks WHERE id = ?";
                await runAsync(stmt, [task_id], db);
                res.status(STATUS_SUCCESSFULLY_DELETED)
                    .send([])
            } else {
                res.status(STATUS_NOT_FOUND)
                    .send(getErrorResponse([NOT_FOUND, TASK_ID_NOT_FOUND]))
            }
        } catch (e) {
            res.status(ERROR)
                .send(getErrorResponse([e.message]))
        }
    }
}


async function addTaskTags(task_id, tags, db) {
    // create new tag_task entries
    for (let i = 0; i < tags.length; i++) {
        let tag = tags[i];
        let stmt = ("SELECT id FROM tags WHERE name = ?");
        let response = await getAsync(stmt, [tags[i]], db);
        let tag_exists = response != null;
        let tag_id;
        if (tag_exists) {
            tag_id = response.id;
        } else { // create tag
            stmt = "INSERT INTO tags (name, description) VALUES(?, ?)";
            tag_id = await runAsync(stmt, [tag, ""], db);
        }
        stmt = "INSERT OR IGNORE INTO task_tags (tag_id, task_id) VALUES(?, ?)";
        await runAsync(stmt, [tag_id, task_id], db);
    }
}

async function updateTaskWithTags(task, db) {
    let stmt = "SELECT name FROM task_tags tt" +
        " INNER JOIN tags t ON t.id = tt.tag_id" +
        " WHERE tt.task_id = ?" +
        " ORDER BY t.name ASC";
    let tagNames = await allAsync(stmt, [task.id], db);
    task["tags"] = "";
    for (let i = 0; i < tagNames.length; i++) {
        if (i !== 0) {
            task["tags"] += ",";
        }
        task["tags"] += escapeTag(tagNames[i].name);
    }
    return task;
}

module.exports = TaskController;
