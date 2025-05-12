#include "queue.h"
#include <stdio.h>


///////////////iteration functions
void print_number_queue(queue_t queue, void*data){

    (void)queue;

    int*num = (int*)data;

    printf("%d ",*num);
}

void print_or_delete(queue_t queue, void*data){

    (void)queue;

    int*num = (int*)data;

    if(*num==3 || *num==6){
        queue_delete(queue,data);
        return;
    }

    printf("%d ",*num);
}
////////////////////////////////

/////////////////////////tester functions
void test_destroy(){
    printf("--test_destroy--\n");
    queue_t q = queue_create();
    int ret = queue_destroy(q);

    if(ret==-1){
        printf("fail\n");
        return;
    }

    printf("pass\n");
}

void test_enqueue(){
    printf("--test enqueue--\n");
    queue_t q = queue_create();
    int data[10];

    for(int i=0;i<10;i++){
       data[i]=i;
       int ret=queue_enqueue(q, &data[i]);
       if(ret==-1){
           printf("fail\n");
           return;
       }
   }

    printf("pass\n");
}

//should print every number 0-9 except 3 or 6
void test_iteration_robustness(){
    printf("---test iteration robustness---\n");
    queue_t q = queue_create();

    int data[10];

    for(int i=0;i<10;i++){
        data[i]=i;
        queue_enqueue(q, &data[i]);
    }

    queue_iterate(q,print_or_delete);
    printf("pass if every number 0-9 except 3 or 6 is printed\n");
    queue_destroy(q);
}

//prints numbers except 1,6,3
void test_removals(){
    printf("----test removals------\n");

    queue_t q = queue_create();

    int data[10];

    for(int i=0;i<10;i++){
        data[i]=i;

        queue_enqueue(q, &data[i]);
    }

    queue_delete(q,&data[1]);
    queue_delete(q,&data[6]);
    queue_delete(q,&data[3]);

    queue_iterate(q,print_number_queue);
    printf("pass if every number 0-9 except 1,3,6 is printed\n");
    queue_destroy(q);
}

int main(){
    test_destroy();
    test_enqueue();
    test_removals();
    test_iteration_robustness();
}







