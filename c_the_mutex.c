#include "c_the_mutex.h"

void* lockTheMutexGetData(TheMutex* the_mutex) {
  if (the_mutex == NULL) {
    return NULL;
  }
  pthread_mutex_lock(&(the_mutex->mutex_internal));
  return the_mutex->data;
}

int unlockTheMutexWithData(TheMutex* the_mutex, void** data) {
  if (*data != the_mutex->data) {
    return -1;
  }
  pthread_mutex_unlock(&(the_mutex->mutex_internal));
  return 0;
}
