#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(char *host, char *url, char *query_params,
                            char *cookies, char *token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // Step 2: add the host
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Step 3 (optional): add headers and/or cookies, according to the protocol format
    if (cookies != NULL) {
        memset(line, 0, LINELEN);
        strcat(line, "Cookie: ");
        strcat(line, cookies);
        //strcat(line, ";");
        compute_message(message, line);
    }
     if (token != NULL) {
        memset(line, 0, LINELEN);
        strcat(line, "Authorization: Bearer ");
        strcat(line, token);

        compute_message(message, line);
    }

    // Step 4: add final new line
    compute_message(message, "");
    return message;
}

char *compute_post_request(char *host, char *url, char* content_type, char *body_data,
                         char *cookies, char* token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    // Step 2: add the host
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Step 3: add necessary headers (Content-Type and Content-Length are mandatory)
    sprintf(line, "Content-Type: %s\r\nContent-Length: %ld", content_type, strlen(body_data));
    compute_message(message, line);

    // Step 4 (optional): add cookies
    if (cookies != NULL) {
        memset(line, 0, LINELEN);
        strcat(line, "Cookie: ");

        strcat(line, cookies);


        compute_message(message,line);
    }
    if (token != NULL) {
        memset(line, 0, LINELEN);
        strcat(line, "Authorization: Bearer ");
        strcat(line, token);
        compute_message(message,line);
    }

    // Step 5: add new line at end of header
    compute_message(message, "");

    // Step 6: add data
    memset(line, 0, LINELEN);
    strcat(message, body_data);

    free(line);
    return message;
}

char *compute_delete_request(char *host, char *url, char *query_params,
                            char *cookies, char *token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // Step 2: add the host
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Step 3 (optional): add headers and/or cookies, according to the protocol format
    if (cookies != NULL) {
        memset(line, 0, LINELEN);
        strcat(line, "Cookie: ");
        strcat(line, cookies);
        //strcat(line, ";");
        compute_message(message, line);
    }
     if (token != NULL) {
        memset(line, 0, LINELEN);
        strcat(line, "Authorization: Bearer ");
        strcat(line, token);

        compute_message(message, line);
    }

    // Step 4: add final new line
    compute_message(message, "");
    return message;
}
