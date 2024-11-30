#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

//Use lalloc!

typedef struct {
    pthread_mutex_t mutex;
    void*           data;
} Mutex;

Mutex* mutex_new(void* initial_data) {
  Mutex* mutex = malloc(sizeof(Mutex));
  if (mutex == NULL) {
    return NULL;
  }

  if (pthread_mutex_init(&mutex->mutex, NULL) != 0) {
    free(mutex);
    return NULL;
  }

  mutex->data = initial_data;
  return mutex;
}

void mutex_free(Mutex* mutex) {
  if (mutex != NULL) {
    pthread_mutex_destroy(&mutex->mutex);
    free(mutex);
  }
}

typedef struct {
    Mutex* mutex;
    void*  data;
} MutexGuard;

MutexGuard* mutex_lock(Mutex* mutex) {
  if (pthread_mutex_lock(&mutex->mutex) != 0) {
    return NULL;
  }

  MutexGuard* guard = (MutexGuard*)malloc(sizeof(MutexGuard));
  if (guard == NULL) {
    pthread_mutex_unlock(&mutex->mutex);
    return NULL;
  }

  guard->mutex = mutex;
  guard->data  = mutex->data;
  return guard;
}

void mutex_unlock(MutexGuard* guard) {
  if (guard != NULL) {
    pthread_mutex_unlock(&guard->mutex->mutex);
    free(guard);
  }
}

// New function for auto-unlocking guard
static inline void mutex_guard_cleanup(MutexGuard** guard) {
  if (*guard != NULL) {
    mutex_unlock(*guard);
    *guard = NULL;
  }
}

// Macro for creating an auto-unlocking guard
#define AUTO_LOCK(mutex) __attribute__((cleanup(mutex_guard_cleanup))) MutexGuard* guard = mutex_lock(mutex);

// Example usage
void* thread_function(void* arg) {
  Mutex* mutex = (Mutex*)arg;

  AUTO_LOCK(mutex);
  int* value = (int*)data;
  printf("Thread: value is %d\n", *value);
  (*value)++;

  return NULL;
}

int main() {
  int    shared_data = 0;
  Mutex* mutex       = mutex_new(&shared_data);

  pthread_t thread1, thread2;
  pthread_create(&thread1, NULL, thread_function, mutex);
  pthread_create(&thread2, NULL, thread_function, mutex);

  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);

  {
    AUTO_LOCK(mutex);
    int* final_value = (int*)data;
    printf("Main: final value is %d\n", *final_value);
  }

  mutex_free(mutex);
  return 0;
}