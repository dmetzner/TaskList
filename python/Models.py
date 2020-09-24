from datetime import datetime
from sqlalchemy import Table, Column, Integer, String, DATETIME, ForeignKey
from flask_sqlalchemy import SQLAlchemy

# needed to create in other filer, namely here ;)
db = SQLAlchemy()


task_tags = db.Table('task_tags',
                     Column('task_id', Integer, ForeignKey('tasks.id'), primary_key=True),
                     Column('tag_id', Integer, ForeignKey('tags.id'), primary_key=True)
                     )


class Tasks(db.Model):
    __tablename__ = 'tasks'
    id = Column(Integer, primary_key=True, autoincrement=True)
    name = Column(String(30), unique=False, nullable=False)
    description = Column(String(100), unique=False, nullable=False)
    creation_date = Column(DATETIME, nullable=False, default=datetime.utcnow())
    tags = db.relationship('Tags', secondary=task_tags, backref='tasks')


class Tags(db.Model):
    __tablename__ = 'tags'
    id = Column(Integer, primary_key=True, autoincrement=True)
    name = Column(String(30), unique=True, nullable=False)
    description = Column(String(100), unique=False, nullable=False)
    creation_date = Column(DATETIME, nullable=False, default=datetime.utcnow())
