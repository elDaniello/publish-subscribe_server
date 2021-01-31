# Publish-subscribe server 1.0.0
* By default runs server on localhost:12345 (can be changed in config.h file)
* Run by typing ```make run``` or simply ```make```
## Test login credentials:
* login: `admin`
* password: `qwerty`
## Can handle both CLI and GUI clients
* you can use [dedicated GUI client](https://github.com/j-magdalena/client.git)
* or you can use command line unix ```nc``` (netcat)
***
## Custom protocol
There are three types of commands:
* before login commands
* after login single string
* after login multiple string 
***
After login multiple string returns a bunch of strings separated by '\t', with first string representing how many substrings are in it (excluding the number string). Every message shoud end with '\n', and multiple string message shoud end with '\t\n'. This is made to keep netcat compatibility. 
***
## Functionality
Every user can create tag or subscribe exiting one. Only the creator of the tag is allowed to post on it (may it change later).
