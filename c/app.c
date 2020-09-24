#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/errno.h>
#include <sqlite3.h>
#include "dbconfig.h"
#include "cJSON.h"
#include "routes.h"
#include "request.h"
#include "return_t.h"


// Global values
int server_fd;
sqlite3 *db;
volatile int running = 0; // Volatile to prevent compiler optimization


// based on tutorial code tutorial SASD 2018
return_t server_accept(int s_fd) {
    return_t ret;
    int c_fd;
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    socklen_t client_addrlen = sizeof(client_addr);

    if (-1 == (c_fd = accept(s_fd, (struct sockaddr *) &client_addr, &client_addrlen))) {
        perror("accept");
        return RET_ABORT;
    }

    printf("Connection opened...\n");

    /* Initialize with NULL s.t. request_free does no double free */
    request_t *request = NULL;
    if (RET_OK != (ret = request_parse(c_fd, &request))) {
        printf("Request error\n");
        if (RET_TIME_OUT == ret) {
            const char *const data[] = {INVALID_REQUEST, TIMEOUT};
            sendErrorResponse(c_fd, 504, data, 2); // shouldn't get tested since blocking is okay
        }
        else {
            // this api has no 400 bad request so just use 422
            const char *const data[] = {INVALID_REQUEST};
            sendErrorResponse(c_fd, 422, data, 1);
        }
        goto cleanup;
    }

    handleRouting(c_fd, db, request);

    cleanup:
    shutdown(c_fd, SHUT_RDWR);
    close(c_fd);
    printf("Connection closed\n");
    request_free(&request);
    return ret;
}


// copied from tutorial SASD 2018
return_t server_create(int *s_fd, size_t port) {
    return_t ret;
    if (!s_fd) {
        return RET_INV;
    }
    if ((*s_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("socket");
        return RET_FAIL;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons((uint16_t) port);

    if (bind(*s_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        ret = RET_FAIL;
        goto cleanup;
    }

    if (listen(*s_fd, 1024) < 0) {
        perror("listen");
        ret = RET_FAIL;
        goto cleanup;
    }
    return RET_OK;

    cleanup:
    close(*s_fd);
    return ret;
}


static void sigint(int signo) {
    printf("\nSignal received\n");
    printf("Shutting down...\n");
    running = 0;
    close(server_fd);
    sqlite3_close(db);
}


int main(int argc, char *argv[]) {
    if (3 != argc) {
        fprintf(stderr, "Provide db as first argument and port as second argument\n");
        return -1;
    }

    // Ids are stored in size_t in this app
    // size_t max size is not defined (at least 65535) but should be bigger than int32 most of the time. To
    // this is the case on the tests system with size_t_max == 18446744073709551615 vs int32_max == 2147483647
    // To prevent overflows and not the time to rewrite everything the app is blocked for architectures
    // where this is not the case
    size_t max_check = (size_t) -1;
    if (max_check < INT32_MAX) {
        printf("Architecture not supported\n");
        return -1;
    }

    // Setup signal handler for graceful shutdown
    if (SIG_ERR == signal(SIGINT, sigint)) {
        printf("Signal error\n");
        return -1;
    }

    // create or open and init database
    for (size_t i = 0, b = 0; i < strlen(argv[1]); i++) {
        char c = argv[1][i];
        if ('.' == c) {
            b++;
        }
        else {
            b = 0;
        }
        if (2 <= b) {
            fprintf(stderr, "Invalid db path, can't leave root");
            sqlite3_close(db);
            return -1;
        }
    }
    int rc = sqlite3_open_v2(argv[1], &db, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
         sqlite3_close(db);
        return -1;
    }
    rc = initDatabase(db);
    if (rc) {
        fprintf(stderr, "db init error\n");
        sqlite3_close(db);
        return -1;
    }
    fprintf(stderr, "database '%s' successfully initialized\n", argv[1]);

    // Create server
    errno = 0;
    size_t port = (size_t) strtol(argv[2], 0, 10);
    char *endptr = NULL;
    long number = strtol(argv[2], &endptr, 10);
    if (errno == 0 && !*endptr && number > 0 && number < INT32_MAX) {
        // valid  (and represents all characters read
        port = (size_t) number;
    }
    else {
        // set port to invalid
        port = 0;
    }

    if (RET_OK != server_create(&server_fd, port)) {
        fprintf(stderr, "Could not create server\n");
        sqlite3_close(db);
        return -1;
    }
    fprintf(stdout, "Starting server on port %zu...\n", port);

    running = 1;
    while (running) {
        server_accept(server_fd);
    }
}
