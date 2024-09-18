/*
 * Our own header, to be included before all standard system headers.
 */

#ifndef __USG_COMMON_H__
#define __USG_COMMON_H__

#include "comm_inc.h"
#include "comm_typedef.h"

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

/**
 * @ seperator 分割符
 */
char * str_join( const char *seperator, const char *first, ...);
/**
 * 把字符串以@delim分割为数组，数组的地址存放在@arr_addr
 */
int str_split(const char *str, const char *delim, char *** arr_addr);
char * array_join(char **, const char *seperator);
char * path_join(const char *path, ...);
/**
 * 反向查询字符串needle
 *
 * @return 查询到的字符串的地址
 */
char *strstr_r( char *str, const  char * needle);
// 递归创建文件夹
int mkdir_r(const char *pathname, mode_t mode);
// 在dirfd指向的目录下面创建文件夹
int mkdiratfd_r(int dirfd, const char *pathname, mode_t mode);
int mkdirat_r(const char * dir, const char *pathname, mode_t mode);


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
