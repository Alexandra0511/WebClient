#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson-master/parson.h"
#include <ctype.h>


#define HOST "34.241.4.235"
#define PORT 8080

int is_number(char nr[10]){
    for(int i=0; i<strlen(nr); i++){
        if(isdigit(nr[i]) == 0){
            return 0;
        }
    }
    return 1;
}

int main(int argc, char *argv[])
{
    char *message;
    char *response;
    int sockfd;
    char* cookie = NULL;
    char* token = NULL;
    char* found = calloc(4, sizeof(char)); //variabila pentru memorarea codului de eroare/succes
    while(1) {
        char command[50];
        scanf("%s", command);
        if(strcmp(command, "register") == 0){
            printf("username=");
            char username[20];
            scanf("%s", username);
            char password[20];
            printf("password=");
            scanf("%s", password);
            JSON_Value *value = json_value_init_object();
            JSON_Object *object = json_value_get_object(value);
            json_object_set_string(object, "username", username);
            json_object_set_string(object, "password", password);
            message = compute_post_request(HOST, "/api/v1/tema/auth/register", "application/json", 
                    json_serialize_to_string_pretty(value), NULL, NULL);
            
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            
            strncpy(found, response + 9, 3);
            if(atoi(found) >= 400) {
                if(atoi(found) != 429) { // pentru orice eroare care nu e de too many requests
                    printf("Error: Username %s already used\n", username);
                }
                else {
                    printf("429 error\n");
                }
            }
            else{
                printf("200 - OK - User added successfully!\n");
            }
            close_connection(sockfd);
        }
        else if(strcmp(command, "login") == 0) {
            if(cookie) {
                printf("You are already logged!\n");
            }
            else{
                printf("username=");
                char username[20];
                scanf("%s", username);
                char password[20];
                printf("password=");
                scanf("%s", password);
                JSON_Value *value = json_value_init_object();
                JSON_Object *object = json_value_get_object(value);
                json_object_set_string(object, "username", username);
                json_object_set_string(object, "password", password);
                message = compute_post_request(HOST, "/api/v1/tema/auth/login", "application/json", 
                        json_serialize_to_string_pretty(value), NULL, NULL);
                
                sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                close_connection(sockfd);
                
                strncpy(found, response + 9, 3);
                if(atoi(found) >= 400) {
                    if(atoi(found) != 429) { // pentru orice eroare care nu e de too many requests
                        printf("Error: Password/Username wrong\n");
                    }
                    else {
                        printf("429 ERROR\n");
                    }
                }
                else{
                    // memorez cookie-ul
                    cookie = malloc(1024);
                    strcpy(cookie, strstr(response, "connect.sid="));
                    cookie = strtok(cookie, ";");
                    printf("200 - OK - Welcome!\n");
                }
            }
        }
        else if(strcmp(command, "logout") == 0){
            if(cookie == NULL) {
                printf("Error: You are not logged in\n");
            }
            else{
                message = compute_get_request(HOST, "/api/v1/tema/auth/logout", NULL, cookie, NULL);
                sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                close(sockfd);
                
                free(cookie);
                cookie = NULL;
                free(token);
                token = NULL;
                printf("200 - OK - User logged out\n");
            }
        }
        else if(strcmp(command, "enter_library") == 0) {
            if(cookie == NULL) {
                printf("Error: You are not logged in\n");
            }
            else{
                message = compute_get_request(HOST, "/api/v1/tema/library/access", NULL, cookie, NULL);
                sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                close(sockfd);
                strncpy(found, response + 9, 3);
                if(atoi(found) >= 400) {
                    if(atoi(found) != 429) { // pentru orice eroare care nu e de too many requests
                        printf("Error: Access not permited\n");
                    }
                    else {
                    printf("429 ERROR\n");
                    }
                }
                else {
                    // memorez token-ul
                    token = malloc(1024);
                    strcpy(token, strstr(response, "token"));
                    memmove(token, token+8, strlen(token));
                    token[strlen(token) - 2] = '\0';
                    printf("200 - OK - Access permited\n");
                }
            }
            
        }
        else if(strcmp(command, "get_books") == 0){
            if(token == NULL) {
                printf("Error: You don't have library access\n");
            }
            else{
                message = compute_get_request(HOST, "/api/v1/tema/library/books", NULL, cookie, token);
                sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                close(sockfd);
                strncpy(found, response + 9, 3);
                if(atoi(found) >= 400) {
                    if(atoi(found) != 429) { 
                        printf("Error: Connot get the books\n");
                    }
                    else {
                    printf("429 ERROR\n");
                    }
                }
                else {
                    char *payload = strstr(response, "\r\n\r\n");
                    char aux[1500];
                    strcpy(aux, payload + 4);
                    printf("%s\n", aux); 
                }
            }
        }
        else if(strcmp(command, "get_book") == 0){
            if(token == NULL) {
                printf("Error: You don't have library access\n");
            }
            else{
                char id[10];
                printf("id=");
                scanf("%s", id);
                if(is_number(id)==0){
                    printf("Error: id is not a number");
                }
                else {
                    char* access_route = malloc(50);
                    strcpy(access_route, "/api/v1/tema/library/books/");
                    access_route = strcat(access_route, id);
                    message = compute_get_request(HOST, access_route, NULL, cookie, token);
                    free(access_route);
                    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
                    send_to_server(sockfd, message);
                    response = receive_from_server(sockfd);
                    close(sockfd);
                    
                    strncpy(found, response + 9, 3);
                    if(atoi(found) >= 400) {
                        if(atoi(found) != 429) { // pentru orice eroare care nu e de too many requests
                            printf("Error: Cannot get book\n");
                        }
                        else {
                        printf("429 ERROR\n");
                        }
                    }
                    else{
                        char* payload = strstr(response, "\r\n\r\n");
                        char aux[1500];
                        strcpy(aux, payload + 4);
                        printf("%s\n", aux); 
                    }
                }
            }
        }
        else if(strcmp(command, "add_book") == 0){
            if(token == NULL) {
                printf("Error: You don't have library access\n");
            }
            else{
                printf("title=");
                char title[20];
                scanf("%s", title);
                printf("author=");
                char author[20];
                scanf("%s", author);
                printf("genre=");
                char genre[20];
                scanf("%s", genre);
                printf("publisher=");
                char publisher[20];
                scanf("%s", publisher);
                printf("page_count=");
                char page_count[10];
                scanf("%s", page_count);
                if(is_number(page_count) == 0) {
                    printf("Error: page_count is not a number\n");
                }
                else{
                    JSON_Value *value = json_value_init_object();
                    JSON_Object *object = json_value_get_object(value);
                    json_object_set_string(object, "title", title);
                    json_object_set_string(object, "author", author);
                    json_object_set_string(object, "genre", genre);
                    json_object_set_string(object, "publisher", publisher);
                    json_object_set_string(object, "page_count", page_count);
                    message = compute_post_request(HOST, "/api/v1/tema/library/books", "application/json", 
                        json_serialize_to_string_pretty(value), NULL, token);
            
                    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
                    send_to_server(sockfd, message);
                    response = receive_from_server(sockfd);

                    strncpy(found, response + 9, 3);
                    if(atoi(found) >= 400) {
                        if(atoi(found) != 429) { // pentru orice eroare care nu e de too many requests
                            printf("Error: Cannot add the book\n");
                        }
                        else {
                        printf("429 ERROR\n");
                    }
                    }
                    else{
                        printf("200 - OK - Book added successfully\n");
                    }
                }
            }
        }
        else if(strcmp(command, "delete_book") == 0) {
             if(token == NULL) {
                printf("Error: You don't have library access\n");
            }
            else{
                printf("id=");
                char id[10];
                scanf("%s", id);
                if(is_number(id) == 0) {
                    printf("Error: id is not a number\n");
                }
                else {
                    char* route = malloc(50);
                    strcpy(route, "/api/v1/tema/library/books/");
                    route = strcat(route, id);
                    message = compute_delete_request(HOST, route, NULL, NULL, token);
                    free(route);
                    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
                    send_to_server(sockfd, message);
                    response = receive_from_server(sockfd);
                    close(sockfd);
                    
                    strncpy(found, response + 9, 3);
                    if(atoi(found) >= 400) {
                        if(atoi(found) != 429) { // pentru orice eroare care nu e de too many requests
                            printf("Error: No book with that id\n");
                        }
                        else {
                        printf("429 ERROR\n");
                    }
                    }
                    else{
                        printf("200 - OK - Book deleted successfully\n");
                    }
                }
            }
        }
        else if(strcmp(command, "exit") == 0){
            if(cookie) {
                free(cookie);
                cookie = NULL;
            }
            if(token) {
                free(token);
                token = NULL;
            }
            break;
        }
        else{
            printf("Please enter a valid command!\n");
        }
    }
    free(found);
    free(message);
    free(response);

    return 0;
}
