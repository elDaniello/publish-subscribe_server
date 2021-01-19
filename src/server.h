#define SERVER_PORT 12345
#define QUEUE_SIZE 5
#define LOGIN_LEN 32
#define PASSWD_LEN 32
#define MAX_USR_COUNT 128
#define MAX_TAGS_COUNT 64
#define TAG_NAME_LEN 16
#define MAX_MSG_LEN 280 //is that a twitter reference?


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
    struct TAGS * tags;
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
    char name[TAG_NAME_LEN];
    char admin[LOGIN_LEN];
    int subsCount;
    char *subs[LOGIN_LEN];
    pthread_mutex_t subsMutex;
    long messagesCount;
    struct MESSAGE * message;
    pthread_mutex_t messageMutex;
    
};

struct TAGS
{
    long tagsCount;
    struct TAG tag[MAX_TAGS_COUNT];
    pthread_mutex_t mutex;
};

struct MESSAGE
{
char author[LOGIN_LEN];
char text[MAX_MSG_LEN];
};

//display human-readable error description to stderr if function fails
void handle_error(int exitCode);

//writes plain text to client at @clientsocket
void write_to_client(int clientsocket, const char* message);

//check if user login credentials are valid
bool Login(struct USERS * users, char* login, char* password);

//add new user to server
bool registerUser(struct USERS * users, char * login, char * password);

//load user data to the memory
void loadUserData(struct USERS * users);

//add user to tag subscribe list
bool subscribe(struct TAGS * tags, char * tagName, char * user);


//return TAG struct with given name
struct TAG * getTagStructByName(struct TAGS * tags, char * name);

//creates new tag if it's name is uniqe and server didn't reached MAX_TAGS_COUNT
bool createNewTag(struct TAGS * tags, char * tagname, char * admin);

//creates new message on given tag
bool newMessage(struct TAG * tag, char * name, char * text);

//checks if user has permission to post on this tag
bool isAllowedToPost(struct TAG tag, char * name);

//return all tags names if user wants to subscribe one
char ** getAllTagsNames(struct TAGS * tags);

//gets all tags names subscribed by given
char ** getMySubscriptionsTags(char * user, struct TAGS * tags);


struct MESSAGE loadTag(struct TAG * tag);

//check if user is subscribing tag
bool isSubscriber(struct TAG  tag, char * name);


//check how many tags users is subscribing
int getUserSubscriptionsCount(struct TAGS tags, char * name);

#endif