<?php
/**
 * Created by PhpStorm.
 * User: sasd
 * Date: 09.01.19
 * Time: 20:33
 */

namespace App\Http\Responses;


class InvalidRequestResponse extends ApiResponse
{
    private $message;

    function __construct($message)
    {
        $this->message = $message;
    }

    public function get()
    {
        return response(
            array(
                self::MESSAGES => array(
                    self::INVALID_REQUEST_DATA,
                    self::WELL_FORMED,
                    $this->message
                )
            ),
            self::STATUS_INVALID_REQUEST_DATA
        );
    }
}
