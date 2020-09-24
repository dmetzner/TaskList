<?php
/**
 * Created by PhpStorm.
 * User: sasd
 * Date: 09.01.19
 * Time: 20:27
 */

namespace App\Http\Responses;


class ApiResponse
{
    const MESSAGES = "messages";
    const NOT_FOUND = "Not Found";
    const INVALID_REQUEST_DATA = "Unprocessable Entity";
    const WELL_FORMED = "The request was well-formed but was unable to be followed due to semantic errors.";
    const BAD_TAGS = "Invalid escape characters in tags";

    const INVALID_REQUEST = "Invalid Request";

    const TASK_ID_NOT_FOUND = "Task ID not found";
    const TAG_ID_NOT_FOUND = "Tag ID not found";

    const URL_NOT_FOUND = "The requested URL was not found on the server.  If you entered the URL manually please check your spelling and try again.";

    const METHOD_NOT_ALLOWED_URL = "The method is not allowed for the requested URL.";

    const SUCCESSFUL = "OK";
    const CREATED = "CREATED";
    const NO_CONTENT = "NO CONTENT";
    const METHOD_NOT_ALLOWED = "Method Not Allowed";

    const STATUS_SUCCESSFUL = 200;
    const STATUS_SUCCESSFULLY_CREATED = 201;
    const STATUS_SUCCESSFULLY_DELETED = 204;
    const STATUS_NOT_FOUND = 404;
    const STATUS_METHOD_NOT_ALLOWED = 405;
    const STATUS_INVALID_REQUEST_DATA = 422;
}