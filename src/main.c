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




//funkcja opisującą zachowanie wątku - musi przyjmować argument typu (void *) i zwracać (void *)
void *ThreadBehavior(void *input)
{
    pthread_detach(pthread_self());
    char login[LOGIN_LEN];
    char password[PASSWD_LEN];
    struct THREAD_DATA thread_data = *((struct THREAD_DATA*)input);
    
    int connectionsocketdescriptor = thread_data.connectionsocketdescriptor;
    
    bool login_status = false;
   
    do{

        write_to_client(connectionsocketdescriptor, CONNECTION_ESTABLILISHED);

        printf("%d\n", connectionsocketdescriptor);
        
        char request[REQUEST_MAX_LEN]; 
        

        handle_error(read(connectionsocketdescriptor, request, REQUEST_MAX_LEN));

        if(strcmp(request, LOGIN_REQUEST)==0){

            write_to_client(connectionsocketdescriptor, GET_LOGIN);
            handle_error(read(connectionsocketdescriptor, login, LOGIN_LEN));

            write_to_client(connectionsocketdescriptor, GET_PASSWORD);
            handle_error(read(connectionsocketdescriptor, password, PASSWD_LEN));

            login_status=Login(thread_data.users, login, password);

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

            if(registerUser(thread_data.users, login, password)){
                write_to_client(connectionsocketdescriptor, REGISTATION_SUCCES);
            }else{
                write_to_client(connectionsocketdescriptor, REGISTATION_FAIL);
                memset(login, 0, LOGIN_LEN);
                memset(password, 0, PASSWD_LEN);
            }
        }
        else{
            write_to_client(connectionsocketdescriptor, UNKNOWN_REQUEST);
            memset(request, 0, REQUEST_MAX_LEN);
        }  
    }while(login_status==false);
    // TODO - send list of client's subscribed topics
     //second char is \n, but we don't care about it
    
    while(true){ //when logged in
    char request[REQUEST_MAX_LEN]={0};
    handle_error(read(connectionsocketdescriptor, request, REQUEST_MAX_LEN));
  
    if(strcmp(request, CREATE_TAG)==0){
        
        write_to_client(connectionsocketdescriptor, "choose name:\n");
        char tagname[TAG_NAME_LEN]={0};
        handle_error(read(connectionsocketdescriptor, tagname, TAG_NAME_LEN));
        if(createNewTag(thread_data.tags, tagname, login)){
            write_to_client(connectionsocketdescriptor, "tag created\n");
        }else{
             write_to_client(connectionsocketdescriptor, "failure while creating tag\n");
             memset(tagname, 0, TAG_NAME_LEN);
        };
    
    }else if(strcmp(request, GET_FEED)==0){
        int tagsCount = getUserSubscriptionsCount(*thread_data.tags, login);
        char tagsCountText[4];
        sprintf(tagsCountText, "%d", tagsCount);
        write_to_client(connectionsocketdescriptor, tagsCountText);
        write_to_client(connectionsocketdescriptor, "\n");

     for ( int i = 0; i<thread_data.tags->tagsCount; i++){
        if(isSubscriber(thread_data.tags->tag[i], login)){
            write_to_client(connectionsocketdescriptor, thread_data.tags->tag[i].name);
        }
    }

    }else if(strcmp(request, GET_TAGS_NAMES)==0){
        char tagsCount[4];
        sprintf(tagsCount, "%ld", thread_data.tags->tagsCount);
        //writes number of tags to read 
        write_to_client(connectionsocketdescriptor, tagsCount);
        write_to_client(connectionsocketdescriptor, "\n");

        for(int i = 0; i < thread_data.tags->tagsCount; i++){
            write_to_client(connectionsocketdescriptor, thread_data.tags->tag[i].name);
            //write_to_client(connectionsocketdescriptor, "\n");
        };
    }else if(strcmp(request, LOAD_TAG)==0){
        char tagname[TAG_NAME_LEN]={0};
        handle_error(read(connectionsocketdescriptor,tagname, TAG_NAME_LEN));
        //char msgCount[5];
        //sprintf(msgCount, "%ld", thread_data.tags->tag[])

    }else if(strcmp(request, LOGOUT)==0){
        write_to_client(connectionsocketdescriptor, LOGGED_OUT);
        break;

    }else if(strcmp(request, SUBSCRIBE_TAG)==0){
        write_to_client(connectionsocketdescriptor, TAG_NAME_REQUEST);
        char tagname [TAG_NAME_LEN] = {0};
        handle_error(read(connectionsocketdescriptor, tagname, TAG_NAME_LEN));
        bool status = subscribe(thread_data.tags, tagname, login);
        if(status){
            write_to_client(connectionsocketdescriptor, SUBSCRIBTION_SUCCES);
        }else{
            write_to_client(connectionsocketdescriptor,SUBSCRIBTION_FAIL);
        }
        
    }
    else{
        //printf("what do you mean?\n"); 
        write_to_client(connectionsocketdescriptor, UNKNOWN_REQUEST);
        
    }
    };
    write_to_client(connectionsocketdescriptor, CONNECTION_CLOSED);
    close(connectionsocketdescriptor);

    return NULL;
}

//funkcja obsługująca połączenie z nowym klientem
void handleUser(int connection_socket_descriptor, struct USERS users, struct TAGS tags) {
    int create_result = 0;

    pthread_t thread1;

    
    struct THREAD_DATA thread_data;
    thread_data.connectionsocketdescriptor=connection_socket_descriptor;
    thread_data.users=&users;
    thread_data.tags=&tags;
    create_result = pthread_create(&thread1, NULL, ThreadBehavior, (void *)&thread_data);
    handle_error(create_result);
    

    return;
    //TODO (przy zadaniu 1) odbieranie -> wyświetlanie albo klawiatura -> wysyłanie
}

int main(int argc, char* argv[])
{   
    //read data login from file

    struct TAGS tags;
    initTags(&tags);


    struct USERS users;
    loadUserData(&users);
    //test

    createNewTag(&tags, "tag testowy\n", "admin\n");
    //newMessage(getTagStructByName(&tags, "tag testowy"), "admin", "hejo");
    //newMessage(getTagStructByName(&tags, "tag testowy"), "admin", "hejo");


   int server_socket_descriptor;
   int connection_socket_descriptor;
   int bind_result;
   int listen_result;
   char reuse_addr_val = 1;
   struct sockaddr_in server_address;

   //inicjalizacja gniazda serwera
   
   memset(&server_address, 0, sizeof(struct sockaddr));
   server_address.sin_family = AF_INET;
   server_address.sin_addr.s_addr = htonl(INADDR_ANY);
   server_address.sin_port = htons(SERVER_PORT);

   server_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
   if (server_socket_descriptor < 0)
   {
       fprintf(stderr, "%s: Błąd przy próbie utworzenia gniazda..\n", argv[0]);
       exit(1);
   }
   setsockopt(server_socket_descriptor, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse_addr_val, sizeof(reuse_addr_val));

   bind_result = bind(server_socket_descriptor, (struct sockaddr*)&server_address, sizeof(struct sockaddr));
   if (bind_result < 0)
   {
       fprintf(stderr, "%s: Błąd przy próbie dowiązania adresu IP i numeru portu do gniazda.\n", argv[0]);
       exit(1);
   }

   listen_result = listen(server_socket_descriptor, QUEUE_SIZE);
   if (listen_result < 0) {
       fprintf(stderr, "%s: Błąd przy próbie ustawienia wielkości kolejki.\n", argv[0]);
       exit(1);
   }

   while(1)
   {
       connection_socket_descriptor = accept(server_socket_descriptor, NULL, NULL);
       if (connection_socket_descriptor < 0)
       {
           fprintf(stderr, "%s: Błąd przy próbie utworzenia gniazda dla połączenia.\n", argv[0]);
           exit(1);
       }

       handleUser(connection_socket_descriptor, users, tags);
   }

   close(server_socket_descriptor);
   return(0);
}