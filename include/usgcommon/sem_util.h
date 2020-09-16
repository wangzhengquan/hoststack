#ifndef PCSEM_H
#define PCSEM_H  

#include "usg_common.h"
#include "usg_typedef.h"

namespace SemUtil {

	int get(key_t key, unsigned int value);
	int dec(int semId);
	int dec_nowait(int semId);
	int dec_timeout(const int semId, const struct timespec * timeout);
	int zero(int semId);
	int zero_nowait(int semId);
	int zero_timeout(const int semId, const struct timespec * timeout);
	int inc(int semId);
	void remove(int semid);

	void set(int semId, int val);

}

#endif
