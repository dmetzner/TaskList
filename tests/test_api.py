import requests
import json

BASE_URL = 'http://localhost:5000/'
headers = {'Content-Type': 'application/json' }

def test_index_page():
    url = BASE_URL
    resp = requests.get(url)
    assert resp.status_code == 200

def test_get_tasks_01():
    url = BASE_URL+'tasks'
    resp = requests.get(url, headers=headers)
    assert resp.status_code == 200
    resp_body = resp.json()
    assert resp_body == []

def test_get_tags_01():
    url = BASE_URL+'tags'
    resp = requests.get(url, headers=headers)
    assert resp.status_code == 200
    resp_body = resp.json()
    assert resp_body == []

def test_post_tasks_01():
    url = BASE_URL+'tasks'
    payload = {'name': 'Task 1', 'description': '', 'tags': ''}
    resp = requests.post(url, headers=headers, data=json.dumps(payload,indent=4))
    assert resp.status_code == 201
    resp_body = resp.json()
    has_task_schema(resp_body)
    assert resp_body['name'] == 'Task 1'
    assert resp_body['description'] == ''
    assert resp_body['tags'] == ''

def test_post_tasks_02():
    url = BASE_URL+'tasks'
    payload = {'name': 'Task 2', 'description': 'Second task', 'tags': 'high,asap, AB C'}
    resp = requests.post(url, headers=headers, data=json.dumps(payload,indent=4))
    assert resp.status_code == 201
    resp_body = resp.json()
    has_task_schema(resp_body)
    assert resp_body['name'] == 'Task 2'
    assert resp_body['description'] == 'Second task'
    assert resp_body['tags'] == ' AB C,asap,high'

def test_post_tasks_03():
    url = BASE_URL+'tasks'
    payload = {'name': '', 'description': '', 'tags': ''}
    resp = requests.post(url, headers=headers, data=json.dumps(payload,indent=4))
    assert resp.status_code == 422

def test_get_tasks_id_01():
    url = BASE_URL+'tasks/2'
    resp = requests.get(url, headers=headers)
    assert resp.status_code == 200
    resp_body = resp.json()
    has_task_schema(resp_body)
    assert resp_body['name'] == 'Task 2'
    assert resp_body['description'] == 'Second task'
    assert resp_body['tags'] == ' AB C,asap,high'

def test_put_tasks_id_01():
    url = BASE_URL+'tasks/2'
    payload = {'name': 'Task 2 updated', 'description': 'Second fancy task', 'tags': 'high,asap'}
    resp = requests.put(url, headers=headers, data=json.dumps(payload,indent=4))
    assert resp.status_code == 200
    resp_body = resp.json()
    has_task_schema(resp_body)
    assert resp_body['name'] == 'Task 2 updated'
    assert resp_body['description'] == 'Second fancy task'
    assert resp_body['tags'] == 'asap,high'

def test_put_tasks_id_02():
    url = BASE_URL+'tasks/77'
    payload = {'name': 'Updated', 'description': '', 'tags': ''}
    resp = requests.put(url, headers=headers)
    assert resp.status_code == 404

def test_put_tasks_id_03():
    url = BASE_URL+'tasks/1'
    payload = {'name': '', 'description': '', 'tags': ',,,,,,,,'}
    resp = requests.put(url, headers=headers, data=json.dumps(payload,indent=4))
    assert resp.status_code == 422

def test_get_tasks_02():
    url = BASE_URL+'tasks'
    resp = requests.get(url, headers=headers)
    assert resp.status_code == 200
    resp_body = resp.json()
    assert len(resp_body) == 2
    has_task_schema(resp_body[0])
    assert resp_body[0]['name'] == 'Task 1'
    assert resp_body[0]['description'] == ''
    assert resp_body[0]['tags'] == ''
    has_task_schema(resp_body[1])
    assert resp_body[1]['name'] == 'Task 2 updated'
    assert resp_body[1]['description'] == 'Second fancy task'
    assert resp_body[1]['tags'] == 'asap,high'

def test_get_tags_02():
    url = BASE_URL+'tags'
    resp = requests.get(url, headers=headers)
    assert resp.status_code == 200
    resp_body = resp.json()
    assert len(resp_body) == 3
    has_tag_schema(resp_body[0])
    assert resp_body[0]['name'] == 'high'
    assert resp_body[0]['description'] == ''
    has_tag_schema(resp_body[1])
    assert resp_body[1]['name'] == 'asap'
    assert resp_body[1]['description'] == ''
    has_tag_schema(resp_body[2])
    assert resp_body[2]['name'] == ' AB C'
    assert resp_body[2]['description'] == ''

def test_get_tags_id_01():
    url = BASE_URL+'tags/1'
    resp = requests.get(url, headers=headers)
    assert resp.status_code == 200
    resp_body = resp.json()
    has_tag_schema(resp_body)
    assert resp_body['name'] == 'high'
    assert resp_body['description'] == ''

def test_get_tags_id_02():
    url = BASE_URL+'tags/77'
    resp = requests.get(url, headers=headers)
    assert resp.status_code == 404

def test_put_tags_id_01():
    url = BASE_URL+'tags/2'
    payload = {'name': 'Updated Tag', 'description': 'fancy tag'}
    resp = requests.put(url, headers=headers, data=json.dumps(payload,indent=4))
    assert resp.status_code == 200
    resp_body = resp.json()
    has_tag_schema(resp_body)
    assert resp_body['name'] == 'Updated Tag'
    assert resp_body['description'] == 'fancy tag'

def test_get_tags_id_03():
    url = BASE_URL+'tags/2'
    resp = requests.get(url, headers=headers)
    assert resp.status_code == 200
    resp_body = resp.json()
    has_tag_schema(resp_body)
    assert resp_body['name'] == 'Updated Tag'
    assert resp_body['description'] == 'fancy tag'

def test_get_tasks_id_02():
    url = BASE_URL+'tasks/2'
    resp = requests.get(url, headers=headers)
    assert resp.status_code == 200
    resp_body = resp.json()
    has_task_schema(resp_body)
    assert resp_body['name'] == 'Task 2 updated'
    assert resp_body['description'] == 'Second fancy task'
    assert resp_body['tags'] == 'Updated Tag,high'

def test_put_tags_id_02():
    url = BASE_URL+'tags/77'
    payload = {'name': 'Updated', 'description': ''}
    resp = requests.put(url, headers=headers)
    assert resp.status_code == 404

def test_put_tags_id_03():
    url = BASE_URL+'tags/2'
    payload = {'name': '', 'description': ''}
    resp = requests.put(url, headers=headers)
    assert resp.status_code == 422

def test_get_tasks_03():
    url = BASE_URL+'tasks'
    resp = requests.get(url, headers=headers)
    assert resp.status_code == 200
    resp_body = resp.json()
    assert len(resp_body) == 2
    has_task_schema(resp_body[0])
    assert resp_body[0]['name'] == 'Task 1'
    assert resp_body[0]['description'] == ''
    assert resp_body[0]['tags'] == ''
    has_task_schema(resp_body[1])
    assert resp_body[1]['name'] == 'Task 2 updated'
    assert resp_body[1]['description'] == 'Second fancy task'
    assert resp_body[1]['tags'] == 'Updated Tag,high'

def test_get_tags_03():
    url = BASE_URL+'tags'
    resp = requests.get(url, headers=headers)
    assert resp.status_code == 200
    resp_body = resp.json()
    assert len(resp_body) == 3
    has_tag_schema(resp_body[0])
    assert resp_body[0]['name'] == 'high'
    assert resp_body[0]['description'] == ''
    has_tag_schema(resp_body[1])
    assert resp_body[1]['name'] == 'Updated Tag'
    assert resp_body[1]['description'] == 'fancy tag'
    has_tag_schema(resp_body[2])
    assert resp_body[2]['name'] == ' AB C'
    assert resp_body[2]['description'] == ''

def test_delete_tags_id_01():
    url = BASE_URL+'tags/1'
    resp = requests.delete(url, headers=headers)
    assert resp.status_code == 204

def test_delete_tags_id_02():
    url = BASE_URL+'tags/77'
    resp = requests.delete(url, headers=headers)
    assert resp.status_code == 404

def test_get_tags_id_04():
    url = BASE_URL+'tags/1'
    resp = requests.get(url, headers=headers)
    assert resp.status_code == 404

def test_get_tasks_id_03():
    url = BASE_URL+'tasks/2'
    resp = requests.get(url, headers=headers)
    assert resp.status_code == 200
    resp_body = resp.json()
    has_task_schema(resp_body)
    assert resp_body['name'] == 'Task 2 updated'
    assert resp_body['description'] == 'Second fancy task'
    assert resp_body['tags'] == 'Updated Tag'

def test_delete_tags_id_03():
    url = BASE_URL+'tags/2'
    resp = requests.delete(url, headers=headers)
    assert resp.status_code == 204

def test_get_tasks_id_04():
    url = BASE_URL+'tasks/2'
    resp = requests.get(url, headers=headers)
    assert resp.status_code == 200
    resp_body = resp.json()
    has_task_schema(resp_body)
    assert resp_body['name'] == 'Task 2 updated'
    assert resp_body['description'] == 'Second fancy task'
    assert resp_body['tags'] == ''

def test_delete_tags_id_04():
    url = BASE_URL+'tags/3'
    resp = requests.delete(url, headers=headers)
    assert resp.status_code == 204

def test_get_tags_02():
    url = BASE_URL+'tags'
    resp = requests.get(url, headers=headers)
    assert resp.status_code == 200
    resp_body = resp.json()
    assert resp_body == []

def test_delete_tasks_id_01():
    url = BASE_URL+'tasks/1'
    resp = requests.delete(url, headers=headers)
    assert resp.status_code == 204

def test_delete_tasks_id_02():
    url = BASE_URL+'tasks/77'
    resp = requests.delete(url, headers=headers)
    assert resp.status_code == 404

def test_delete_tasks_id_03():
    url = BASE_URL+'tasks/2'
    resp = requests.delete(url, headers=headers)
    assert resp.status_code == 204

def test_get_tasks_04():
    url = BASE_URL+'tasks'
    resp = requests.get(url, headers=headers)
    assert resp.status_code == 200
    resp_body = resp.json()
    assert resp_body == []


def has_tag_schema(tag):
    assert len(tag) == 4
    assert 'id' in tag
    assert 'creation_date' in tag
    assert 'name' in tag
    assert 'description' in tag

def has_task_schema(task):
    assert len(task) == 5
    assert 'id' in task
    assert 'creation_date' in task
    assert 'name' in task
    assert 'description' in task
    assert 'tags' in task
