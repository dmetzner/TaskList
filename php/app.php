<?php

/*
|--------------------------------------------------------------------------
| Create The Application
|--------------------------------------------------------------------------
|
| First we need to get an application instance. This creates an instance
| of the application / container and bootstraps the application so it
| is ready to receive HTTP / Console requests from the environment.
|
*/


// dynamic sqlite db hack ;)
// Lets just build the env file which is needed for the configuration
try {
    $db = getenv('DATABASE');   // from command line argv
    if (empty($db)) {
        die("DATABASE not specified");
    }
    if (!file_exists($db)) {             // create file if not exists
        $f = fopen($db, 'w') or die("Can't create file");
        fclose($f);
    }
    $fp = fopen('.env', 'w');
    fwrite($fp, "APP_ENV=local\n" .
        "APP_DEBUG=true\n" .
        "APP_KEY=SASE\n" .
        "APP_TIMEZONE=UTC\n" .
        "DB_CONNECTION=sqlite\n" .
        "DB_DATABASE=$db\n" .
        "CACHE_DRIVER=file\n" .
        "QUEUE_DRIVER=sync\n");
    fclose($fp);
} catch (Exception $e) {
    die("Init db failed");
}

$app = require __DIR__ . '/bootstrap/app.php';

/*
|--------------------------------------------------------------------------
| Run The Application
|--------------------------------------------------------------------------
|
| Once we have the application, we can handle the incoming request
| through the kernel, and send the associated response back to
| the client's browser allowing them to enjoy the creative
| and wonderful application we have prepared for them.
|
*/

$app->run();

