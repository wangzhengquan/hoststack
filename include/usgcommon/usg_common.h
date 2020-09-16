/*
 * Our own header, to be included before all standard system headers.
 */

#ifndef __USG_COMMON_H__
#define __USG_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <dirent.h>
#include <time.h>
#include <sched.h>

#include <sys/time.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/file.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <libgen.h>
/*
 * define int8_t uint8_t int16_t uint16_t int32_t uint32_t int64_t uint64_t
*/
#include <stdint.h>
#include <assert.h>


#ifdef __cplusplus
}
#endif



//c++ header
#ifdef __cplusplus

 
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <cstdlib>
#include <atomic>
#include <algorithm>
#include <iomanip>
#include <limits>

#include <initializer_list>
#include <vector>
#include <map>
#include <set>

#include <functional>
#include <thread>
#endif

#include "usg_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Signal wrappers */
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);



/* Our own error-handling functions */

void err_exit(int error, const char *fmt, ...);
void err_msg(int error, const char *fmt, ...);

char *ltrim(char *str, const char *seps);
char *rtrim(char *str, const char *seps);
char *trim(char *str, const char *seps);

char * str_join( const char *seperator, const char *first...);
char ** str_split(const char *str, const char *delim, int *arr_len);
char * array_join(char * const arr[], const char *seperator);
char * path_join(const char *path, ...);



static inline int 
strtoi(int num, char *str) 
{
	return sprintf(str, "%d", num); 

}



 
#ifdef __cplusplus
}
#endif



#ifdef __cplusplus

// static inline std::string& ltrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
// {
//     str.erase(0, str.find_first_not_of(chars));
//     return str;
// }
 
// static inline std::string& rtrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
// {
//     str.erase(str.find_last_not_of(chars) + 1);
//     return str;
// }
 
// static inline std::string& trim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
// {
//     return ltrim(rtrim(str, chars), chars);
// }


#endif

#endif
