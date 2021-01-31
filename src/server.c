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
#include "err.h"
void handle_error(int exitCode){
if(exitCode<0){
    fprintf(stderr, "Error: %s\n", strerror( errno ));
    exit(1);
}
};

int replace_delimiter(char *str) {
    char rep = '\t';
    char orig = '\n';
    char *ix = str;
    int n = 0;
    while((ix = strchr(ix, orig)) != NULL) {
        *ix++ = rep;
        n++;
    }
    return n;

}

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

bool createNewTag(struct TAGS * tags, char * tagname, char * admin){
    pthread_mutex_lock(&(tags->mutex));
    //first, check if tag's name is unique
    for (int i = 0; i < tags->tagsCount; i ++){
        if(strcmp(tags->tag[i].name, tagname)==0){ 
            pthread_mutex_unlock(&(tags->mutex));
            return false;
        }    
    }
    struct TAG * newTag = &tags->tag[tags->tagsCount];

    strncpy((*newTag).name, tagname, TAG_NAME_LEN);
    strncpy((*newTag).admin, admin, LOGIN_LEN);
    (*newTag).subsCount=0;
    (*newTag).message=malloc(sizeof(struct  MESSAGE*)*0);
    (*newTag).messagesCount=0;
    pthread_mutex_init(&((*newTag).messageMutex), NULL);
    pthread_mutex_init(&((*newTag).subsMutex), NULL);
    
    
    tags->tagsCount++;

    pthread_mutex_unlock(&(tags->mutex));
    
    printf("Tag %s created.\n", tagname);
    return true;

}


char ** getAllTagsNames(struct TAGS * tags){

char ** tagnames = malloc(sizeof(char)*TAG_NAME_LEN*tags->tagsCount);
for (int i = 0; i < tags->tagsCount; i++){
    strncpy(tagnames[i], tags->tag->name, TAG_NAME_LEN);
}
return tagnames;
}

char ** getMySubscriptionsTags(char * name, struct TAGS * tags){
    int tagsCount = getUserSubscriptionsCount(*tags, name);
    char ** MySubscriptionsTags = malloc(sizeof(char)*TAG_NAME_LEN*tagsCount);
    int subscriptionTagsCount = 0;
    for ( int i = 0; i<tags->tagsCount; i++){
        if(isSubscriber(tags->tag[i], name)){
            strncpy(MySubscriptionsTags[subscriptionTagsCount],tags->tag[i].name, TAG_NAME_LEN);
            subscriptionTagsCount++;
        }
    }
    return MySubscriptionsTags;
}



bool isSubscriber(struct TAG  tag, char * name){
    for (int i = 0; i < tag.subsCount; i++){
        if(strcmp(tag.subs[i], name)==0){
            return true;
        }
    }
    return false;
}


struct TAG * getTagStructByName(struct TAGS * tags, char * name){
    for( int i = 0; i<tags->tagsCount; i++){
        if(strcmp(tags->tag[i].name,name)==0){
            return &tags->tag[i];
        }
    }
    return NULL;
}

bool subscribe(struct TAGS * tags, char * tagName, char * user){
    
    struct TAG  * targetTag = getTagStructByName(tags, tagName);
    if(targetTag==NULL){
        printf("Count't subscribe, tag %s not found\n",tagName);
        return false;
    }
    if(isSubscriber(*targetTag, user)){
        return false; //can't subscibe tag that is already subscibed by user
    }

    pthread_mutex_lock(&targetTag->subsMutex);
        targetTag->subs[targetTag->subsCount] = malloc(sizeof(char) * LOGIN_LEN);
        strncpy(targetTag->subs[targetTag->subsCount], user, LOGIN_LEN);
        targetTag->subsCount++;
    pthread_mutex_unlock(&targetTag->subsMutex);    
    return true;
}

int getUserSubscriptionsCount(struct TAGS tags, char * name){
    
    int count = 0;
    for( int i = 0; i<tags.tagsCount; i++){
        if(isSubscriber(tags.tag[i], name)){
            count++;
        }
    }
    return count;
}

bool newMessage(struct TAG * tag,char * name, char * text){
    if(isAllowedToPost(*tag, name)==false){
        printf("User %s is not allowed to post in tag %s\n", name, tag->name);

        return false;
        
    }
    pthread_mutex_lock(&tag->messageMutex);
        tag->message = realloc(tag->message, (tag->messagesCount+1)*sizeof(struct MESSAGE));
        struct MESSAGE * newMessage = malloc(sizeof(struct MESSAGE));
        strncpy(newMessage->author, name, LOGIN_LEN);
        strncpy(newMessage->text, text, MAX_MSG_LEN);
        (tag->message[tag->messagesCount])=*newMessage;
        tag->messagesCount++;

    pthread_mutex_unlock(&tag->messageMutex);
    return true; //temp
}


bool isAllowedToPost(struct TAG tag, char * name){
    return strcmp(tag.admin, name)==0; //only admin (creator) of tag is allowed to publish, may change it later
}

void initTags(struct TAGS * tags){
    pthread_mutex_init(&(tags->mutex), NULL);
    tags->tagsCount = 0;
}

bool isRegistered(struct USERS users, char * user){
    for (int i = 0; i < users.registeredUsersCount; i++){
        if(strcmp(users.login[i], user)==0){
            return true;
        }
    
    }
    return false;
}

bool unsubscribe(struct TAGS * tags, char * tagName, char * user){
    
    struct TAG  * targetTag = getTagStructByName(tags, tagName);
    if(targetTag==NULL){
        printf("Count't unsubscribe, tag %s not found\n",tagName);
        return false;
    }
    if(!isSubscriber(*targetTag, user)){
        printf("Can't unsubscribe if user %s does not subscribe tag %s.\n", user, tagName);
        return false; //can't unsubscribe tag that is not subscribed
    }

    pthread_mutex_lock(&targetTag->subsMutex);
        for( int i=0; i<targetTag->subsCount; i++){
            if(strcmp(user, targetTag->subs[i])==0){ 
                
                targetTag->subsCount--;
                if(targetTag->subsCount>0){ //we want to prevent array *subs* for having 'holes' after deletion
                    strncpy(targetTag->subs[i], targetTag->subs[targetTag->subsCount], LOGIN_LEN);
                    free(targetTag->subs[targetTag->subsCount]); //delete last element from array after moving it to the place of deleted sub
                }else{ //if deleted sub is last one
                    free(targetTag->subs[0]);
                }
            }
        }      
    pthread_mutex_unlock(&targetTag->subsMutex);    
    return true;
}