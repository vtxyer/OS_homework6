#include<stdio.h>
#include<stdlib.h>
#include<sys/mman.h>
#include<malloc.h>
#include<unistd.h>
#include<string.h>
#include<signal.h>

char *array;

void handler(){
    printf("%s\n", array);
}

int main(int argc, char **argv){

    int k;
    int size = 100;
    signal(SIGUNUSED, handler);
    array = (char *)malloc(sizeof(char)*size);    
    if(array==NULL){
        printf("alloc error\n");
        exit(1);
    }
    memset(array, 0, size);
    printf("pid:%d\narray address:0x%lx\n", getpid(), array);


    /*only for hang the process*/
    scanf("%d", &k);
    return 0;
}
