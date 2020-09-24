<?php
/**
 * Created by PhpStorm.
 * User: sasd
 * Date: 09.01.19
 * Time: 20:29
 */

namespace App\Http\Responses;


class NotFoundResponse extends ApiResponse
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
                    self::NOT_FOUND,
                    $this->message
                )
            ),
            self::STATUS_NOT_FOUND
        );
    }
}
