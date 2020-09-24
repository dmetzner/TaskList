from Models import Tags, Tasks
from Schemas import tasks_schema, task_schema, tag_schema
from flask import request, jsonify
from validator import *
from responses import *


def tc_get_all_tasks():
    all_tasks = Tasks.query.all()
    all_tasks_json = tasks_schema.dump(Tasks.query.all())[0]
    for i, task in enumerate(all_tasks):
        all_tasks_json[i] = tc_update_task_with_tags(task)
    return jsonify(all_tasks_json), STATUS_SUCCESSFUL


def tc_add_task(db):

    content = request.get_json()
    if not content:
        return jsonify(get_error_response([INVALID_REQUEST_DATA])), STATUS_INVALID_REQUEST_DATA

    name = content.get('name', None)
    description = content.get('description', None)
    tags = content.get('tags', None)

    if validate_name(name) or validate_description(description) or validate_tags(tags):
        return jsonify(get_error_response([INVALID_REQUEST_DATA, WELL_FORMED])), STATUS_INVALID_REQUEST_DATA

    all_tags = None
    if len(tags) > 0:
        # only parse if there are tags -> no tags are allowed!
        all_tags = parse_tags(tags)
        if all_tags is None or validate_parsed_tags(all_tags):
            return jsonify(get_error_response([INVALID_REQUEST_DATA, WELL_FORMED])), STATUS_INVALID_REQUEST_DATA

    try:
        task = Tasks(name=name, description=description)
        db.session.add(task)
        if all_tags is not None:
            tc_add_task_tags(task, all_tags, db)
        db.session.commit()
        return jsonify(tc_update_task_with_tags(task)), STATUS_SUCCESSFULLY_CREATED

    except:
        db.session.rollback()

    return jsonify(get_error_response([INVALID_REQUEST_DATA, WELL_FORMED])), STATUS_INVALID_REQUEST_DATA


def tc_get_task(_id, db):
    task = db.session.query(Tasks).get(_id)
    if task is not None:
        return jsonify(tc_update_task_with_tags(task)), STATUS_SUCCESSFUL
    return jsonify(get_error_response([NOT_FOUND, TASK_ID_NOT_FOUND])), STATUS_NOT_FOUND


def tc_update_task(_id, db):
    content = request.get_json()
    if not content:
        return jsonify(get_error_response([INVALID_REQUEST_DATA])), STATUS_INVALID_REQUEST_DATA

    name = content.get('name', None)
    description = content.get('description', None)
    tags = content.get('tags', None)

    if validate_name(name) or validate_description(description) or validate_tags(tags):
        return jsonify(get_error_response([INVALID_REQUEST_DATA, WELL_FORMED])), STATUS_INVALID_REQUEST_DATA

    all_tags = None
    if len(tags) > 0:
        all_tags = parse_tags(tags)
        if all_tags is None or validate_parsed_tags(all_tags):
            return jsonify(get_error_response([INVALID_REQUEST_DATA, WELL_FORMED])), STATUS_INVALID_REQUEST_DATA

    try:
        task = db.session.query(Tasks).get(_id)
        if task is None:
            return jsonify(get_error_response([NOT_FOUND, TASK_ID_NOT_FOUND])), STATUS_NOT_FOUND
        task.name = name
        task.description = description
        db.session.add(task)
        task.tags[:] = []  # python syntax for clearing a list
        if all_tags is not None:
            tc_add_task_tags(task, all_tags, db)
        db.session.commit()
        return jsonify(tc_update_task_with_tags(task)), STATUS_SUCCESSFUL

    except:
        db.session.rollback()

    return jsonify(get_error_response([INVALID_REQUEST_DATA, WELL_FORMED])), STATUS_INVALID_REQUEST_DATA


def tc_delete_task(_id, db):
    task = db.session.query(Tasks).get(_id)
    if task is None:
        return jsonify(get_error_response([NOT_FOUND, TASK_ID_NOT_FOUND])), STATUS_NOT_FOUND

    db.session.delete(task)
    db.session.commit()
    return jsonify([]), STATUS_SUCCESSFULLY_DELETED


########################################################################################################################
# Helper Functions
#
def tc_add_task_tags(task, all_tags, db):
    for tag_name in all_tags:
        tag = db.session.query(Tags).filter(Tags.name == tag_name).first()
        if tag is None:
            tag = Tags(name=tag_name, description="")
        db.session.add(tag)
        task.tags.append(tag)
        db.session.add(task)


def tc_update_task_with_tags(task):
    # first sort
    all_tags = []
    for tag in task.tags:
        all_tags.append(tag_schema.dump(tag)[0]['name'])
    all_tags.sort()
    # then escape and build final tags string
    all_tags_as_string = ""
    for i, tag_name in enumerate(all_tags):
        if i != 0:
            all_tags_as_string += delimiter
        tag_name = escape_tag(tag_name)
        all_tags_as_string += tag_name
    task_json = task_schema.dump(task)[0]
    task_json['tags'] = all_tags_as_string
    return task_json
