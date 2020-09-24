<?php

namespace App\Http\Controllers;

use App\Http\Responses\ApiResponse;
use Illuminate\Validation\Validator;
use Laravel\Lumen\Routing\Controller as BaseController;

class Controller extends BaseController
{
    /**
     * {@inheritdoc}
     */
    protected function formatValidationErrors(Validator $validator)
    {
        $msg = [];
        array_push($msg, ApiResponse::INVALID_REQUEST);
        foreach ($validator->errors()->all() as $e) {
            array_push($msg, $e);
        }

        return array(ApiResponse::MESSAGES => $msg);
    }
}
