#include <stdlib.h>
#include <stdio.h>
#include "../src/server.h"

int TEST_COUNT = 0;
int TEST_PASS = 0;

void assert_true(bool value, const char* test_name){
    TEST_COUNT++;
    if(value){
        TEST_PASS++;
    }else{
        fprintf(stderr, "test %s failed\n", test_name);
    }
    return;
}

void summary(){
    if(TEST_PASS==TEST_COUNT){
        printf("All tests passed.\n");
        exit(0);
    }else{
       printf("%d/%d tests passed.\n", TEST_PASS, TEST_COUNT);
       exit(-1); 
    }
    
}


int main(){
assert_true(true, "generic green assert :)");
{
struct TAGS tags;
initTags(&tags);
struct USERS users;
loadUserData(&users);

createNewTag(&tags, "tag testowy", "admin");

createNewTag(&tags, "drugi tag dla zmylki", "hackerman");
newMessage(getTagStructByName(&tags, "tag testowy"), "admin", "hejo");
newMessage(getTagStructByName(&tags, "tag testowy"), "admin", "witam na nowym tagu");
newMessage(getTagStructByName(&tags, "tag testowy"), "hackerman", "proba nieautoryzowanego dostepu przez osoby trzecie");

assert_true(tags.tagsCount==2 && tags.tag[0].messagesCount==2 && tags.tag[1].messagesCount==0, "tag adding test");

}


summary();
}

