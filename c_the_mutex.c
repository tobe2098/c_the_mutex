#include "c_the_mutex.h"

void* lockTheMutexGetData(TheMutex* the_mutex) {
  if (the_mutex == NULL) {
    return NULL;
  }
  pthread_mutex_lock(&(the_mutex->mutex_internal));
  return the_mutex->__data_ptr;
}

int unlockTheMutexWithData(TheMutex* the_mutex, void** data) {
  if (data==NULL|| the_mutex==NULL||*data==NULL||*data != the_mutex->__data_ptr||pthread_mutex_unlock(&(the_mutex->mutex_internal))!=0) {
    return -1;
  }
  *data=NULL;
  return 0;
}
