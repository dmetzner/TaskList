<?php

namespace App;


use Illuminate\Database\Eloquent\Model;


class Tag extends Model
{

    public $timestamps = false;

    /**
     * The attributes that are mass assignable.
     *
     * @var array
     */
    protected $fillable = [
        'description', 'name',
    ];

    /**
     * The attributes excluded from the model's JSON form.
     *
     * @var array
     */
    protected $hidden = [];

    public function tasks()
    {
        return $this->belongsToMany('App\Task', 'task_tags');
    }

    public function toArray()
    {
        return [
            "creation_date" => $this->creation_date,
            "description" => $this->description,
            "id" => $this->id,
            "name" => $this->name,
        ];
    }

    // fill able fields
    const name = "name";
    const description = "description";
}
