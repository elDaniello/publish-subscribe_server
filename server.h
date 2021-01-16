#define SERVER_PORT 12345
#define QUEUE_SIZE 5
#define LOGIN_LEN 32
#define PASSWD_LEN 32
#define MAX_USR_COUNT 128
#define MAX_TAGS_COUNT 64
#define MAX_TAG_NAME_LEN 32

#include <pthread.h>
#include <stdbool.h>



#ifndef SERVER_H
#define SERVER_H


struct LOGIN_DATA
{
char * login;
char * password;
};

struct THREAD_DATA
{
    struct USERS * users;
    int connectionsocketdescriptor;
};



struct USERS
{
    int registeredUsersCount;
    char password [MAX_USR_COUNT][PASSWD_LEN];
    char  login[MAX_USR_COUNT][LOGIN_LEN];
    pthread_mutex_t mutex;
};

struct TAG
{
    char admin[LOGIN_LEN];
    int subsCount;
    char **subs;
};

struct TAGS
{
    long tagsCount;
    struct TAG tag[MAX_TAGS_COUNT];
    
};


void handle_error(int exitCode);

void write_to_client(int clientsocket, const char* message);

//check if user login credentials are valid
bool Login(struct USERS * users, char* login, char* password);

//add new user to server
bool registerUser(struct USERS * users, char * login, char * password);

//load user data to the memory
void loadUserData(struct USERS * users);

#endif