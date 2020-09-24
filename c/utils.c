#include "utils.h"

return_t readFile(char *name, char **file_buffer) {

    FILE *fp = fopen(name, "rb");
    if (!fp) {
        return RET_FAIL;
    }

    // check how big the file is
    fseek(fp, 0L, SEEK_END);
    size_t file_size = (size_t) ftell(fp);
    rewind(fp);

    // allocate buffer with file_size
    *file_buffer = calloc(1, file_size + 1);
    if (!*file_buffer) {
        fclose(fp);
        return RET_OOM;
    }

    // read file into buffer
    if (1 != fread(*file_buffer, file_size, 1, fp)) {
        fclose(fp);
        return RET_FAIL;
    }

    fclose(fp);
    return RET_OK;
}