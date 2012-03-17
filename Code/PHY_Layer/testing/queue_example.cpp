//#include "all.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <list>
#include <cstdlib>
#include <queue>
#include <deque>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

using namespace std;

void *thread1(void* num);
void *thread2(void* num);

//queues
queue<string> q1;
//queue<string> q2;

int main(){

pthread_t thread_1, thread_2;
int iret1, iret2;
iret1 = pthread_create( &thread_1, NULL, thread1, (void*) 1);
iret2 = pthread_create( &thread_2, NULL, thread2, (void*) 1);

for(int i=0;i<12;i++){

sleep(1);
cout<<"Main Thread Size:"<<q1.size()<<endl;

}

/*
cout<<q1.size()<<endl;
q1.push("Travis");
cout<<q1.front()<<endl;
cout<<q1.size()<<endl;
q1.pop();
cout<<q1.size()<<endl;
cout<<"Test"<<endl;
*/

return 0;


}


void *thread1(void* num){
cout<<q1.size()<<endl;
sleep(5);
q1.push("James");
return 0;
}

void *thread2(void* num){
cout<<q1.size()<<endl;
sleep(10);
q1.push("Travis");
return 0;
}
