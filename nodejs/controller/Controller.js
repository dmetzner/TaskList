import {
    getErrorResponse,
    METHOD_NOT_ALLOWED, METHOD_NOT_ALLOWED_URL,
    NOT_FOUND,
    STATUS_METHOD_NOT_ALLOWED, STATUS_NOT_FOUND, URL_NOT_FOUND
} from "./Responses";

class Controller {

    handleMethodNotAllowed(res) {
        res.status(STATUS_METHOD_NOT_ALLOWED)
            .send(getErrorResponse([METHOD_NOT_ALLOWED, METHOD_NOT_ALLOWED_URL]))
    }

    handleUrlNotFound(res) {
        res.status(STATUS_NOT_FOUND)
            .send(getErrorResponse([NOT_FOUND, URL_NOT_FOUND]))
    }
}

module.exports = Controller;


