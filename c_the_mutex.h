#ifndef _C_THE_MUTEX_HEADER
#define _C_THE_MUTEX_HEADER
#ifdef _WIN32 
#include <malloc.h>
#include <pthread.h>
#else
#include <alloca.h>
#include <pthread.h>

#endif

#include<stdio.h>

#define DATA_SIZE 8

// Quality of life 1: A wrapper for the data with the mutex
typedef struct TheMutex {
    pthread_mutex_t mutex_internal;
    void*           data;
} TheMutex;

typedef struct TheGuard {
    void*           data;
    pthread_mutex_t* mutex_ptr;
} TheGuard;


// Quality of life 1.5: Cannot lock twice by requiring data adress and nulling it afterwards
void* lockTheMutexGetData(TheMutex* the_mutex);
int   unlockTheMutexWithData(TheMutex* the_mutex, void** data);

// void lockTheMutex(TheMutex* the_mutex);
// void unlockTheMutex(TheMutex* the_mutex);

// Quality of life 2: A code block wrapper so you can forget
#define WITH_MUTEX_LOCK(mutex, code_block) \
  pthread_mutex_lock(&mutex);              \
  do {                                     \
    code_block                             \
  } while (0);                             \
  pthread_mutex_unlock(&mutex);

// Quality of life 3: Out of scope, out of mind
// Guards with cleanup attribute? LALLOC!!!

static void destroyGuard(TheGuard **guard_ptr) {
  if (guard_ptr&&*guard_ptr) {
    if ((*guard_ptr)->mutex_ptr)
    pthread_mutex_unlock((*guard_ptr)->mutex_ptr);
    (*guard_ptr)->mutex_ptr=NULL;
    if ((*guard_ptr)->data)(*guard_ptr)->data=NULL;
  }
}

// #define CREATE_MUTEX_GUARD(mutex_wrap)                                       \
//     __attribute__((cleanup(destroyGuard))) TheGuard *guard = alloca(sizeof(TheGuard));                             \
//     do {                                                                    \
//         guard->data = (mutex_wrap)->data;                                   \
//         if (pthread_mutex_lock(guard->mutex_ptr) != 0) {                    \
//             perror("Failed to lock mutex");                                 \
//             exit(EXIT_FAILURE);                                             \
//         }                                                                   \
//     } while (0)

#define CREATE_MUTEX_GUARD(mutex_wrap, mutex_guard_ptr)                                       \
         TheGuard *__guard __attribute__((cleanup(destroyGuard))) = alloca(sizeof(TheGuard));                         \
        __guard->mutex_ptr = &((mutex_wrap)->mutex_internal);               \
        __guard->data = (mutex_wrap)->data;                                   \
        if (pthread_mutex_lock(__guard->mutex_ptr) != 0) {                    \
            perror("Failed to lock mutex");                                 \
            exit(EXIT_FAILURE);                                             \
        }                                                                   \
        mutex_guard_ptr=__guard; /* Return the guard pointer */                               \

// Sample critical section of code
void critical_section(TheMutex* wrapper) {
  TheGuard* guard;
  CREATE_MUTEX_GUARD(wrapper,guard);
  printf("Critical section running...\n");
  for (int i=0;i<DATA_SIZE;i++){
    printf("Num:%i",(intptr_t)guard->data);
    // printf("%i,", ((int*)guard->data)[i]);
  }
}

int main() {
  int* data=(int*)calloc(DATA_SIZE,sizeof(int));
  TheMutex wrapper;
  pthread_mutex_init(&wrapper.mutex_internal, NULL);
  wrapper.data=data;
  // Use the macro to lock the mutex, execute the code, and unlock the mutex

  // TheGuard* guard=CREATE_MUTEX_GUARD(wrapper);

  critical_section(&wrapper);
  // WITH_MUTEX_LOCK(mutex, {
  //   critical_section();
  //   // You can add more code here if needed
  // });

  pthread_mutex_destroy(&wrapper.mutex_internal);
  free(data);
  return 0;
}
#endif