export const MESSAGES = "messages";
export const NOT_FOUND = "Not Found";
export const INVALID_REQUEST_DATA = "Unprocessable Entity";
export const WELL_FORMED = "The request was well-formed but was unable to be followed due to semantic errors.";
export const BAD_TAGS = "Invalid escape characters in tags";
export const METHOD_NOT_ALLOWED = "Method Not Allowed";
export const METHOD_NOT_ALLOWED_URL = "The method is not allowed for the requested URL.";
export const URL_NOT_FOUND = "The requested URL was not found on the server.  " +
    "If you entered the URL manually please check your spelling and try again.";
export const TASK_ID_NOT_FOUND = "Task ID not found";
export const TAG_ID_NOT_FOUND = "Tag ID not found";

export const STATUS_SUCCESSFUL = 200;
export const STATUS_SUCCESSFULLY_CREATED = 201;
export const STATUS_SUCCESSFULLY_DELETED = 204;
export const STATUS_NOT_FOUND = 404;
export const STATUS_METHOD_NOT_ALLOWED = 405;
export const STATUS_INVALID_REQUEST_DATA = 422;


export function getErrorResponse(messages) {
    let error_response = {};
    error_response["messages"] = messages;
    return error_response;
}


export const ERROR = 422;
