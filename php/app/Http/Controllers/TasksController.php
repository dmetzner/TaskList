<?php

namespace App\Http\Controllers;

use App\Http\Responses\CreationResponse;
use App\Http\Responses\DeletionResponse;
use App\Http\Responses\InvalidRequestResponse;
use App\Http\Responses\NotFoundResponse;
use App\Http\Responses\SuccessResponse;
use App\Tag;
use App\Task;
use Exception;
use Illuminate\Database\Eloquent\ModelNotFoundException;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\DB;
use Symfony\Component\HttpFoundation\Response;


class TasksController extends Controller
{
    //------------------------------------------------------------------------------------------------------------------
    // custom error messages
    //
    const TASK_ID_NOT_FOUND = "Task ID not found";

    //------------------------------------------------------------------------------------------------------------------
    // public API calls
    //
    public function showAllTasks(): Response
    {
        return (new SuccessResponse(Task::all()))->get();
    }

    public function addTask(Request $request): Response
    {
        // validate user input
        // sends 422 if fails -> look into controller
        $this->validate($request, [
            Task::name => 'required|string|min:1|max: 30',
            Task::description => 'present |string|     |max:100',
            Task::tags => 'present |string'
        ]);

        // parse and split tags!
        // parse and split tags! only if there are tags -> no tags is valid
        $all_tags = $request->input(Task::tags);
        $tags = [];
        if (strlen($all_tags) > 0) {
            $tags = $this->tagsParser($all_tags);
            if (!$tags) {
                return (new InvalidRequestResponse("Bad Tags"))->get();
            }
        }

        DB::beginTransaction();
        // create new task object
        try {
            $task = Task::query()->create([
                Task::name => $request->input(Task::name),
                Task::description => $request->input(Task::description)
            ]);
            foreach ($tags as $t) {
                try {
                    $tag = Tag::query()->where(Tag::name, '=', $t)->firstOrFail();
                } catch (ModelNotFoundException $e) {
                    $tag = Tag::query()->create([Tag::name => $t, Tag::description => ""]);
                }
                try {
                    $task->tags()->attach($tag);
                } catch (Exception $e) {
                    // should have the same effect as insert or ignore ;)
                }
            }
            $task = Task::query()->findOrFail($task->id); // get again from db cause we need the creation date
            DB::commit();
        } catch (Exception $e) {
            DB::rollBack();
            return (new InvalidRequestResponse($e->getMessage()))->get();
        }
        return (new CreationResponse($task))->get();
    }

    public function deleteTask(int $id): Response
    {
        try {
            Task::query()->findOrFail($id)->delete();
        } catch (Exception $e) {
            return (new NotFoundResponse(self::TASK_ID_NOT_FOUND))->get();
        }
        return (new DeletionResponse())->get();
    }

    public function showTask(int $id): Response
    {
        try {
            $task = Task::query()->findOrFail($id);
        } catch (ModelNotFoundException $e) {
            return (new NotFoundResponse(self::TASK_ID_NOT_FOUND))->get();
        }
        return (new SuccessResponse($task))->get();
    }

    public function updateTask(int $id, Request $request): Response
    {
        try {
            $task = Task::query()->findOrFail($id);
        } catch (ModelNotFoundException $e) {
            return (new NotFoundResponse(self::TASK_ID_NOT_FOUND))->get();
        }

        $this->validateUserInput($request);

        // parse and split tags! only if there are tags -> no tags is valid
        $all_tags = $request->input(Task::tags);
        $tags = [];
        if (strlen($all_tags) > 0) {
            $tags = $this->tagsParser($all_tags);
            if (!$tags) {
                return (new InvalidRequestResponse("Bad Tags"))->get();
            }
        }

        DB::beginTransaction();
        try {
            $task->update([
                Task::name => $request->input(Task::name),
                Task::description => $request->input(Task::description),
                Task::tags => $request->input(Task::tags)
            ]);

            $task->tags()->detach();
            foreach ($tags as $t) {
                try {
                    $tag = Tag::query()->where(Tag::name, '=', $t)->firstOrFail();
                } catch (ModelNotFoundException $e) {
                    $tag = Tag::query()->create([Tag::name => $t, Tag::description => ""]);
                }
                try {
                    $task->tags()->attach($tag);
                } catch (Exception $e) {
                    // should have the same effect as insert or ignore ;)
                }

            }
            DB::commit();
        } catch (Exception $e) {
            DB::rollBack();
            return (new InvalidRequestResponse($e))->get();
        }
        return (new SuccessResponse($task))->get();
    }


    //------------------------------------------------------------------------------------------------------------------
    // Helper methods
    //
    public static function escapeTag(String $tag): string
    {
        $delimiter = ",";
        $escape_sign = "%";
        $escaped_tag = "";

        foreach (str_split($tag) as $c) {
            if ($c === $escape_sign || $c === $delimiter) {
                $escaped_tag .= $escape_sign;
                $escaped_tag .= $c;
            } else {
                $escaped_tag .= $c;
            }
        }
        return $escaped_tag;
    }

    private function tagsParser(String $tags_as_string): ?array
    {
        // init data structures
        $tags = [];
        $tag = "";
        $delimiter = ",";
        $escape_sign = "%";
        $escaped = false;

        for ($i = 0; $i < strlen($tags_as_string); $i++) {
            $c = $tags_as_string[$i];
            if ($escaped) {
                if ($c === $escape_sign || $c === $delimiter) {
                    // if escaped -> only allow escaped chars!
                    $tag .= $c;
                } else {
                    // error!
                    break;
                }
                $escaped = false;
            } else {
                if ($c === $escape_sign) {
                    $escaped = true;
                } else if ($c === $delimiter) {
                    // add tag and start a new one
                    array_push($tags, $tag);
                    $tag = "";
                } else {
                    $tag .= $c;
                }
            }
        }
        // still escaped? we had an error!
        if ($escaped) {
            return NULL;
        }
        // also add the last tag to the tags
        array_push($tags, $tag);

        foreach ($tags as $tag) {
            if (strlen($tag) < 1 || strlen($tag) > 30) {
                return NULL;
            }
        }

        return $tags;
    }

    private function validateUserInput(Request $request): void
    {
        // validate user input -> sends 422 if fails -> look into controller
        $this->validate($request, [
            Task::name => 'required|string|min:1|max: 30',
            Task::description => 'present |string|     |max:100',
            Task::tags => 'present |string'
        ]);
    }
}
