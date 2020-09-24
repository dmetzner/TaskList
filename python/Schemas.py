from marshmallow import Schema, fields


class TagSchema(Schema):
    id = fields.Int(dump_only=True)
    name = fields.Str()
    description = fields.Str()
    creation_date = fields.DateTime()


tag_schema = TagSchema()
tags_schema = TagSchema(many=True)


class TaskSchema(Schema):
    id = fields.Int(dump_only=True)
    name = fields.Str()
    description = fields.Str()
    creation_date = fields.DateTime()
    tags = (fields.Nested('TagSchema', many=True, only=["name"]))


task_schema = TaskSchema()
tasks_schema = TaskSchema(many=True)
