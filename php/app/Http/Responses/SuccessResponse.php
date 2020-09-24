<?php
/**
 * Created by PhpStorm.
 * User: sasd
 * Date: 09.01.19
 * Time: 20:27
 */

namespace App\Http\Responses;


class SuccessResponse extends ApiResponse
{
    private $array;

    function __construct($array)
    {
        $this->array = $array;
    }

    public function get()
    {
        return response(
            $this->array,
            self::STATUS_SUCCESSFUL
        );
    }
}
