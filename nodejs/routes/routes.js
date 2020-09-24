import {STATUS_SUCCESSFUL} from "../controller/Responses";

module.exports = function (app, db) {

    const Controller = require("../controller/Controller");
    const TagController = require("../controller/TagController");
    const TaskController = require("../controller/TaskController");

    const controller = new Controller();
    const tagController = new TagController();
    const taskController = new TaskController();

    let path = require('path');

    app.get("/", function (req, res) {
        res.status(STATUS_SUCCESSFUL).sendFile(
            path.resolve("view/index.html")
        );
    });


    // tags
    app.get("/tags", function (req, res) {
        db.serialize(function () {
            tagController.showAll(db, res)
        });
    });
    app.all('/tags', function (req, res) {
        controller.handleMethodNotAllowed(res);
    });

    app.get("/tags/:id(\\d+)", function (req, res) {
        db.serialize(function () {
            tagController.show(db, req, res)
        });
    });

    app.delete("/tags/:id(\\d+)", function (req, res) {
        db.serialize(function () {
            tagController.delete(db, req, res)
        });
    });

    app.put("/tags/:id(\\d+)", function (req, res) {
        db.serialize(function () {
            tagController.update(db, req, res)
        });
    });
    app.all('/tags/:id(\\d+)', function (req, res) {
        controller.handleMethodNotAllowed(res);
    });

    // tasks
    app.get("/tasks", function (req, res) {
        db.serialize(function () {
            taskController.showAll(db, res)
        });
    });

    app.post("/tasks", function (req, res) {
        db.serialize(function () {
            taskController.add(db, req, res)
        });
    });
    app.all('/tasks', function (req, res) {
        controller.handleMethodNotAllowed(res);
    });

    app.get("/tasks/:id(\\d+)", function (req, res) {
        db.serialize(function () {
            taskController.show(db, req, res)
        });
    });

    app.delete("/tasks/:id(\\d+)", function (req, res) {
        db.serialize(function () {
            taskController.delete(db, req, res)
        });
    });

    app.put("/tasks/:id(\\d+)", function (req, res) {
        db.serialize(function () {
            taskController.update(db, req, res)
        });
    });
    app.all('/tasks/:id(\\d+)', function (req, res) {
        controller.handleMethodNotAllowed(res);
    });

    // fallback
    app.all('*', function (req, res) {
        controller.handleUrlNotFound(res);
    });


};
