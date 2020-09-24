#ifndef PROJECT_UTILS_H
#define PROJECT_UTILS_H

#include "return_t.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNUSED(expr) do{(void)(expr);}while(0)

return_t readFile(char* name, char **file_buffer);

#endif //PROJECT_UTILS_H
