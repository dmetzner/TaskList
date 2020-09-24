from flask import Flask, jsonify, render_template
import sys
from responses import *
from Models import db
from TasksController import tc_add_task, tc_delete_task, tc_get_all_tasks, tc_get_task, tc_update_task
from TagsController import tg_get_tag, tg_get_all_tags, tg_delete_tag, tg_update_tag


########################################################################################################################
# create flask app
#
app = Flask(__name__)


########################################################################################################################
# routes
#

# Home
@app.route('/', methods=["GET"])
def index():  # searches in templates folder
    return render_template('index.html'), STATUS_SUCCESSFUL


# Tasks
@app.route('/tasks', methods=["GET"])
def get_all_tasks():
    return tc_get_all_tasks()


@app.route('/tasks', methods=["POST"])
def add_task():
    return tc_add_task(db)


@app.route('/tasks/<int:_id>', methods=["GET"])
def get_task(_id):
    return tc_get_task(_id, db)


@app.route('/tasks/<int:_id>', methods=["PUT"])
def update_task(_id):
    return tc_update_task(_id, db)


@app.route('/tasks/<int:_id>', methods=["DELETE"])
def delete_task(_id):
    return tc_delete_task(_id, db)


# Tags
@app.route('/tags', methods=["GET"])
def get_all_tags():
    return tg_get_all_tags()


@app.route('/tags/<int:_id>', methods=["GET"])
def get_tag(_id):
    return tg_get_tag(_id, db)


@app.route('/tags/<int:_id>', methods=["PUT"])
def update_tag(_id):
    return tg_update_tag(_id, db)


@app.route('/tags/<int:_id>', methods=["DELETE"])
def delete_tag(_id):
    return tg_delete_tag(_id, db)


# catch errors
@app.errorhandler(400)
def bad_request(e):
    return jsonify(get_error_response([INVALID_REQUEST_DATA])), STATUS_NOT_FOUND


@app.errorhandler(404)
def page_not_found(e):
    return jsonify(get_error_response([NOT_FOUND, URL_NOT_FOUND])), STATUS_NOT_FOUND


@app.errorhandler(405)
def method_not_allowed(e):
    return jsonify(get_error_response([METHOD_NOT_ALLOWED, METHOD_NOT_ALLOWED_URL])), STATUS_METHOD_NOT_ALLOWED


@app.errorhandler(500)
def server_error(e):
    return jsonify(get_error_response(["Error"])), 500


@app.errorhandler(Exception)
def unhandled_exception(e):
    return jsonify(get_error_response(["Error"])), 500


@app.errorhandler
def default_error_handler(error):
    return jsonify(get_error_response(["Error"])), 500


########################################################################################################################
# main
#
def main(argv):
    # validate args
    if len(argv) != 3:
        print("Provide a valid database path and port as argument!")
        exit(-1)

    try:
        db_path = argv[1]
        port = int(argv[2])

        if ".." in db_path or port == 0:
            raise ValueError

        # config app
        app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = True  # allow overhead ;)
        app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///' + db_path
        app.config['APP_PORT'] = port

        # init instead of creating -> allows us to have database models in a different file
        db.init_app(app)

        with app.app_context():
            # we can only create all if we have the app context! ( also needed cause of separate file! )
            db.create_all()

        app.run(debug=False, port=app.config['APP_PORT'])

    except ValueError:
        print("Provide a valid database path and port as argument!")
        exit(-1)
    except:
        print("Closing app")
        exit(-1)


if __name__ == '__main__':
    main(sys.argv)
