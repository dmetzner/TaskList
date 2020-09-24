
MESSAGES = "messages"
NOT_FOUND = "Not Found"
INVALID_REQUEST_DATA = "Unprocessable Entity"
WELL_FORMED = "The request was well-formed but was unable to be followed due to semantic errors."
BAD_TAGS = "Invalid escape characters in tags"
METHOD_NOT_ALLOWED = "Method Not Allowed"
METHOD_NOT_ALLOWED_URL = "The method is not allowed for the requested URL."
URL_NOT_FOUND = "The requested URL was not found on the server.  " \
                "If you entered the URL manually please check your spelling and try again."
TASK_ID_NOT_FOUND = "Task ID not found"
TAG_ID_NOT_FOUND = "Tag ID not found"

STATUS_SUCCESSFUL = 200
STATUS_SUCCESSFULLY_CREATED = 201
STATUS_SUCCESSFULLY_DELETED = 204
STATUS_NOT_FOUND = 404
STATUS_METHOD_NOT_ALLOWED = 405
STATUS_INVALID_REQUEST_DATA = 422


def get_error_response(messages):
    error_response = {}
    error_response["messages"] = messages
    return error_response
