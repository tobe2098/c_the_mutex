#include <stdio.h>
#include <stdlib.h>
#include "c_the_mutex_posix.h"

#define DATA_SIZE 8
#ifndef INT_MAX
#define INT_MAX   (((long long)1) << (sizeof(int) * 8) - 1) - 1
#endif
// Example of custom type declarations
DECLARE_MUTEX_WRAPPER(int)
DECLARE_MUTEX_GUARD(int)

// Sample critical section of code

void* critical_section(void* wrapper) {
  CREATE_MUTEX_GUARD_TYPE((TheMutex_int*)wrapper, guard, TheGuard_int);
  printf("Critical section with guards running...\n");
  for (int i = 0; i < DATA_SIZE; i++) {
    printf("Num:%i\n", ((int*)guard.data)[i]);
    ((int*)guard.data)[i] -= 1;
  }
  destroyTheGuard_int(&guard);

  printf("No mutex:0x%p\n", ((void*)guard.data));

  CREATE_MUTEX_GUARD((TheMutex_int*)wrapper, guard2);
  printf("Second critical section with guards running...\n");
  for (int i = 0; i < DATA_SIZE; i++) {
    printf("Num:%i\n", ((int*)guard2.data)[i]);
    ((int*)guard2.data)[i] -= 1;
  }
  return NULL;
}

void* critical_section_unordered(void* wrapper) {
  // CREATE_MUTEX_GUARD_TYPE((TheMutex_int*)wrapper, guard, TheGuard_int);
  printf("Critical section without guards running...\n");
  for (int i = 0; i < DATA_SIZE; i++) {
    printf("Num:%i\n", (((TheMutex_int*)wrapper)->__data_ptr)[i]);
    (((TheMutex_int*)wrapper)->__data_ptr)[i] -= 1;
  }
  // destroyTheGuard_int(&guard);

  // printf("No mutex:0x%p\n", ((void*)guard.data));

  // CREATE_MUTEX_GUARD((TheMutex_int*)wrapper, guard2);
  printf("Second critical section without guards running...\n");
  for (int i = 0; i < DATA_SIZE; i++) {
    printf("Num:%i\n", (((TheMutex_int*)wrapper)->__data_ptr)[i]);
    (((TheMutex_int*)wrapper)->__data_ptr)[i] -= 1;
  }
  return NULL;
}

int main() {
  int* data = (int*)calloc(DATA_SIZE, sizeof(int));
  for (int i = 0; i < DATA_SIZE; i++) {
    data[i] = INT_MAX - 1;
  }
  TheMutex_int wrapper;
  initTheMutex_int(&wrapper, data, NULL);

  pthread_t thread1, thread2;
  pthread_create(&thread1, NULL, critical_section, &wrapper);
  pthread_create(&thread2, NULL, critical_section, &wrapper);

  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);
  pthread_create(&thread1, NULL, critical_section_unordered, &wrapper);
  pthread_create(&thread2, NULL, critical_section_unordered, &wrapper);

  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);
  {
    CREATE_MUTEX_GUARD_TYPE(&wrapper, guard, TheGuard_int)
    for (int i = 0; i < DATA_SIZE; i++) {
      printf("Final:%i\n", ((int*)guard.data)[i]);
    }
  }
  int result = pthread_mutex_trylock(&wrapper.mutex_internal);                                         
  if (result == 0) {                                                                                      
    pthread_mutex_unlock(&wrapper.mutex_internal);                                                     
    printf("The mutex was properly handled.\n");
  } else {                                                                                                
    printf("The mutex was not properly handled.\n");
  }  
  freeTheMutex_int(&wrapper);
  free(data);
  return result;
}