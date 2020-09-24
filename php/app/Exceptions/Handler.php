<?php

namespace App\Exceptions;

use App\Http\Responses\ApiResponse;
use Exception;
use Illuminate\Http\Request;
use Illuminate\Http\Response;
use Illuminate\Validation\ValidationException;
use Illuminate\Auth\Access\AuthorizationException;
use Illuminate\Database\Eloquent\ModelNotFoundException;
use Laravel\Lumen\Exceptions\Handler as ExceptionHandler;
use Symfony\Component\HttpKernel\Exception\BadRequestHttpException;
use Symfony\Component\HttpKernel\Exception\HttpException;
use Symfony\Component\HttpKernel\Exception\MethodNotAllowedHttpException;
use Symfony\Component\HttpKernel\Exception\NotFoundHttpException;
use Throwable;

class Handler extends ExceptionHandler
{
    /**
     * A list of the exception types that should not be reported.
     *
     * @var array
     */
    protected $dontReport = [
        AuthorizationException::class,
        HttpException::class,
        ModelNotFoundException::class,
        ValidationException::class,
    ];

    /**
     * Report or log an exception.
     *
     * This is a great spot to send exceptions to Sentry, Bugsnag, etc.
     *
     * @param Throwable $e
     * @return void
     * @throws Exception
     */
    public function report(Throwable $e)
    {
        parent::report($e);
    }

    /**
     * Render an exception into an HTTP response.
     *
     * @param Request $request
     * @param Throwable $e
     *
     * @return Response
     * @throws Throwable
     */
    public function render($request, Throwable $e)
    {
        if ($e instanceof NotFoundHttpException) {
            return response(array(ApiResponse::MESSAGES => array(
                ApiResponse::NOT_FOUND,
                ApiResponse::URL_NOT_FOUND
            )), ApiResponse::STATUS_NOT_FOUND);
        } else if ($e instanceof MethodNotAllowedHttpException) {
            return response(array(ApiResponse::MESSAGES => array(
                ApiResponse::METHOD_NOT_ALLOWED,
                ApiResponse::METHOD_NOT_ALLOWED_URL
            )), ApiResponse::STATUS_METHOD_NOT_ALLOWED);
        } else if ($e instanceof BadRequestHttpException) {
            return response(array(ApiResponse::MESSAGES => array(
                ApiResponse::INVALID_REQUEST,
            )), ApiResponse::STATUS_INVALID_REQUEST_DATA);
        }

        // all has to be json! (stuff like not impl. etc!)
        return response(array(ApiResponse::MESSAGES => array(
            ApiResponse::INVALID_REQUEST,
        )), ApiResponse::STATUS_INVALID_REQUEST_DATA);
}
}
