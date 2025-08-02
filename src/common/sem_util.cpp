#include "sem_util.h"
#include "log.h"

int SemUtil::get(key_t key, unsigned int value) {
// printf("==================SemUtil::get===============================\n");
  int semid, perms;

  perms = S_IRUSR | S_IWUSR;

  semid = semget(key, 1, IPC_CREAT | IPC_EXCL | perms);

  if (semid != -1) { /* Successfully created the semaphore */
    union semun arg;
    struct sembuf sop;

    //logger.info("%ld: created semaphore\n", (long)getpid());

    arg.val = 0; /* So initialize it to 0 */
    if (semctl(semid, 0, SETVAL, arg) == -1)
      err_exit(errno, "semctl 1");
    //logger.info("%ld: initialized semaphore\n", (long)getpid());

    /* Perform a "no-op" semaphore operation - changes sem_otime
       so other processes can see we've initialized the set. */

    sop.sem_num = 0; /* Operate on semaphore 0 */
    sop.sem_op = value;
    sop.sem_flg = 0;
    if (semop(semid, &sop, 1) == -1)
      err_exit(errno, "semop");
    //logger.info("%ld: completed dummy semop()\n", (long)getpid());

  } else { /* We didn't create the semaphore set */

    if (errno != EEXIST) { /* Unexpected error from semget() */
      err_exit(errno, "semget 1");

    } else { /* Someone else already created it */
      const int MAX_TRIES = 10;
      int j;
      union semun arg;
      struct semid_ds ds;

      semid = semget(key, 1, perms); /* So just get ID */
      if (semid == -1)
        err_exit(errno, "semget 2");

     // logger.info("%ld: got semaphore key\n", (long)getpid());
      /* Wait until another process has called semop() */

      arg.buf = &ds;
      for (j = 0; j < MAX_TRIES; j++) {
        //logger.info("Try %d\n", j);
        if (semctl(semid, 0, IPC_STAT, arg) == -1)
          err_exit(errno, "semctl 2");

        if (ds.sem_otime != 0) /* Semop() performed? */
          break;               /* Yes, quit loop */
        sleep(1);              /* If not, wait and retry */
      }

      if (ds.sem_otime == 0) /* Loop ran to completion! */
        err_exit(errno, "Existing semaphore not initialized");
    }
  }
  return semid;
}

/* Release semaphore - increment it by 1 */
int SemUtil::inc(int semId) {
  struct sembuf sops;

  sops.sem_num = 0;
  sops.sem_op = 1;
  sops.sem_flg = SEM_UNDO;

  return semop(semId, &sops, 1);
}
/* Reserve semaphore (blocking), return 0 on success, or -1 with 'errno'
   set to EINTR if operation was interrupted by a signal handler */

/* Reserve semaphore - decrement it by 1 */
int SemUtil::dec(int semId) {
  struct sembuf sops;

  sops.sem_num = 0;
  sops.sem_op = -1;
  sops.sem_flg = SEM_UNDO;

  while (semop(semId, &sops, 1) == -1)
    if (errno != EINTR) {
      // err_msg(errno, "SemUtil::dec");
      return -1;
    }

  return 0;
}

int SemUtil::dec_nowait(int semId) {
  struct sembuf sops;

  sops.sem_num = 0;
  sops.sem_op = -1;
  sops.sem_flg = IPC_NOWAIT | SEM_UNDO;
 

  return semop(semId, &sops, 1);
}

int SemUtil::dec_timeout(const int semId, const struct timespec *timeout) {
  struct sembuf sops;

  sops.sem_num = 0;
  sops.sem_op = -1;
  sops.sem_flg = SEM_UNDO;

  while (semtimedop(semId, &sops, 1, timeout) == -1)
    if (errno != EINTR) {
      // err_msg(errno, "SemUtil::dec_timeout");
      return -1;
    }

  return 0;
}


/**
 * If sem_op equals 0, the value of the semaphore is checked to see whether it
 * currently equals 0. If it does, the operation completes immediately; otherwise,
 * semop() blocks until the semaphore value becomes 0.
 */
int SemUtil::zero(int semId) {
// logger.debug("%d: SemUtil::dec\n", semId);
  struct sembuf sops;

  sops.sem_num = 0;
  sops.sem_op = 0;
  sops.sem_flg = 0;

  while (semop(semId, &sops, 1) == -1)
    if (errno != EINTR) {
      // err_msg(errno, "SemUtil::zero");
      return -1;
    }

  return 0;
}


int SemUtil::zero_nowait(int semId) {
  struct sembuf sops;

  sops.sem_num = 0;
  sops.sem_op = 0;
  sops.sem_flg = IPC_NOWAIT;

  

  return semop(semId, &sops, 1);
}

int SemUtil::zero_timeout(const int semId, const struct timespec *timeout) {
  struct sembuf sops;

  sops.sem_num = 0;
  sops.sem_op = 0;
  sops.sem_flg = 0;

  while (semtimedop(semId, &sops, 1, timeout) == -1)
    if (errno != EINTR) {
      // err_msg(errno, "SemUtil::zero_timeout");
      return -1;
    }

  return 0;
}




int SemUtil::set(int semId, int val) {
  union semun arg;
  arg.val = val;
  return semctl(semId, 0, SETVAL, arg);
}


void SemUtil::remove(int semid) {
  union semun dummy;
  if (semctl(semid, 0, IPC_RMID, dummy) == -1)
    err_msg(errno, "SemUtil::remove");
}

