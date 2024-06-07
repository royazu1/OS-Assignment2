#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char **argv){
    int cd = channel_create();
    if (cd < 0) {
        printf("Failed to create channel\n");
        exit(1);
    }

    if (fork() == 0) { //child proc
        if (channel_put(cd, 42) < 0) {
        printf("Failed to put data in channel\n");
        exit(1);
        }
        else {
            printf("child has put the data 42 succesfuly\n");
        }

        if (channel_put(cd, 43) < 0) { // Sleeps until cleared
            printf("child failed to put the data 43\n");
        }
        else {
            printf("child has put the data 43 succesfuly\n");
        }
        // Handle error
        if (channel_destroy(cd) < 0) { // Sleeps until cleared
            printf("child has not deleted the channel\n");
        }
        else {
            printf("child has deleted the channel succesfully\n");
        }
        // Handle error
    } 
    else { //parent proc
        int data;
        if (channel_take(cd, &data) < 0) { // 42 take fail
        printf("Failed to take data from channel\n");
        }
        else { //42 take success
             printf("Taken data 42 from channel succesfully\n");
        }
        if ( (data = channel_take(cd, &data)) < 0) {  // 43
            printf("failed to take data 43 from channel\n");
        }
        else { //take 43 success
            printf("Taken data 43 from channel succesfully\n");
        }
        
        // Handle error
        data = channel_take(cd, &data); // Sleep until child destroys channel
        // Handle error
        printf("parent exiting..\n");
        exit(1);
    }

    printf("child exiting..\n");
    exit(0);
}