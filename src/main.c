#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <time.h>
#include <pthread.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>

#include "server.h"
#include "API.h"
#include "config.h"



//funkcja opisującą zachowanie wątku - musi przyjmować argument typu (void *) i zwracać (void *)
void *ThreadBehavior(void *input)
{
    pthread_detach(pthread_self());
    char login[LOGIN_LEN]={0};
    char password[PASSWD_LEN]={0};
    struct THREAD_DATA * thread_data = ((struct THREAD_DATA*)input);
    
    int connectionsocketdescriptor = thread_data->connectionsocketdescriptor;
    
    bool login_status = false;
   
    do{

        write_to_client(connectionsocketdescriptor, CONNECTION_ESTABLILISHED);
        
        char request[REQUEST_MAX_LEN]={0};
        
        handle_error(read(connectionsocketdescriptor, request, REQUEST_MAX_LEN));
        printf("Got %s request from user %s\n", request, login);
        if(strcmp(request, LOGIN_REQUEST)==0){

            write_to_client(connectionsocketdescriptor, GET_LOGIN);
            handle_error(read(connectionsocketdescriptor, login, LOGIN_LEN));

            write_to_client(connectionsocketdescriptor, GET_PASSWORD);
            handle_error(read(connectionsocketdescriptor, password, PASSWD_LEN));

            login_status=Login(thread_data->users, login, password);

            if(login_status){
                write_to_client(connectionsocketdescriptor, LOGIN_SUCCESFULL);
            }else{
                write_to_client(connectionsocketdescriptor, BAD_LOGIN_CREDENTIALS);
                memset(login, 0, LOGIN_LEN);
                memset(password, 0, PASSWD_LEN);

            }
        }else
        if(strcmp(request, REGISTER_REQUEST)==0){

            write_to_client(connectionsocketdescriptor, GET_LOGIN);
            handle_error(read(connectionsocketdescriptor, login, LOGIN_LEN));
            write_to_client(connectionsocketdescriptor, GET_PASSWORD);
            handle_error(read(connectionsocketdescriptor, password, PASSWD_LEN));

            if(registerUser(thread_data->users, login, password)){
                write_to_client(connectionsocketdescriptor, REGISTATION_SUCCES);
                memset(request, 0, REQUEST_MAX_LEN);
            }else{
                write_to_client(connectionsocketdescriptor, REGISTATION_FAIL);
            }
            memset(login, 0, LOGIN_LEN);
            memset(password, 0, PASSWD_LEN);
            memset(request, 0, REQUEST_MAX_LEN);
        }
        else{
            write_to_client(connectionsocketdescriptor, UNKNOWN_REQUEST);
            memset(request, 0, REQUEST_MAX_LEN);
        }  
    }while(login_status==false);
    

    
    while(true){ //when logged in
    char request[REQUEST_MAX_LEN]={0};
    handle_error(read(connectionsocketdescriptor, request, REQUEST_MAX_LEN));
  
    if(strcmp(request, CREATE_TAG)==0){
        
        write_to_client(connectionsocketdescriptor, "choose name:\n");
        char tagname[TAG_NAME_LEN]={0};
        handle_error(read(connectionsocketdescriptor, tagname, TAG_NAME_LEN));
        if(createNewTag(thread_data->tags, tagname, login)){
            write_to_client(connectionsocketdescriptor, "tag created\n");
        }else{
             write_to_client(connectionsocketdescriptor, "failure while creating tag\n");
             memset(tagname, 0, TAG_NAME_LEN);
        };
    
    }else if(strcmp(request, GET_FEED)==0){
        int tagsCount = getUserSubscriptionsCount(*thread_data->tags, login);
        char tagsCountText[4];
        sprintf(tagsCountText, "%d", tagsCount);
        write_to_client(connectionsocketdescriptor, tagsCountText);
        write_to_client(connectionsocketdescriptor, "\t");

     for ( int i = 0; i<thread_data->tags->tagsCount; i++){
        if(isSubscriber(thread_data->tags->tag[i], login)){
            char tagName[TAG_NAME_LEN] = {0};
            strncpy(tagName, thread_data->tags->tag[i].name, TAG_NAME_LEN);
            replace_delimiter(tagName);
            write_to_client(connectionsocketdescriptor, tagName);
        }
       
    }
     write_to_client(connectionsocketdescriptor, "\n");

    }else if(strcmp(request, GET_TAGS_NAMES)==0){
        char tagsCount[4];
        sprintf(tagsCount, "%ld", thread_data->tags->tagsCount);
        //writes number of tags to read 
        write_to_client(connectionsocketdescriptor, tagsCount);
        write_to_client(connectionsocketdescriptor, "\t");

        for(int i = 0; i < thread_data->tags->tagsCount; i++){
             char name[TAG_NAME_LEN] = {0};
            strncpy(name, thread_data->tags->tag[i].name, TAG_NAME_LEN);
            replace_delimiter(name);
            write_to_client(connectionsocketdescriptor, name);
        
        };
        write_to_client(connectionsocketdescriptor, "\n");
    }else if(strcmp(request, LOAD_TAG)==0){
        char tagname[TAG_NAME_LEN]={0};
        write_to_client(connectionsocketdescriptor, TAG_NAME_REQUEST);
        handle_error(read(connectionsocketdescriptor,tagname, TAG_NAME_LEN));
        char msgCount[5]={0};
        struct TAG * targetTag = malloc(sizeof(struct TAG));
        targetTag = getTagStructByName(thread_data->tags, tagname);
        if(targetTag==NULL){
            write_to_client(connectionsocketdescriptor, TAG_NOT_FOUND);
        }else{
             sprintf(msgCount, "%ld", targetTag->messagesCount);
             write_to_client(connectionsocketdescriptor, msgCount);
             write_to_client(connectionsocketdescriptor, "\t");
             for(int i=0; i<targetTag->messagesCount; i++){
                char text[MAX_MSG_LEN] = {0};
                strncpy(text, targetTag->message[i].text, MAX_MSG_LEN);
                replace_delimiter(text);
                write_to_client(connectionsocketdescriptor, text);
             }
             write_to_client(connectionsocketdescriptor, "\n");  

        }
       
    }else if(strcmp(request, LOGOUT)==0){
        write_to_client(connectionsocketdescriptor, LOGGED_OUT);
        break;

    }else if(strcmp(request, SUBSCRIBE_TAG)==0){
        write_to_client(connectionsocketdescriptor, TAG_NAME_REQUEST);
        char tagname [TAG_NAME_LEN] = {0};
        handle_error(read(connectionsocketdescriptor, tagname, TAG_NAME_LEN));
        bool status = subscribe(thread_data->tags, tagname, login);
        if(status){
            write_to_client(connectionsocketdescriptor, SUBSCRIBTION_SUCCES);
        }else{
            write_to_client(connectionsocketdescriptor,SUBSCRIBTION_FAIL);
        }
        
    }else if(strcmp(request, UNSUBSCRIBE)==0){
        write_to_client(connectionsocketdescriptor, TAG_NAME_REQUEST);
        char tagname [TAG_NAME_LEN] = {0};
        handle_error(read(connectionsocketdescriptor, tagname, TAG_NAME_LEN));
        bool status = unsubscribe(thread_data->tags, tagname, login);
        if(status){
            write_to_client(connectionsocketdescriptor, UNSUBSCRIBTION_SUCCES);
        }else{
            write_to_client(connectionsocketdescriptor,UNSUBSCRIBTION_FAIL);
        }

    }else if(strcmp(request, POST)==0){
        write_to_client(connectionsocketdescriptor, TAG_NAME_REQUEST);
        char tagname [TAG_NAME_LEN] = {0};
        handle_error(read(connectionsocketdescriptor, tagname, TAG_NAME_LEN));
        struct TAG * targetTag = getTagStructByName(thread_data->tags, tagname);

        if(targetTag==NULL){
            printf("Coudn't find tag %s, creating message failure.\n", tagname);
            write_to_client(connectionsocketdescriptor, TAG_NOT_FOUND);
        }else{
            write_to_client(connectionsocketdescriptor, MSG_BODY_REQUEST);
            char msg[MAX_MSG_LEN] = {0};
            handle_error(read(connectionsocketdescriptor, msg, MAX_MSG_LEN));
            if(!newMessage(targetTag, login, msg)){
                write_to_client(connectionsocketdescriptor, ACCESS_DENIED); //when creating message fails - probably you don't have perrmision
            }else{
                write_to_client(connectionsocketdescriptor, MSG_CONFIRMATION);
        
            }
        }

    }else{ //default 
        printf("Request %s not recognised\n", request); 
        write_to_client(connectionsocketdescriptor, UNKNOWN_REQUEST);
        
    }
    };
    write_to_client(connectionsocketdescriptor, CONNECTION_CLOSED);
    close(connectionsocketdescriptor);

    return NULL;
}

//funkcja obsługująca połączenie z nowym klientem
void handleUser(int connection_socket_descriptor, struct USERS * users, struct TAGS * tags) {
    int create_result = 0;

    pthread_t thread1;

    
    struct THREAD_DATA * thread_data = malloc(sizeof(struct THREAD_DATA));
    thread_data->connectionsocketdescriptor=connection_socket_descriptor;
    thread_data->users=users;
    thread_data->tags=tags;
    create_result = pthread_create(&thread1, NULL, ThreadBehavior, (void *)thread_data);
    handle_error(create_result);
    
    return;
    
}

int main(int argc, char* argv[])
{   
    //initialize server state

    struct TAGS * tags = malloc(sizeof (struct TAGS));
    initTags(tags);

    struct USERS * users =malloc(sizeof (struct USERS));
    loadUserData(users);

   int server_socket_descriptor;
   int connection_socket_descriptor;
   int bind_result;
   int listen_result;
   char reuse_addr_val = 1;
   struct sockaddr_in server_address;

   
   memset(&server_address, 0, sizeof(struct sockaddr));
   server_address.sin_family = AF_INET;
   server_address.sin_addr.s_addr = inet_addr(SERVER_ADDR);
   server_address.sin_port = htons(SERVER_PORT);

   server_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
   handle_error(server_socket_descriptor);
   setsockopt(server_socket_descriptor, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse_addr_val, sizeof(reuse_addr_val));

   bind_result = bind(server_socket_descriptor, (struct sockaddr*)&server_address, sizeof(struct sockaddr));
   handle_error(bind_result);

   listen_result = listen(server_socket_descriptor, QUEUE_SIZE);
   handle_error(listen_result);

    printf("Starting server at %s:%d\n", SERVER_ADDR, SERVER_PORT);
    
   while(1)
   {
       connection_socket_descriptor = accept(server_socket_descriptor, NULL, NULL);
       handle_error(connection_socket_descriptor);
       handleUser(connection_socket_descriptor, users, tags);
   }

   close(server_socket_descriptor);
   free(tags);
   free(users);
   return(0);
}