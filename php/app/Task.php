<?php

namespace App;

use App\Http\Controllers\TasksController;
use Illuminate\Database\Eloquent\Model;


class Task extends Model
{
    public $timestamps = false;

    /**
     * The attributes that are mass assignable.
     *
     * @var array
     */
    protected $fillable = [
        'description', 'name'
    ];

    /**
     * The attributes excluded from the model's JSON form.
     *
     * @var array
     */
    protected $hidden = [];


    public function tags()
    {
        return $this->belongsToMany('App\Tag', 'task_tags');
    }

    /**
     * create custom json response to fit the api
     */
    public function toArray()
    {
        $tags = $this->tags()->get();
        $tag_names = [];
        foreach ($tags as $tag) {

            array_push($tag_names, TasksController::escapeTag($tag->name));
        }

        sort($tag_names);

        return [
            "creation_date" => $this->creation_date,
            "description" => $this->description,
            "id" => $this->id,
            "name" => $this->name,
            "tags" => implode(",", $tag_names)
        ];
    }

    // request fields
    const name = "name";
    const description = "description";
    const tags = "tags";
}
