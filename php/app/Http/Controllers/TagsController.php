<?php

namespace App\Http\Controllers;

use App\Http\Responses\DeletionResponse;
use App\Http\Responses\InvalidRequestResponse;
use App\Http\Responses\NotFoundResponse;
use App\Http\Responses\SuccessResponse;
use App\Tag;
use Dotenv\Exception\ValidationException;
use Illuminate\Database\Eloquent\ModelNotFoundException;
use Illuminate\Http\Request;
use Symfony\Component\HttpFoundation\Response;


class TagsController extends Controller
{
    public function __construct()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    // custom error messages
    //
    const TAG_ID_NOT_FOUND = "Tag ID not found";


    //------------------------------------------------------------------------------------------------------------------
    // public API calls
    //
    public function showAllTags(): Response
    {
        return (new SuccessResponse(Tag::all()))->get();
    }

    public function deleteTag($id): Response
    {
        try {
            Tag::query()->findOrFail($id)->delete();
        } catch (\Exception $e) {
            return (new NotFoundResponse(self::TAG_ID_NOT_FOUND))->get();
        }
        return (new DeletionResponse())->get();
    }

    public function showTag($id): Response
    {
        try {
            $tag = Tag::query()->findOrFail($id);
        } catch (ModelNotFoundException $e) {
            return (new NotFoundResponse(self::TAG_ID_NOT_FOUND))->get();
        }
        return (new SuccessResponse($tag))->get();
    }

    public function updateTag(int $id, Request $request): Response
    {
        try {
            /** @var Tag $tag */
            $tag = Tag::query()->findOrFail($id);
        } catch (ModelNotFoundException $e) {
            return (new NotFoundResponse(self::TAG_ID_NOT_FOUND))->get();
        }

        // sends 422 if fails -> look into controller
        $this->validate($request, [
            Tag::name => 'required|string|min:1|max: 30|unique:tags,name,' . $tag->id,
            Tag::description => 'present |string|min:0|max:100|',
        ]);

        try {
            $tag->update([
                Tag::name => $request->input(Tag::name),
                Tag::description => $request->input(Tag::description)
            ]);
        } catch (ModelNotFoundException $e) {
            return (new InvalidRequestResponse($e))->get();
        }
        return (new SuccessResponse($tag))->get();
    }
}
