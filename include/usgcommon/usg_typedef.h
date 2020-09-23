#ifndef _USG_TYPEDEF_H__
#define _USG_TYPEDEF_H__

#ifdef __cplusplus
extern "C" {
#endif


#if ! defined(__FreeBSD__) && ! defined(__OpenBSD__) && \
                ! defined(__sgi) && ! defined(__APPLE__)
                /* Some implementations already declare this union */

union semun {                   /* Used in calls to semctl() */
    int                 val;
    struct semid_ds *   buf;
    unsigned short *    array;
#if defined(__linux__)
    struct seminfo *    __buf;
#endif
};

#endif

#ifdef _WIN32
#define PATH_SEPERATOR   "\\"
#else
#define PATH_SEPERATOR   "/"
#endif

/* Default file permissions are DEF_MODE & ~DEF_UMASK */
#define DEF_MODE   S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH
#define DEF_UMASK  S_IWGRP|S_IWOTH
/*
 * Default file access permissions for new files.
 */
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

/*
 * Default permissions for new directories.
 */
#define DIR_MODE  (FILE_MODE | S_IXUSR | S_IXGRP | S_IXOTH)


#define MIN(a,b)  ((a) < (b) ? (a) : (b))
#define MAX(a,b)  ((a) > (b) ? (a) : (b))

#define BLANK "\t\n\v\f\r "
/* Misc constants */
#define	MAXLINE	 1024  /* Max text line length */
#define MAXBUF   8192  /* Max I/O buffer size */

#ifdef __cplusplus
}
#endif

#endif  /* #ifndef  _USG_TYPEDEF_H__ */
