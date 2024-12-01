
//TASKS
//PUT THE MUTEX INITIALIZER AND DESTROYER JUST DEALS WITH INTERNALS, NOT WITH ALLOCATION DONE
//MAKE A DECLARATION FOR GENERIC TYPES IN DATA, AND MAKE MACRO ACCEPT THE GUARD TYPE
//BEFORE MAKE THE GUARD HAVE TYPE OF MUTEX WITH CONDITIONAL TO POSIX OR WINDOWS
//THEN MAKE DIFFERENT TYPES OF LOCK WITH TRY, NOT TRY, THE SHARED ONES, ETC
//THEN MAKE THE LOCKING AND UNLOCKING FUNCTIONS BE ACCEPTED VIA THE MACRO, MAKING IT CUSTOMIZABLE

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
    void*           __data_ptr;
} TheMutex;

#define DECLARE_MUTEX_WRAPPER(type) \
  typedef struct TheMutex_##type {\
    pthread_mutex_t mutex_internal; \
    type* __data_ptr; \
  } TheMutex_##type;  \
static int initTheMutex_##type(TheMutex_##type* the_mutex,type* data, const pthread_mutexattr_t * attr){\
  if(the_mutex==NULL||data==NULL) return -1;\
  int result=pthread_mutex_init(&the_mutex->mutex_internal,attr);\
  if (result!=0){\
    return -1;\
  }\
  the_mutex->__data_ptr=data;\
  return 0;\
}\
static int freeTheMutex_##type(TheMutex_##type* the_mutex){\
  if(the_mutex==NULL || the_mutex->__data_ptr==NULL) return -1;\
  int result = pthread_mutex_trylock(&the_mutex->mutex_internal);\
  if (result == 0) {\
      pthread_mutex_unlock(&the_mutex->mutex_internal); \
  } else { \
    return -1; \
  } \
  pthread_mutex_destroy(&the_mutex->mutex_internal); \
  the_mutex->__data_ptr=NULL; \
  return 0; \
}

static int initTheMutex(TheMutex* the_mutex,void* data, const pthread_mutexattr_t * attr){
  if(the_mutex==NULL||data==NULL) return -1;
  int result=pthread_mutex_init(&the_mutex->mutex_internal,attr);
  if (result!=0){
    return -1;
  }
  the_mutex->__data_ptr=data;
  return 0;
}

static int freeTheMutex(TheMutex* the_mutex){
  if(the_mutex==NULL || the_mutex->__data_ptr==NULL) return -1;

  int result = pthread_mutex_trylock(&the_mutex->mutex_internal);
  if (result == 0) {
      pthread_mutex_unlock(&the_mutex->mutex_internal);  // Don't forget to unlock!
  } else {
    return -1;
  }
  pthread_mutex_destroy(&the_mutex->mutex_internal);
  the_mutex->__data_ptr=NULL;
  return 0;
}

// Quality of life 1.5: Cannot lock twice by requiring data adress and nulling it afterwards
void* lockTheMutexGetData(TheMutex* the_mutex);
int   unlockTheMutexWithData(TheMutex* the_mutex, void** data);

// Quality of life 2: A code block wrapper so you can forget
#define WITH_MUTEX_LOCK(mutex, code_block) \
  pthread_mutex_lock(&mutex);              \
  do {                                     \
    code_block                             \
  } while (0);                             \
  pthread_mutex_unlock(&mutex);

// Quality of life 3: Out of scope, out of mind. Rust-like mutex guards!

//Generic with void
typedef struct TheGuard {
    void*           data;
    pthread_mutex_t* mutex_ptr;
} TheGuard;

//Declare with custom type directly
#define DECLARE_MUTEX_GUARD(type) \
  typedef struct TheGuard_##type {\
    type* data; \
    pthread_mutex_t* mutex_ptr; \
  } TheGuard_##type; \
  static void destroyGuard_##type(TheGuard_##type **guard_ptr) {\
    if (guard_ptr&&*guard_ptr) {\
      if ((*guard_ptr)->mutex_ptr){\
        pthread_mutex_unlock((*guard_ptr)->mutex_ptr);\
        (*guard_ptr)->mutex_ptr=NULL;\
      }\
      if ((*guard_ptr)->data)(*guard_ptr)->data=NULL;\
    }\
  }



static void destroyGuard(TheGuard **guard_ptr) {
  if (guard_ptr&&*guard_ptr) {
    if ((*guard_ptr)->mutex_ptr){
#ifdef DEBUG_MUTEX
      printf("Unlocking...");
#endif
      pthread_mutex_unlock((*guard_ptr)->mutex_ptr);
      (*guard_ptr)->mutex_ptr=NULL;
#ifdef DEBUG_MUTEX
      printf("Unlocked.\n");
#endif
    }
    if ((*guard_ptr)->data)(*guard_ptr)->data=NULL;
  }
}

#ifdef DEBUG_MUTEX

#define CREATE_MUTEX_GUARD(mutex_wrap, mutex_guard_ptr)                                       \
         TheGuard *mutex_guard_ptr __attribute__((cleanup(destroyGuard))) = alloca(sizeof(TheGuard));                         \
         printf("Creating mutex guard...\n");\
        mutex_guard_ptr->mutex_ptr = &((mutex_wrap)->mutex_internal);               \
        mutex_guard_ptr->data = (mutex_wrap)->__data_ptr;                                   \
         printf("Copied data.");\
        if (pthread_mutex_lock(mutex_guard_ptr->mutex_ptr) != 0) {                    \
            perror("Failed to lock mutex");                                 \
            exit(EXIT_FAILURE);                                             \
        }                                                                   \
         printf("Locked.\n");\

#define CREATE_MUTEX_GUARD_TYPE(mutex_wrap, mutex_guard_ptr,guard_type)  \
         guard_type *mutex_guard_ptr __attribute__((cleanup(destroyGuard_##guard_type))) = alloca(sizeof(guard_type));                         \
         printf("Creating mutex guard...\n");\
        mutex_guard_ptr->mutex_ptr = &((mutex_wrap)->mutex_internal);               \
        mutex_guard_ptr->data = (mutex_wrap)->__data_ptr;                                   \
         printf("Copied data.");\
        if (pthread_mutex_lock(mutex_guard_ptr->mutex_ptr) != 0) {                    \
            perror("Failed to lock mutex");                                 \
            exit(EXIT_FAILURE);                                             \
        }                                                                  \
         printf("Locked.\n");\

#else

#define CREATE_MUTEX_GUARD(mutex_wrap, mutex_guard_ptr)                                               \
        TheGuard *mutex_guard_ptr __attribute__((cleanup(destroyGuard))) = alloca(sizeof(TheGuard));  \
        mutex_guard_ptr->mutex_ptr = &((mutex_wrap)->mutex_internal);                                 \
        mutex_guard_ptr->data = (mutex_wrap)->__data_ptr;                                             \
        if (pthread_mutex_lock(mutex_guard_ptr->mutex_ptr) != 0) {                                    \
            exit(EXIT_FAILURE);                                                                       \
        }                                                                                             \

#define CREATE_MUTEX_GUARD_TYPE(mutex_wrap, mutex_guard_ptr,guard_type)                                   \
        guard_type *mutex_guard_ptr __attribute__((cleanup(destroyGuard_##guard_type))) = alloca(sizeof(guard_type));  \
        mutex_guard_ptr->mutex_ptr = &((mutex_wrap)->mutex_internal);                                     \
        mutex_guard_ptr->data = (mutex_wrap)->__data_ptr;                                                 \
        if (pthread_mutex_lock(mutex_guard_ptr->mutex_ptr) != 0) {                                        \
            exit(EXIT_FAILURE);                                                                           \
        }                                                                                                 \

#endif

//Example of custom type declarations
DECLARE_MUTEX_WRAPPER(int)
DECLARE_MUTEX_GUARD(int)

// Sample critical section of code
void critical_section(TheMutex* wrapper) {
  CREATE_MUTEX_GUARD_TYPE(wrapper,guard,TheGuard_int);
  printf("Critical section running...\n");
  for (int i=0;i<DATA_SIZE;i++){
    printf("Num:%i\n",((int*)guard->data)[i]);
  }
  destroyGuard(&guard);
  for (int i=0;i<DATA_SIZE;i++){
    printf("Num:%p\n",((void*)guard->data));
  }
  CREATE_MUTEX_GUARD(wrapper,guard2);
  for (int i=0;i<DATA_SIZE;i++){
    printf("Num:%i\n",((int*)guard2->data)[i]);
  }
}



int main() {
  int* data=(int*)calloc(DATA_SIZE,sizeof(int));
  for (int i=0;i<DATA_SIZE;i++) data[i]=INT_MAX-i;
  TheMutex_int wrapper;
  initTheMutex_int(&wrapper, data,NULL);
  // Use the macro to lock the mutex, execute the code, and unlock the mutex
  
  // TheGuard* guard=CREATE_MUTEX_GUARD(wrapper);

  critical_section(&wrapper);
  // WITH_MUTEX_LOCK(mutex, {
  //   critical_section();
  //   // You can add more code here if needed
  // });

  freeTheMutex(&wrapper);
  free(data);
  return 0;
}
#endif