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

#define SERVER_PORT 12345
#define QUEUE_SIZE 5
#define LOGIN_LEN 32
#define PASSWD_LEN 32
#define MAX_USR_COUNT 128

typedef int bool;
#define true 1
#define false 0


void handle_error(int exitCode){
if(exitCode<0){
    fprintf(stderr, "Error: %s\n", strerror( errno ));
    exit(1);
}
};


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

pthread_mutex_t USERS_MUTEX = PTHREAD_MUTEX_INITIALIZER;

struct USERS
{
    long registeredUsersCount;
    char password [MAX_USR_COUNT][PASSWD_LEN];
    char  login[MAX_USR_COUNT][LOGIN_LEN];
};

void write_to_client(int clientdesc, const char* message){

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
    pthread_mutex_lock(&USERS_MUTEX);
    for(int i = 0; i < users->registeredUsersCount; i++)
    {
        if(strcmp(users->login[i], login)==0){
            pthread_mutex_unlock(&USERS_MUTEX);
            return false; //user already exists
        }
    }

    if(users->registeredUsersCount==MAX_USR_COUNT){
        pthread_mutex_unlock(&USERS_MUTEX);
        printf("max user count reached");
        return false; //users limit reached 
    }
    strncpy(users->login[users->registeredUsersCount], login, LOGIN_LEN);
    strncpy(users->password[users->registeredUsersCount], password, PASSWD_LEN);
    users->registeredUsersCount++;
    pthread_mutex_unlock(&USERS_MUTEX);
    return true;
}

//load user data to the memory
void loadUserData(struct USERS * users){
    users->registeredUsersCount=0;
    registerUser(users, "admin\n", "qwerty\n"); //for testing
    //todo - add read from file
};

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

    const char * msg = "welcome\n L-login, R-register\n";
    handle_error(write(connectionsocketdescriptor, msg, strlen(msg)));

    printf("%d\n", connectionsocketdescriptor);
    
    char request[2]; //second char is \n, but we don't care about it
    

    handle_error(read(connectionsocketdescriptor, request, 2));

    switch (request[0])
    {
    case 'L':
    
        handle_error(write(connectionsocketdescriptor, "I need your login\n", strlen("I need your login\n")));
        handle_error(read(connectionsocketdescriptor, login, LOGIN_LEN));

        handle_error(write(connectionsocketdescriptor, "Now I need your password\n", strlen("Now I need your password\n")));
        handle_error(read(connectionsocketdescriptor, password, PASSWD_LEN));

        login_status=Login(thread_data.users, login, password);

        if(login_status){
            handle_error(write(connectionsocketdescriptor, "Logged in\n", strlen("Logged in\n")));
        }else{
            handle_error(write(connectionsocketdescriptor, "Try again\n", strlen("Try again\n")));
        }
        break;
    case 'R':
         handle_error(write(connectionsocketdescriptor, "type your login\n", strlen("type your login\n")));
         handle_error(read(connectionsocketdescriptor, login, LOGIN_LEN));
         handle_error(write(connectionsocketdescriptor, "choose your password\n", strlen("choose your password\n")));
         handle_error(read(connectionsocketdescriptor, password, PASSWD_LEN));
         if(registerUser(thread_data.users, login, password)){
            handle_error(write(connectionsocketdescriptor, "registered succesfully\n", strlen("registered succesfully\n")));
         }else{
            handle_error(write(connectionsocketdescriptor, "failed to register\n", strlen("failed to register\n")));
         }
        break;
    default:
         printf("what?\n");
        break;
    }

    }
    while(login_status==false);

    
    //while(!(strcmp(login, "admin\n")==0 && strcmp(password, "qwerty\n")==0)); //simple login for testing

    //handle_error(write(connectionsocketdescriptor, "logged in\n", 4));
    //you are logged now

    return NULL;
}

//funkcja obsługująca połączenie z nowym klientem
void gatherLoginData(int connection_socket_descriptor, struct USERS users) {
    //wynik funkcji tworzącej wątek
    int create_result = 0;

    //uchwyt na wątek
    pthread_t thread1;

    //dane, które zostaną przekazane do wątku
    //TODO dynamiczne utworzenie instancji struktury thread_data_t o nazwie t_data (+ w odpowiednim miejscu zwolnienie pamięci)
    //TODO wypełnienie pól struktury


    //struct LOGIN_DATA * t_data  = malloc( sizeof(struct LOGIN_DATA));


   

    
    struct THREAD_DATA thread_data;
    thread_data.connectionsocketdescriptor=connection_socket_descriptor;
    thread_data.users=&users;
    create_result = pthread_create(&thread1, NULL, ThreadBehavior, (void *)&thread_data);
    handle_error(create_result);
    

    

    return;
    //TODO (przy zadaniu 1) odbieranie -> wyświetlanie albo klawiatura -> wysyłanie
}

int main(int argc, char* argv[])
{   
    //read data login from file

    struct USERS users;
    loadUserData(&users);
    

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

       gatherLoginData(connection_socket_descriptor, users);
   }

   close(server_socket_descriptor);
   return(0);
}