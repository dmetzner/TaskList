package main

import (
	"fmt"
	"github.com/gin-gonic/gin"
	"github.com/jinzhu/gorm"
	_ "github.com/jinzhu/gorm/dialects/sqlite"
	"net/http"
	"os"
	"sort"
	"time"
)



type Task struct {
	TaskId uint 	     	`json:"id"            gorm:"column:id;type:Integer;primary_key;AUTO_INCREMENT"`
	Name string				`json:"name"          gorm:"column:name;type:varchar(30);not null"`
	Description string		`json:"description"   gorm:"column:description;type:varchar(100);not null"`
	CreationDate time.Time	`json:"creation_date" gorm:"column:creation_date;DEFAULT:current_timestamp"`
	Tags []Tag              `json:"tags"          gorm:"many2many:task_tags;"`
}

type TaskResponse struct {
	TaskId uint 	     	`json:"id"            `
	Name string				`json:"name"          `
	Description string		`json:"description"   `
	CreationDate time.Time	`json:"creation_date" `
	Tags string              `json:"tags"         `
}

type TaskRequest struct {
	Name string `form:"name" json:"name" binding:"required"`
	Description *string `form:"description" json:"description" binding:"required"`
	Tags *string `form:"tags" json:"tags" binding:"required"`
}


type Tag struct {
	TagId uint 				`json:"id"            gorm:"column:id;type:Integer;primary_key;AUTO_INCREMENT"`
	Name string				`json:"name"          gorm:"column:name;type:varchar(30);not null;unique"`
	Description string		`json:"description"   gorm:"column:description;type:varchar(100);not null"`
	CreationDate time.Time	`json:"creation_date" gorm:"column:creation_date;DEFAULT:current_timestamp"`
}

type TaskTags struct {
	TaskId uint `json:"task_id" gorm:"column:task_id;type:Integer;primary_key;foreignkey:TaskId;"`
	TagId uint  `json:"task_id" gorm:"column:tag_id; type:Integer;primary_key;foreignkey:TagId"`
}


type TagRequest struct {
	Name string `form:"name" json:"name" binding:"required"`
	Description *string `form:"description" json:"description" binding:"required"`
}


var db *gorm.DB
var err error


func main() {

	if len(os.Args) != 3 {
		fmt.Println("Provide db_path and port as arguments")
		return
	}

	dbPath := os.Args[1]
	port := ":" + os.Args[2]

	db, err = gorm.Open("sqlite3", dbPath)
	if err != nil {
		panic("failed to connect database")
	}

	defer db.Close()

	db.AutoMigrate(&Task{}, &Tag{}, &TaskTags{})

	// Creates a gin router with default middleware:
	// logger and recovery (crash-free) middleware
	router := gin.Default()

	// define template pattern
	router.LoadHTMLGlob("*.html")

	// Define all routes
	router.GET("/", index)
	router.POST("/", methodNotAllowed)
	router.PUT("/", methodNotAllowed)
	router.DELETE("/", methodNotAllowed)
	router.PATCH("/", methodNotAllowed)
	router.HEAD("/", methodNotAllowed)
	router.OPTIONS("/", methodNotAllowed)

	router.GET("/tasks", getAllTasks)
	router.POST("/tasks", addTask)
	router.PUT("/tasks", methodNotAllowed)
	router.DELETE("/tasks", methodNotAllowed)
	router.PATCH("/tasks", methodNotAllowed)
	router.HEAD("/tasks", methodNotAllowed)
	router.OPTIONS("/tasks", methodNotAllowed)


	router.GET("/tags", getAllTags)
	router.POST("/tags", methodNotAllowed)
	router.PUT("/tags", methodNotAllowed)
	router.DELETE("/tags", methodNotAllowed)
	router.PATCH("/tags", methodNotAllowed)
	router.HEAD("/tags", methodNotAllowed)
	router.OPTIONS("/tags", methodNotAllowed)


	router.PUT("/tasks/:id", updateTask)
	router.DELETE("/tasks/:id", deleteTask)
	router.GET("/tasks/:id", getTask)
	router.PATCH("/tasks/:id", methodNotAllowed)
	router.HEAD("/tasks/:id", methodNotAllowed)
	router.OPTIONS("/tasks/:id", methodNotAllowed)
	router.POST("/tasks/:id", methodNotAllowed)

	router.GET("/tags/:id", getTag)
	router.PUT("/tags/:id", updateTag)
	router.DELETE("/tags/:id", deleteTag)
	router.PATCH("/tags/:id", methodNotAllowed)
	router.HEAD("/tags/:id", methodNotAllowed)
	router.OPTIONS("/tags/:id", methodNotAllowed)
	router.POST("/tags/:id", methodNotAllowed)

	// catch 404
	router.NoRoute(func(c *gin.Context) {
		urlNotFoundResponse(c)
	})

	// catch 405
	router.NoMethod(func(c *gin.Context) {
		methodNotAllowed(c)
	})

	// server the router
	if err := router.Run(port); err != nil {
		fmt.Printf(err.Error())
	}
}



func index (c *gin.Context) {
	c.HTML(http.StatusOK, "index.html", nil)
}

func getAllTasks (c *gin.Context) {
	var tasks []Task
	db.Preload("Tags").Find(&tasks)
	var resp []TaskResponse
	for _, task := range tasks {
		resp = append(resp, getTaskResponse(task))
	}
	if resp == nil {
		// Found nothing!
		c.JSON(http.StatusOK, []string{})
		return
	}
	c.JSON(http.StatusOK, resp)
}

func getAllTags (c *gin.Context) {
	var tags []Tag
	db.Find(&tags)
	c.JSON(http.StatusOK, tags)
}

func getTask (c *gin.Context) {
	id := c.Params.ByName("id")
	var task Task
	if err := db.Where("id = ?", id).Preload("Tags").First(&task).Error; err != nil {
		notFoundResponse(c)
	} else {
		c.JSON(http.StatusOK, getTaskResponse(task))
	}
}

func getTaskResponse(task Task) TaskResponse {
	var tags = ""
	var container []string
	for _, tag := range task.Tags {
		container = append(container, tag.Name)
	}
	sort.Strings(container)
	for i, tagName := range container {
		if i != 0 {
			tags += delimiter
		}
		tags += escapeTag(tagName)
	}
	return TaskResponse{TaskId:task.TaskId, CreationDate:task.CreationDate,
		Name:task.Name, Description:task.Description, Tags:tags}
}

func getTag (c *gin.Context) {
	id := c.Params.ByName("id")
	var tag Tag
	if err := db.Where("id = ?", id).First(&tag).Error; err != nil {
		notFoundResponse(c)
	} else {
		c.JSON(http.StatusOK, tag)
	}
}

func deleteTask (c *gin.Context) {
	id := c.Params.ByName("id")
	var task Task
	if err := db.Where("id = ?", id).First(&task).Error; err != nil {
		notFoundResponse(c)
	} else {

		var tt []TaskTags
		if err := db.Where("task_id = ?", task.TaskId).First(&tt).Error; err == nil {
			for _, t := range tt{
				db.Delete(t)
			}
		}
		db.Delete(&task)
		deletionResponse(c)
	}
}

func deleteTag (c *gin.Context) {
	id := c.Params.ByName("id")
	var tag Tag
	if err := db.Where("id = ?", id).Find(&tag).Error; err != nil {
		notFoundResponse(c)
	} else {

		var tt []TaskTags
		if err := db.Where("tag_id = ?", tag.TagId).Find(&tt).Error; err == nil {
			for _, t := range tt{
				db.Delete(t)
			}
		}
		db.Delete(&tag)
		deletionResponse(c)
	}
}

func updateTag (c *gin.Context) {

	// bin and validate the request
	var request TagRequest
	if err := c.ShouldBindJSON(&request); err != nil {
		invalidRequestResponse(c)
		return
	}
	if validateName(request.Name) || validateDescription(*request.Description) {
		invalidRequestResponse(c)
		return
	}

	tx := db.Begin()
	if tx.Error != nil {
		return
	}
	id := c.Params.ByName("id")
	var tag Tag
	if err := tx.Where("id = ?", id).First(&tag).Error; err != nil {
		notFoundResponse(c)
		return
	}
	tag.Name = request.Name
	tag.Description = *request.Description
	if err := tx.Save(&tag).Error; err != nil {
		invalidRequestResponse(c) // not unique
		return
	}
	if res := tx.Commit(); res.Error != nil {
		invalidRequestResponse(c)
		return
	}
	c.JSON(http.StatusOK, tag)
}


func addTask (c *gin.Context) {

	// bin and validate the request
	var request TaskRequest
	if err := c.ShouldBindJSON(&request); err != nil {
		invalidRequestResponse(c)
		return
	}
	if validateName(request.Name) || validateDescription(*request.Description) {
		invalidRequestResponse(c)
		return
	}

	var allTags []string = nil
	if len(*request.Tags) > 0 {
		// if there are tags -> parse and split em
		allTags = parseTags(*request.Tags)
		if validateParsedTags(allTags) {
			invalidRequestResponse(c)
			return
		}
	}

	tx := db.Begin()
	if tx.Error != nil {
		return
	}
	task := &Task{Name: request.Name, Description: *request.Description}

	if res := tx.Create(task); res.Error != nil {
		invalidRequestResponse(c)
		return
	}

	for _, tagName := range allTags {
		var tag Tag
		if res := tx.FirstOrCreate(&tag, Tag{Name: tagName, Description: ""}); res.Error != nil {
			tx.Rollback()
			invalidRequestResponse(c)
			return
		}
		if res := tx.Model(task).Association("tags").Append(tag); res.Error != nil {
			tx.Rollback()
			invalidRequestResponse(c)
			return
		}
	}

	var resp Task
	tx.Where("id = ?", task.TaskId).Preload("Tags").First(&resp)

	if res := tx.Commit(); res.Error != nil {
		invalidRequestResponse(c)
		return
	}

	c.JSON(http.StatusCreated, getTaskResponse(resp))
	return
}

func updateTask (c *gin.Context) {

	id := c.Params.ByName("id")

	// bin and validate the request
	var request TaskRequest
	if err := c.ShouldBindJSON(&request); err != nil {
		invalidRequestResponse(c)
		return
	}
	if validateName(request.Name) || validateDescription(*request.Description) {
		invalidRequestResponse(c)
		return
	}

	var allTags []string = nil
	if len(*request.Tags) > 0 {
		// if there are tags -> parse and split em
		allTags = parseTags(*request.Tags)
		if validateParsedTags(allTags) {
			invalidRequestResponse(c)
			return
		}
	}

	tx := db.Begin()
	if tx.Error != nil {
		return
	}

	var task Task
	if err := tx.Where("id = ?", id).Preload("Tags").First(&task).Error; err != nil {
		notFoundResponse(c)
		return
	}

	task.Name = request.Name
	task.Description = *request.Description
	if err := tx.Save(&task).Error; err != nil {
		invalidRequestResponse(c)
		return
	}

	// delete all task_tag connections
	var tt []TaskTags
	if err := tx.Where("task_id = ?", task.TaskId).Find(&tt).Error; err == nil {
		for _, t := range tt{
			tx.Delete(t)
		}
	}

	for _, tagName := range allTags {
		var tag Tag
		if res := tx.FirstOrCreate(&tag, Tag{Name: tagName, Description: ""}); res.Error != nil {
			tx.Rollback()
			invalidRequestResponse(c)
			return
		}
		if res := tx.Model(task).Association("tags").Append(tag); res.Error != nil {
			tx.Rollback()
			invalidRequestResponse(c)
			return
		}
	}

	var resp Task
	tx.Where("id = ?", task.TaskId).Preload("Tags").First(&resp)

	if res := tx.Commit(); res.Error != nil {
		invalidRequestResponse(c)
		return
	}

	c.JSON(http.StatusOK, getTaskResponse(resp))
	return
}


// Responses

func deletionResponse (c *gin.Context) {
	c.JSON(http.StatusNoContent, nil)
}

func methodNotAllowed (c *gin.Context) {
	var msg = []string { "Method Not allowed" }
	c.JSON(http.StatusMethodNotAllowed, gin.H{
		"messages":  msg,
	})
}

func invalidRequestResponse (c *gin.Context) {
	var msg = []string { "Unprocessable Entity" }
	c.JSON(http.StatusUnprocessableEntity, gin.H{
		"messages":  msg,
	})
}

func urlNotFoundResponse (c *gin.Context) {
	var msg = []string { "Not Found", "url not found" }
	c.JSON(http.StatusNotFound, gin.H{
		"messages":  msg,
	})
}

func notFoundResponse (c *gin.Context) {
	var msg = []string { "Not Found", "id not found" }
	c.JSON(http.StatusNotFound, gin.H{
		"messages":  msg,
	})
}


// Validation Stuff

func validateName(name string) bool {
	// return true if invalid
	return len(name) < 1 || len(name) > 30
}

func validateDescription(desc string) bool {
	// return true if invalid
	return len(desc) < 0 || len(desc) > 100
}

func validateParsedTags(tags []string) bool {
	// return true if invalid
	if tags == nil{
		return true
	}
	for _, tag := range tags {
		if validateName(tag) {
			return true
		}
	}
	return false
}

var escapeSign = "%"
var delimiter = ","


func unescapeTag(tag string) string {
	escaped := false
	unescapedTag := ""

	for _, c := range tag {
		if !escaped && string(c) == escapeSign {
			escaped = true
		} else {
			unescapedTag += string(c)
			escaped = false
		}
	}
	return unescapedTag
}



func escapeTag(tag string) string {
	escapedTag := ""
	for _, c := range tag {
		if string(c) == escapeSign || string(c) == delimiter {
			escapedTag += escapeSign
			escapedTag += string(c)
		}		else {
			escapedTag += string(c)
		}
	}
	return escapedTag
}


func parseTags(tags_as_string string) []string {
	var tags []string
	tag := ""
	escaped := false

	for _, c := range tags_as_string {
		if escaped {
			if string(c) == escapeSign || string(c) == delimiter {
				// if escaped -> only allow escaped chars!
				tag += escapeSign
				tag += string(c)

			} else {
				// error!
				break
			}
			escaped = false

		}else {
			if string(c) == escapeSign {
				escaped = true

			} else if string(c) == delimiter {
				// add tag and start a new one
				tags = append(tags, unescapeTag(tag))
				tag = ""

			} else {
				tag += string(c)
			}
		}
	}
	if escaped {
		// still escaped? we had an error!
		return nil
	}

	// also add the last tag!
	tags = append(tags, unescapeTag(tag))
	return tags
}




