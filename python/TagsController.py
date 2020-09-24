from Models import Tags
from Schemas import tags_schema, tag_schema
from flask import request, jsonify
from validator import *
from responses import *


def tg_get_all_tags():
    return jsonify(tags_schema.dump(Tags.query.all())[0])


def tg_get_tag(_id, db):
    tag = db.session.query(Tags).get(_id)
    if tag is not None:
        return jsonify(tag_schema.dump(tag)[0]), STATUS_SUCCESSFUL
    return jsonify(get_error_response([NOT_FOUND, TAG_ID_NOT_FOUND])), STATUS_NOT_FOUND


def tg_update_tag(_id, db):
    content = request.get_json()
    if not content:
        return jsonify(get_error_response([INVALID_REQUEST_DATA])), STATUS_INVALID_REQUEST_DATA

    name = content.get('name', None)
    description = content.get('description', None)

    if validate_name(name) or validate_description(description):
        return jsonify(get_error_response([INVALID_REQUEST_DATA, WELL_FORMED])), STATUS_INVALID_REQUEST_DATA

    tag = db.session.query(Tags).get(_id)
    if tag is None:
        return jsonify(get_error_response([NOT_FOUND, TAG_ID_NOT_FOUND])), STATUS_NOT_FOUND

    tag.name = name
    tag.description = description
    try:
        db.session.add(tag)
        db.session.commit()
    except:
        return jsonify(get_error_response([INVALID_REQUEST_DATA])), STATUS_INVALID_REQUEST_DATA
    return jsonify(tag_schema.dump(tag)[0]), STATUS_SUCCESSFUL


def tg_delete_tag(_id, db):
    tag = db.session.query(Tags).get(_id)
    if tag is None:
        return jsonify(get_error_response([NOT_FOUND, TAG_ID_NOT_FOUND])), STATUS_NOT_FOUND

    db.session.delete(tag)
    db.session.commit()
    return jsonify([]), STATUS_SUCCESSFULLY_DELETED
