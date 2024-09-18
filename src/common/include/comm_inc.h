/*
 * Our own header, to be included before all standard system headers.
 */

#ifndef __COMMON_INCLUDE_H__
#define __COMMON_INCLUDE_H__

// c header
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

#include <initializer_list>
#include <vector>
#include <queue>
#include <map>
#include <set>

#include <iterator>

#include <cstdlib>
#include <atomic>
#include <algorithm>
#include <iomanip>
#include <limits>


#include <functional>
#include <thread>

#include <cassert>
#endif

#endif
