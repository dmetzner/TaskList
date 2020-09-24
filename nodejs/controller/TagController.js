import {allAsync, getAsync, runAsync} from "./QueryHelper";
import {
    BAD_TAGS,
    ERROR, getErrorResponse,
    INVALID_REQUEST_DATA, NOT_FOUND,
    STATUS_INVALID_REQUEST_DATA,
    STATUS_NOT_FOUND,
    STATUS_SUCCESSFUL,
    STATUS_SUCCESSFULLY_DELETED,
    TAG_ID_NOT_FOUND, WELL_FORMED,
} from "./Responses";
import {validateDescription, validateName} from "./CustomValidator";

class TagController {

    async showAll(db, res) {
        try {
            let stmt = "SELECT creation_date, description, id, name FROM tags";
            let tags = await allAsync(stmt, [], db);
            res.status(STATUS_SUCCESSFUL)
                .send(tags)
        } catch (e) {
            res.status(ERROR)
                .send(getErrorResponse([e.message]))
        }
    }

    async show(db, req, res) {
        try {
            let tag_id = req.params.id;
            let stmt = "SELECT creation_date, description, id, name FROM tags WHERE id = ?";
            let tag = await getAsync(stmt, [tag_id], db);
            let tag_exists = tag != null;
            if (!tag_exists) {
                res.status(STATUS_NOT_FOUND)
                    .send(getErrorResponse([NOT_FOUND, TAG_ID_NOT_FOUND]));
            }
            res.status(STATUS_SUCCESSFUL)
                .send(tag)
        } catch (e) {
            res.status(ERROR)
                .send(getErrorResponse([e.message]))
        }
    }

    async update(db, req, res) {
        try {
            let tag_id = req.params.id;

            let tag_name = req.body.name;
            let tag_description = req.body.description;

            if (validateName(tag_name) || validateDescription(tag_description)) {
                res.status(STATUS_INVALID_REQUEST_DATA)
                    .send(getErrorResponse([INVALID_REQUEST_DATA, WELL_FORMED]));
                return
            }

            await runAsync("Begin", [], db);

            let stmt = "SELECT id FROM tags WHERE id = ?";
            let response = await getAsync(stmt, [tag_id], db);
            let tag_exists = response != null;
            if (!tag_exists) {
                res.status(STATUS_NOT_FOUND)
                    .send(getErrorResponse([NOT_FOUND, TAG_ID_NOT_FOUND]));
                await runAsync("ROLLBACK", [], db);
                return
            }

            stmt = "UPDATE tags SET name = ?, description = ? WHERE id = ?";
            await runAsync(stmt, [tag_name, tag_description, tag_id], db);
            stmt = "SELECT creation_date, description, id, name FROM tags WHERE id = ?";
            let updated_tag = await getAsync(stmt, [tag_id], db);

            await runAsync("Commit", [], db);

            res.status(STATUS_SUCCESSFUL)
                .send(updated_tag)
        } catch (e) {
            await runAsync("ROLLBACK", [], db);
            res.status(STATUS_INVALID_REQUEST_DATA)
                .send(getErrorResponse([INVALID_REQUEST_DATA, WELL_FORMED, e.message]));
        }
    }

    async delete(db, req, res) {
        try {
            let tag_id = req.params.id;

            let stmt = "SELECT id FROM tags WHERE id = ?";
            let response = await getAsync(stmt, [tag_id], db);
            let tag_exists = response != null;
            if (tag_exists) {
                stmt = "DELETE FROM tags WHERE id = ?";
                await runAsync(stmt, [tag_id], db);
                res.status(STATUS_SUCCESSFULLY_DELETED)
                    .send([])
            } else {
                res.status(STATUS_NOT_FOUND)
                    .send(getErrorResponse([NOT_FOUND, TAG_ID_NOT_FOUND]))
            }
        } catch (e) {
            res.status(ERROR)
                .send(getErrorResponse([e.message]))
        }
    }
}

module.exports = TagController;
