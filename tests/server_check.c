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
        printf("test %s failed\n", test_name);
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
summary();
}

