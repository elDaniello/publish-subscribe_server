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

void handle_error(int exitCode){
if(exitCode<0){
    fprintf(stderr, "Error: %s\n", strerror( errno ));
    exit(1);
}
};

void write_to_client(int clientsocket, const char* message){
 handle_error(write(clientsocket, message, strlen(message)));
}


//check if user login credentials are valid
bool Login(struct USERS * users, char* login, char* password){
    for (int i = 0; i < users->registeredUsersCount; i++){
        if( strcmp(login, users->login[i])==0 && strcmp(password, users->password[i])==0){
            return true;
        }
    };
    return false;   
}

//add new user to server
bool registerUser(struct USERS * users, char * login, char * password){
    
    pthread_mutex_lock(&(users->mutex));
    for(int i = 0; i < users->registeredUsersCount; i++)
    {
        if(strcmp(users->login[i], login)==0){
            pthread_mutex_unlock(&(users->mutex));
            return false; //user already exists
        }
    }

    if(users->registeredUsersCount==MAX_USR_COUNT){
        pthread_mutex_unlock(&(users->mutex));
        printf("max user count reached");
        return false; //users limit reached 
    }
    strncpy(users->login[users->registeredUsersCount], login, LOGIN_LEN);
    strncpy(users->password[users->registeredUsersCount], password, PASSWD_LEN);
    users->registeredUsersCount++;
    pthread_mutex_unlock(&(users->mutex));
    return true;
}

//load user data to the memory
void loadUserData(struct USERS * users){
    
    pthread_mutex_init(&(users->mutex), NULL); //initialize mutex
    users->registeredUsersCount=0;

    registerUser(users, "admin\n", "qwerty\n"); //for testing
    //todo - add read from file
};


