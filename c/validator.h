#ifndef WEB_C_VALIDATOR_H
#define WEB_C_VALIDATOR_H

#include <memory.h>
#include "cJSON.h"
#include <stdlib.h>
#include "stdio.h"

int parseTagBody(cJSON *params, char **name, char **description);

int parseTaskBody(cJSON *params, char **name, char **description, char **tags);

char** parseTags(char *tags_as_string, size_t *amount_of_tags);

#endif //WEB_C_VALIDATOR_H
