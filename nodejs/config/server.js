import {getErrorResponse, INVALID_REQUEST_DATA, STATUS_INVALID_REQUEST_DATA} from "../controller/Responses";

let server = undefined;
let database = undefined;
initServer();

// register handler for graceful shutdown
process.on('SIGTERM', shutDown);
process.on('SIGINT', shutDown);


async function initServer() {
    try {
        let database_path = process.argv[2];
        let port = Number(process.argv[3]);

        // validate command line arguments
        if (database_path == null || port == null) {
            console.log("Invalid arguments: Provide sqlite3 db and port as arguments");
            shutDown();
        }

        // load modules
        let express = require("express");
        let bodyParser = require("body-parser");

        // Express config
        let app = express();
        app.use(bodyParser.json());
        app.use((error, req, res, next) => {
            if (error !== null) {

                // We are going to except delete request even with invalid body -> to make the trudel interrface work
                // just to be sure we also allow this for Get since it won't hurt us
                if (req && (req.method === "DELETE" || req.method === "GET")) {
                    return next();
                }

                // request parsing error happened
                return res.status(STATUS_INVALID_REQUEST_DATA)
                    .send(getErrorResponse(["aha", INVALID_REQUEST_DATA]));
            }
            return next();
        });

        // config and init database
        if (database_path.indexOf("..") !== -1) {
            console.log("Invalid db path, can't leave root");
            shutDown();
        } else {
            const database_config = require("./dbconfig");
            database = await database_config(database_path);
        }
        // Router config
        let routes = require("../routes/routes.js");
        routes(app, database);

        // init server listening
        server = app.listen(port, function () {
                let app_port = server.address().port;
                console.log("app running on port.", app_port);
            }
        ).on('error', (err) => {
            shutDown();
            console.log("Port " + port + " is not free or a valid port");
        });
    } catch (e) {
        console.log(e.message);
        shutDown();
    }
}


function shutDown() {
    console.log('\nReceived kill signal, shutting down gracefully');
    let server_off = false;
    let db_off = false;
    if (server) {
        server.close(() => {
            console.log('Closed all server connections');
            server_off = true;
        });
    }
    if (database) {
        database.close(() => {
            console.log('Closed database');
            db_off = true
        });
    }

    setInterval(() => {
        console.log('Shutdown successful');
        process.exit(0);
    }, 250);


    setTimeout(() => {
        console.log('Forced exit caused by timeout');
        process.exit(1);
    }, 10000);
}
