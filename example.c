#include "c_the_mutex_posix.h"
#include<stdio.h>

#define DATA_SIZE 8

//Example of custom type declarations
DECLARE_MUTEX_WRAPPER(int)
DECLARE_MUTEX_GUARD(int)

// Sample critical section of code

void* critical_section(void* wrapper) {
  CREATE_MUTEX_GUARD_TYPE((TheMutex_int*)wrapper,guard,TheGuard_int);
  printf("Critical section running...\n");
  for (int i=0;i<DATA_SIZE;i++){
    printf("Num:%i\n",((int*)guard.data)[i]);
    ((int*)guard.data)[i]-=1;
  }
  destroyTheGuard_int(&guard);

  printf("No mutex:0x%p\n",((void*)guard.data));

  CREATE_MUTEX_GUARD((TheMutex_int*)wrapper,guard2);
  printf("Second critical section running...\n");
  for (int i=0;i<DATA_SIZE;i++){
    printf("Num:%i\n",((int*)guard2.data)[i]);
    ((int*)guard2.data)[i]-=1;
  }
  return NULL;
}



int main() {
  int* data=(int*)calloc(DATA_SIZE,sizeof(int));
  for (int i=0;i<DATA_SIZE;i++) data[i]=INT_MAX-1;
  TheMutex_int wrapper;
  initTheMutex_int(&wrapper, data,NULL);

  pthread_t thread1, thread2;
  pthread_create(&thread1, NULL, critical_section, &wrapper);
  pthread_create(&thread2, NULL, critical_section, &wrapper);

  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);

  {
    CREATE_MUTEX_GUARD_TYPE(&wrapper,guard,TheGuard_int)
    for (int i=0;i<DATA_SIZE;i++){
      printf("Final:%i\n",((int*)guard.data)[i]);
    }
  }

  freeTheMutex_int(&wrapper);
  free(data);
  return 0;
}