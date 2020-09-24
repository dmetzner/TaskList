const AJAX_HTTP_REQUEST__DONE = 4;
const HTTP_RESPONSE__OK = 200;
const HTTP_RESPONSE__DISAPPOINTED = 299;

reload();

function reload() {
    getTasks();
    getTags();
}

function getTasks()
{
    let xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function() {
        if (this.readyState === AJAX_HTTP_REQUEST__DONE && this.status === HTTP_RESPONSE__OK) {
            let json = JSON.parse(this.responseText);
            json2TaskTable(json);
        }
    };
    xhr.open("GET", "/tasks", true);
    xhr.send();
}

function getTags()
{
    let xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function() {
        if (this.readyState === AJAX_HTTP_REQUEST__DONE && this.status === HTTP_RESPONSE__OK) {
            let json = JSON.parse(this.responseText);
            json2TagTable(json);
        }
    };
    xhr.open("GET", "/tags", true);
    xhr.send();
}

function json2TaskTable(json) {
    const htmlTable = document.getElementById('taskTableBody');
    htmlTable.innerHTML = "";
    json.map(function(row) {
        const id = row['id'];
        if (!Number.isInteger(id)) {
            // Prevent XSS via id
            console.debug("Invalid Id: " + id);
            return;
        }
        let newRow = '<tr>';
        newRow += '<td id="taskId' + id + '"></td>';
        newRow += '<td id="taskCreat' + id + '"></td>';
        newRow += '<td><input type="text" name="taskName' + id + '" size="8"/></td>';
        newRow += '<td><input type="text" name="taskDesc' + id + '" size="16"/></td>';
        newRow += '<td><input type="text" name="taskTags' + id + '" size="10"/></td>';
        newRow += '<td><input type="submit" value="Update" onclick="updateTask(' + id + ')" class="button-secondary"/></form></td>';
        newRow += '<td><input type="submit" value="Delete" onclick="deleteTask(' + id + ')" class="button-error"/></form></td>';
        newRow += '</tr>';

        // Insert new row into DOM
        htmlTable.insertAdjacentHTML('beforeend', newRow);

        // Now we can safely insert values without XSS
        document.getElementById('taskId' + id).textContent = row['id'];
        document.getElementById('taskCreat' + id).textContent = row['creation_date'];
        document.getElementsByName('taskName' + id).item(0).value = row['name'];
        document.getElementsByName('taskDesc' + id).item(0).value = row['description'];
        document.getElementsByName('taskTags' + id).item(0).value = row['tags'];
    });
}

function json2TagTable(json) {
    const htmlTable = document.getElementById('tagTableBody');
    htmlTable.innerHTML = "";
    json.map(function(row) {
        const id = row['id'];
        if (!Number.isInteger(id)) {
            // Prevent XSS via id
            console.debug("Invalid Id: " + id);
            return;
        }
        let newRow =  '<tr>';
        newRow += '<td id="tagId' + id + '"></td>';
        newRow += '<td id="tagCreat' + id + '"></td>';
        newRow += '<td><input type="text" name="tagName' + id + '" size="8"/></td>';
        newRow += '<td><input type="text" name="tagDesc' + id + '" size="34"/></td>';
        newRow += '<td><input type="submit" value="Update" onclick="updateTag(' + id + ')" class="button-secondary"/></form></td>';
        newRow += '<td><input type="submit" value="Delete" onclick="deleteTag(' + id + ')" class="button-error"/></form></td>';
        newRow += '</tr>';

        // Insert new row into DOM
        htmlTable.insertAdjacentHTML('beforeend', newRow);

        // Now we can safely insert values without XSS
        document.getElementById('tagId' + id).textContent = row['id'];
        document.getElementById('tagCreat' + id).textContent = row['creation_date'];
        document.getElementsByName('tagName' + id).item(0).value = row['name'];
        document.getElementsByName('tagDesc' + id).item(0).value = row['description'];
    });
}

function createTask() {
    const newName = document.getElementsByName("newTaskName").item(0).value;
    const newDesc = document.getElementsByName("newTaskDesc").item(0).value;
    const newTags = document.getElementsByName("newTaskTags").item(0).value;
    console.debug("Creating Task");
    sendJson("/tasks", "POST", { name: newName, description: newDesc, tags: newTags });
}

function updateTask(id) {
    const newName = document.getElementsByName("taskName" + id).item(0).value;
    const newDesc = document.getElementsByName("taskDesc" + id).item(0).value;
    const newTags = document.getElementsByName("taskTags" + id).item(0).value;
    console.debug("Updating Task " + id);
    sendJson("/tasks/" + id, "PUT", { name: newName, description: newDesc, tags: newTags });
}

function updateTag(id) {
    const newName = document.getElementsByName("tagName" + id).item(0).value;
    const newDesc = document.getElementsByName("tagDesc" + id).item(0).value;
    console.debug("Updating Tag " + id);
    sendJson("/tags/" + id, "PUT", { name: newName, description: newDesc });
}

function deleteTask(id) {
    console.debug("Deleting Task " + id);
    sendJson("/tasks/" + id, "DELETE", null);
}

function deleteTag(id) {
    console.debug("Deleting Tag " + id);
    sendJson("/tags/" + id, "DELETE", null);
}

function sendJson(endpoint, method, data) {
    const json = JSON.stringify(data);
    console.debug("Sending " + endpoint + ":" + method);
    console.debug("Json:" + json);

    const xhr = new XMLHttpRequest();
    xhr.open(method, endpoint, true);
    xhr.setRequestHeader('Content-type','application/json; charset=utf-8');
    xhr.onload = function () {
        if (requestFailed())
        {
            let resp = "Status " + this.status + ": " + this.statusText + "\n";
            resp += "Response: " + this.response;
            alert("Error: " + resp);
        }
        reload();
    }
    xhr.send(json);
}

function requestFailed()
{
    return this.status < HTTP_RESPONSE__OK || this.status > HTTP_RESPONSE__DISAPPOINTED
}
