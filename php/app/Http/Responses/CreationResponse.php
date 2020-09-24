<?php
/**
 * Created by PhpStorm.
 * User: sasd
 * Date: 09.01.19
 * Time: 20:33
 */

namespace App\Http\Responses;


class CreationResponse extends ApiResponse
{
    private $obj;

    function __construct($obj)
    {
        $this->obj = $obj;
    }

    public function get()
    {
        return response($this->obj, self::STATUS_SUCCESSFULLY_CREATED
        );
    }
}
