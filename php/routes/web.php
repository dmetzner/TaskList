<?php

/*
|--------------------------------------------------------------------------
| Application Routes
|--------------------------------------------------------------------------
|
| Here is where you can register all of the routes for an application.
| It is a breeze. Simply tell Lumen the URIs it should respond to
| and give it the Closure to call when that URI is requested.
|
*/

use Illuminate\Database\Schema\Blueprint;
use Illuminate\Support\Facades\DB;
use Illuminate\Support\Facades\Schema;
use Symfony\Component\HttpFoundation\BinaryFileResponse;

// init the db on first request
initDB();

$router->get('/', function () use ($router) {
    return view('index');
});

// tags
$router->get('tags', ['uses' => 'TagsController@showAllTags']);
$router->delete('tags/{id}', ['uses' => 'TagsController@deleteTag']);
$router->get('tags/{id}', ['uses' => 'TagsController@showTag']);
$router->put('tags/{id}', ['uses' => 'TagsController@updateTag']);


// task
$router->get('/tasks', ['uses' => 'TasksController@showAllTasks']);
$router->post('/tasks', ['uses' => 'TasksController@addTask']);
$router->delete('/tasks/{id}', ['uses' => 'TasksController@deleteTask']);
$router->get('/tasks/{id}', ['uses' => 'TasksController@showTask']);
$router->put('/tasks/{id}', ['uses' => 'TasksController@updateTask']);

// Lumen has no any or fallback
// workaround -> Exception handler registered instances of 400, 404, 405


function initDb()
{

    Schema::enableForeignKeyConstraints();

    if (!Schema::hasTable('tasks')) {
        Schema::create('tasks', function (Blueprint $table) {
            $table->increments('id');
            $table->string('name', 30)->nullable(false);
            $table->string('description', 100)->nullable(false);
            $table->dateTime('creation_date')->default(DB::raw('CURRENT_TIMESTAMP'));
        });
    }

    if (!Schema::hasTable('tags')) {
        Schema::create('tags', function (Blueprint $table) {
            $table->increments('id');
            $table->string('name', 30)->unique()->nullable(false);
            $table->string('description', 100)->nullable(false);
            $table->dateTime('creation_date')->default(DB::raw('CURRENT_TIMESTAMP'));
        });
    }

    if (!Schema::hasTable('task_tags')) {
        Schema::create('task_tags', function (Blueprint $table) {
            $table->integer('task_id')->nullable(false);
            $table->integer('tag_id')->nullable(false);
            $table->primary(['task_id', 'tag_id']);
            $table->foreign('task_id')->references('id')->on('tasks')->onDelete('cascade');
            $table->foreign('tag_id')->references('id')->on('tags')->onDelete('cascade');
        });
    }
}
