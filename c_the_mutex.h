#ifndef _C_THE_MUTEX_HEADER
#define _C_THE_MUTEX_HEADER
#ifdef _WIN32 
#include <malloc.h>
#include <pthread.h>
#else
#include <alloca.h>
#include <pthread.h>

#endif
// Quality of life 1: A wrapper for the data with the mutex
typedef struct TheMutex {
    pthread_mutex_t mutex_internal;
    void*           data;
} TheMutex;

typedef struct TheGuard {
    void*           data;
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

static void mutex_unlock_cleanup(pthread_mutex_t **mutex_ptr) {
  if (mutex_ptr && *mutex_ptr) {
    pthread_mutex_unlock(*mutex_ptr);
  }
}

// Function to create and lock a mutex guard
static inline TheGuard *create_mutex_guard(TheMutex *mutex) {
    // Allocate the guard on the stack
    TheGuard *guard = alloca(sizeof(TheGuard));
    
    // Set up the guard
    guard->mutex = mutex;
    pthread_mutex_lock(mutex);
    
    // Register the cleanup function
    __attribute__((cleanup(mutex_unlock_cleanup))) pthread_mutex_t **cleanup_ptr = &guard->mutex;
    
    return guard;
}

// Sample critical section of code
void critical_section() {
  printf("Critical section running...\n");
}

int main() {
  pthread_mutex_t mutex;
  pthread_mutex_init(&mutex, NULL);

  // Use the macro to lock the mutex, execute the code, and unlock the mutex
  WITH_MUTEX_LOCK(mutex, {
    critical_section();
    // You can add more code here if needed
  });

  pthread_mutex_destroy(&mutex);
  return 0;
}
#endif