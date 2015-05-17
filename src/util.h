#ifndef gre_util_h
#define gre_util_h

#define synchronized(mutex, cond, a)  pthread_mutex_lock( mutex ); a; pthread_cond_wait ( cond, mutex ); pthread_mutex_unlock ( mutex );

void log_message ( const char *formatStr, ... );

#endif
