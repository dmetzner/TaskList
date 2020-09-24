<?php
/**
 * Created by PhpStorm.
 * User: sasd
 * Date: 09.01.19
 * Time: 20:32
 */

namespace App\Http\Responses;


class DeletionResponse extends ApiResponse
{
    public function get()
    {
        return response()->json(array(), self::STATUS_SUCCESSFULLY_DELETED);
    }
}
