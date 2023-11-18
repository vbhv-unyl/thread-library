#include "speed.h"
#define nl printf("\n")

void fx(){
    printf("-------------- Entered fx function of testing file ----------------");
    nl;

    for(int i = 0; i < 100; i++){
        printf("%d ", i + 1);
    }
    nl;

    printf("-------------- Exited fx function of testing file -----------------");
    nl;

}

void gx(){
    printf("------------- Entered gx function of testing file -----------------");
    nl;

    char str[100];
    for(int i = 0; i < 100; i++){
        str[i] = (char)(i+1);
    }
    nl;

    printf("--------------- Exited gx function of testing file ----------------");
    nl;
}

int main(){
    printf("----------- This is the main function of testing file -------------");
    nl;nl;

    int ID1 = -1, ID2 = -1;
    ID1 = create(fx);
    ID2 = create(gx);
    printf("Thread ID = %d, CREATED!", ID1);
    nl;nl;

    start();
    run(ID1);
    run(ID2);

    printf("-------------- Exited main function of testing file ---------------");
    nl;

    printf("-------------------------------------------------------------------");
    nl;

    return 0;
} 