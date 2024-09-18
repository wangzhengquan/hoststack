#ifndef _SEM_UTIL_H
#define _SEM_UTIL_H  

#include "usg_common.h"

namespace SemUtil {

	int get(key_t key, unsigned int value);
	int dec(int semId);
	int dec_nowait(int semId);
	int dec_timeout(const int semId, const struct timespec * timeout);
	int zero(int semId);
	int zero_nowait(int semId);
	int zero_timeout(const int semId, const struct timespec * timeout);
	int inc(int semId);
	int set(int semId, int val);

	void remove(int semid);


	

}

#endif
