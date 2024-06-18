#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAX_PRINT 100

int isPrime(int num) {
    int divisor=2;
    while (divisor < num) {
        if (num % divisor == 0) {
            return 0;
        }
        divisor++;
    }
    return 1;
}

int main(int argc, char** argv) {
    //maybe
    int num_checkers= 0; 
    
    if (argc != 2) {
        printf("Supply a single argument - the number of checkers!\n");
    }
    else { // should be greater than 3
        if ((num_checkers= atoi(argv[1])) < 1) {
            printf("The number of prime checkers has to be 3 or more!\n");
            exit(1);
        }
    }
    int num_checkers_backup=num_checkers;
    // DEBUGGING: WHAT'S THE ISSUE WITH READING FROM STDIN?
    char msg[4];
    msg[3]='\0';
    printf("prime process pid=%d\n",getpid());
    printf("enter 3 letters max..\n");
    gets(msg,3);
    //
    int primes_cd, print_cd;
    
    Generate: //we come back to this label when the user prompts us to generate again
    num_checkers= num_checkers_backup;

    if ( (print_cd = channel_create()) < 0) {
        printf("Couldn't create a print channel, exiting...\n");
        exit(1);
    }
    else {
        printf("Created channel with cid=%d\n", print_cd);
    }
    if ( (primes_cd = channel_create()) < 0) {
        printf("Couldn't create a primes channel, exiting...\n");
        exit(1);
    }
    else {
        printf("Created channel with cid=%d\n", primes_cd);
    }
  
    
    if (fork() == 0) { //printer child proc
            int num_printed=0;
            while (num_printed < MAX_PRINT) {
                int prime_to_print=-1;
                if (channel_take(print_cd,&prime_to_print) < 0) {
                    printf("Channel is unused - can't print! , exiting...\n");
                    exit(1);
                }
                else { //taken prime from channel successfully
                    printf("The currently printed prime number is: %d\n" , prime_to_print);
                }
                num_printed++;
            }
            if (channel_destroy(print_cd) < 0) { //printed 100 primes already, destroy print channel
                printf("Printing proc couldn't destroy print channel..\n");
            }
            else {
                printf("Printer proc with pid=%d destroyed print channel successfully\n",getpid());
            }
            exit(0);
        
    }
    else { //number genereator proc

             while (num_checkers > 0) {
                if (fork() == 0) { //child checker - tries to take from prime channel and put in print channel, indefinitely.
                    int checker_pid= getpid();
                    while (1) { //take and put until printing proc destroys the channel
                        int curr_prime=-1;
                        if ( channel_take(primes_cd,&curr_prime) == 0) {
                            if (isPrime(curr_prime)) {  //only put prime numbers for print channel
                                if (channel_put(print_cd, curr_prime) == 0) {
                                printf("Checker proc with pid=%d has put the prime number %d successfully\n", checker_pid, curr_prime);
                                }
                                else { //print channel has been destroyed - race to destroy the prime channel
                                    if (channel_destroy(primes_cd) < 0) {
                                        printf("Printing proc couldn't destroy print channel..\n");
                                    }
                                    else {
                                        printf("Checker with pid=%d destroyed print channel successfully\n", getpid());
                                    }
                                    exit(0); //
                                }
                            }
                        }
                        else { //take from prime channel failed, exit!
                            exit(0);
                        }
                    }
                }
                num_checkers--;
            }

            int num=2; 
            while(1) {
                if (channel_put(primes_cd,num) < 0) { //put numbers to the prime channel indefinitely..
                    char prompt;
                    printf("Prime proc-Prime channel was destroyed, exiting..\n");
                    printf("Do you want to generate again? Y/N\n");
                    printf("pid=%d\n",getpid());
                    while (wait(0) != -1);
                    while(read(0,&prompt,1)) { //clean buffer so the read() call will block!
                        if (prompt == '\n')
                        break;
                    }
                    read(0,&prompt,1); //here we finally know the stdin buffer is empty.. great success
                    
                    if (prompt == 'Y') {
                        printf("Entered Y - generating again\n");
                        goto Generate;
                    }
                    else {
                        printf("Exiting..\n");
                    }

                    exit(1);
                }
                else {
                    //printf("Prime proc has put the prime number %d successfully\n");
                    
                }
                num++;
            }
    }
}
