#include "validator.h"


int validateName(char *name) {
    // return zero if okay
    return !name || strlen(name) < (1) || strlen(name) > (30);
}

int validateDescription(char *description) {
    // return zero if okay
    return !description || strlen(description) > (100);
}

int validateTags(char *tags) {
    // return zero if okay
    return !tags;
}

void getStringFromParams(cJSON *params, char **name, const char *column) {

    cJSON *c_c = cJSON_GetObjectItemCaseSensitive(params, column);
    if (NULL == c_c) {
        return;
    }
    char *tmp = cJSON_Print(c_c);
    if (NULL == tmp || strlen(tmp) < 2) {
        return;
    }

    // get rid of quotations marks
    char *s = tmp;
    s++;
    s[strlen(s) - 1] = '\0';

    *name = strdup(s);
    free(tmp);
}

int parseTagBody(cJSON *params, char **name, char **description) {

    getStringFromParams(params, name, "name");
    getStringFromParams(params, description, "description");

    // return zero if okay
    return (validateName(*name) || validateDescription(*description));
}

int parseTaskBody(cJSON *params, char **name, char **description, char **tags) {

    getStringFromParams(params, name, "name");
    getStringFromParams(params, description, "description");
    getStringFromParams(params, tags, "tags");

    // return zero if okay
    return (validateName(*name) || validateDescription(*description) || validateTags(*tags));
}


char **parseTags(char *tags_as_string, size_t *amount_of_tags) {

    if (!tags_as_string) {
        return NULL;
    }

    char **tags = NULL;

    // lets first find out how many tags we have!
    int escaped = 0;
    *amount_of_tags = 1;
    char delimiter = ',';
    char escape_sign = '%';

    for (size_t i = 0; i < strlen(tags_as_string); i++) {

        char c = tags_as_string[i];
        if (escaped) {
            if (escape_sign != c && delimiter != c) {
                goto error;
            }
            escaped = 0;
        }
        else {
            if (escape_sign == c) {
                escaped = 1;
            }
            else if (delimiter == c) {
                (*amount_of_tags)++;
            }
        }
    }
    if (escaped) {
        goto error; // still escaped? we had an error!
    }

    // allocate enough space for all tags
    tags = calloc(*amount_of_tags, sizeof(char *));
    if (!tags) {
        goto error; // OOM
    }


    // search start address of tags and replace delimter with null bytes to end the tags
    size_t counter = 0;
    char *p = tags_as_string;
    tags[counter] = tags_as_string;
    counter++;
    while (*p) {
        if (escaped) {
            if (escape_sign != *p && delimiter != *p) {
                goto error;
            }
            escaped = 0;
        }
        else {
            if (escape_sign == *p) {
                escaped = 1;
            }
            else if (delimiter == *p) {
                tags[counter] = p + 1; // set start address of next pointer
                counter++;
                *p = '\0'; // overwrite delimiter with null byte
            }
        }
        p++;
    }


    if (counter != *amount_of_tags) {
        goto error; // something went wrong
    }

    // unescape tags and final checks
    char *tmp = NULL;
    for (size_t i = 0; i < *amount_of_tags; i++) {

        tmp = tags[i];
        if (!tmp || strlen(tmp) == 0) {
            goto error; // empty tag names? na thx
        }

        // remove escape signs
        for (size_t j = 0; j < strlen(tmp); j++) {
            if (!escaped && escape_sign == tmp[j]) {
                escaped = 1;
                memmove(&tmp[j], &tmp[j + 1], strlen(tmp) - j);
                j--;
            } else {
                escaped = 0;
            }
        }
        if (validateName(tmp)) {
            goto error;
        }
    }

    return tags;

    error:
    free(tags);
    return NULL;
}


