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

//struktura zawierająca dane, które zostaną przekazane do wątku
struct LOGIN_DATA
{
char * login;
char * password;
};

struct APP_DATA
{
    
};

pthread_mutex_t USERS_MUTEX = PTHREAD_MUTEX_INITIALIZER;

struct USERS
{
    long registeredUsersCount;
    char password [MAX_USR_COUNT][PASSWD_LEN];
    char  login[MAX_USR_COUNT][LOGIN_LEN];
};


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
bool RegisterUser(struct USERS * users, char * login, char * password){
    pthread_mutex_lock(&USERS_MUTEX);
    if(Login(users, login, password)==true){
        pthread_mutex_unlock(&USERS_MUTEX);
        return false; //user already exists
    }
    if(users->registeredUsersCount==MAX_USR_COUNT){
        pthread_mutex_unlock(&USERS_MUTEX);
        return false; //users limit reached 
    }
    strncpy(users->login[users->registeredUsersCount], login, LOGIN_LEN);
    strncpy(users->password[users->registeredUsersCount], password, PASSWD_LEN);
    users->registeredUsersCount++;
    pthread_mutex_unlock(&USERS_MUTEX);
    return true;
}

//funkcja opisującą zachowanie wątku - musi przyjmować argument typu (void *) i zwracać (void *)
void *ThreadBehavior(void *input)
{
    char login[LOGIN_LEN];
    char password[PASSWD_LEN];
        int connectionsocketdescriptor = *((int*)input);
    do{
    pthread_detach(pthread_self());

    const char * msg = "need your login\n";
    handle_error(write(connectionsocketdescriptor, msg, strlen(msg)));

    printf("%d\n", connectionsocketdescriptor);
    
    handle_error(read(connectionsocketdescriptor, login, LOGIN_LEN));

    handle_error(read(connectionsocketdescriptor, password, PASSWD_LEN));
    
    printf("%s\n",login);
    printf("%s\n",password);
    }
    while(!(strcmp(login, "admin\n")==0 && strcmp(password, "qwerty\n")==0)); //simple login for testing

    handle_error(write(connectionsocketdescriptor, "yo\n", 4));
    //you are logged now

    return NULL;
}

//funkcja obsługująca połączenie z nowym klientem
void gatherLoginData(int connection_socket_descriptor) {
    //wynik funkcji tworzącej wątek
    int create_result = 0;

    //uchwyt na wątek
    pthread_t thread1;

    //dane, które zostaną przekazane do wątku
    //TODO dynamiczne utworzenie instancji struktury thread_data_t o nazwie t_data (+ w odpowiednim miejscu zwolnienie pamięci)
    //TODO wypełnienie pól struktury


    //struct LOGIN_DATA * t_data  = malloc( sizeof(struct LOGIN_DATA));


   

    int fd = connection_socket_descriptor;


    create_result = pthread_create(&thread1, NULL, ThreadBehavior, (void *)&fd);
    handle_error(create_result);
    

    

    return;
    //TODO (przy zadaniu 1) odbieranie -> wyświetlanie albo klawiatura -> wysyłanie
}

int main(int argc, char* argv[])
{   
    //read data login from file

    struct USERS users;
    

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

       gatherLoginData(connection_socket_descriptor);
   }

   close(server_socket_descriptor);
   return(0);
}