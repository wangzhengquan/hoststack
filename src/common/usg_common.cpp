#include "usg_common.h"
#include <errno.h> /* for definition of errno */
#include "log.h"



static void _dolog(const char * level, const char *file, const int line, const char *fmt, ...);


/************************************
 * Wrappers for Unix signal functions
 ***********************************/

/* $begin sigaction */
handler_t *Signal(int signum, handler_t *handler)
{
  struct sigaction action, old_action;

  action.sa_handler = handler;
  sigemptyset(&action.sa_mask); /* Block sigs of type being handled */
  action.sa_flags = SA_RESTART; /* Restart syscalls if possible */

  if (sigaction(signum, &action, &old_action) < 0)
  {
    err_msg(errno, "UsgCommon Signal:");
    return SIG_ERR;
  }
  return (old_action.sa_handler);
}

 

char *ltrim(char *str, const char *seps)
{
  if (str == NULL)
  {
    return str;
  }
  size_t totrim;
  if (seps == NULL)
  {
    seps = "\t\n\v\f\r ";
  }
  totrim = strspn(str, seps);
  if (totrim > 0)
  {
    size_t len = strlen(str);
    if (totrim == len)
    {
      str[0] = '\0';
    }
    else
    {
      memmove(str, str + totrim, len + 1 - totrim);
    }
  }
  return str;
}

char *rtrim(char *str, const char *seps)
{
  if (str == NULL)
  {
    return str;
  }
  int i;
  if (seps == NULL)
  {
    seps = "\t\n\v\f\r ";
  }
  i = strlen(str) - 1;
  while (i >= 0 && strchr(seps, str[i]) != NULL)
  {
    str[i] = '\0';
    i--;
  }
  return str;
}

char *trim(char *str, const char *seps)
{
  if (str == NULL)
  {
    return str;
  }
  return ltrim(rtrim(str, seps), seps);
}


char * str_join( const char *seperator, const char *first, ...)
{
  size_t i, len, size = 1024;
  va_list vl;
  char *str, *buf;
  if (first == NULL)
  {
    return NULL;
  }
  len = strlen(first);
  if (len >= size )
  {
    size = len * 2;
  }

  buf = (char *)malloc(size);
  if (NULL == buf) return NULL;

  strcpy(buf, first);

  va_start(vl, first);
  i = 1;
  while ( (str = va_arg(vl, char *) ) != NULL )
  {

    len += strlen(seperator);
    if (len >= size )
    {
      size = len * 2;
      buf = (char *)realloc(buf, size);
    }
    strcat(buf,  seperator);
    len += strlen(str);
    if (len >= size )
    {
      size = len * 2;
      buf = (char *)realloc(buf, size);
    }
    strcat(buf,  str);
    i++;
  }
  va_end(vl);
  buf[len] = '\0';
  return buf;
}


char * array_join(char** arr, const char *seperator)
{
  size_t size = 0, i = 0, arr_len = 0;

  while (arr[i] != NULL)
  {
    size += strlen(arr[i]);
    i++;
  }
  arr_len = i;
  if (arr_len == 0 || size == 0)
  {
    return NULL;
  }

  char *buf = (char *)malloc(size + strlen(seperator) * (arr_len - 1) + 1);
  if (NULL == buf)
  {
    err_exit(errno, "array_join malloc:");
    return NULL;
  }

  strcpy(buf, arr[0]);

  i = 1;
  while (i < arr_len)
  {
    strcat(buf,  seperator);
    strcat(buf,  arr[i]);
    i++;
  }

  return buf;

}


char * path_join(const char *path, ...)
{
  size_t i, spn, len, size = 1024;
  va_list vl;
  char *str;
  if (path == NULL)
  {
    return NULL;
  }

  len = strlen(path);
  if (len >= size )
  {
    size = len * 2;
  }
  char *buf = (char *)malloc(size);
  if (NULL == buf) return NULL;

  strcpy(buf, path);

  va_start(vl, path);
  i = 1;
  while ((str = va_arg(vl, char *)) != NULL)
  {

    if (strcmp(buf + len - strlen(PATH_SEPERATOR), PATH_SEPERATOR) == 0)
    {
      len -= strlen(PATH_SEPERATOR);
      *(buf + len) = '\0';
    }

    if ( (spn = strspn(str, PATH_SEPERATOR)) > 0 )
    {
      str = str + spn;
    }

    len += strlen(PATH_SEPERATOR);
    if (len >= size )
    {
      size = len * 2;
      buf = (char *)realloc(buf, size);
    }
    strcat(buf,  PATH_SEPERATOR);

    len += strlen(str);
    if (len >= size )
    {
      size = len * 2;
      buf = (char *)realloc(buf, size);
    }
    strcat(buf,  str);
    i++;
  }
  va_end(vl);
  buf[len] = '\0';
  return buf;
}

int str_split(const char *_str, const char *delim, char *** arr_addr)
{
// char *[] str_split( const char *_str, const char *delim,  int *arr_len) {

  size_t len = 64;
  size_t i = 0;
  char *token;
  if (_str == NULL)
  {

    return NULL;
  }
  char *str = strdup(_str);
  char **arr = (char **)calloc(len, sizeof(char *));
  if (arr == NULL)
  {
    goto LABEL_ERROR;
  }

  token = strtok(str, delim);
  while (token != NULL)
  {
    if (i >= len)
    {
      len *= 2 ;
      arr = (char **)realloc((void*)arr, len * sizeof(char *));
      if (arr == NULL)
      {
        goto LABEL_ERROR;
        return NULL;
      }
    }
    arr[i] = strdup(token);
    token = strtok(NULL, delim);
    i++;
  }

  if (i >= len)
  {
    len += 1 ;
    arr = (char **)realloc((void*)arr, len * sizeof(char *));
    if (arr == NULL)
    {
      goto LABEL_ERROR;
      return 0;
    }
  }
  arr[i] = NULL;
  *arr_addr = arr;
  // if(arr_len != NULL) {
  //   *arr_len = i;
  // }
  free(str);
  return i;

LABEL_ERROR:
  free(str);
  err_msg(errno, "str_split calloc");
  return 0;
}



/**
 * 反向查询字符串needle
 *
 * @return 查询到的字符串的地址
 */
char *strstr_r( char *str,  const char * needle)
{
  size_t needle_len = strlen(needle);
  if (str == NULL || needle == NULL)
  {
    return NULL;
  }

  char *start = str ;
  char *end = str + strlen(str) - needle_len;

  while (end >= start)
  {

    if (strncmp(end, needle, needle_len) == 0)
    {
      return end;
    }
    end--;
  }

  return NULL;
}

int mkdir_r(const char *pathname, mode_t mode)
{
  char *parent;
  char *parent_end;

  if (access(pathname, F_OK) == 0)
  {
    return 0;
  }

  // 根目录
  if (strcmp(pathname, PATH_SEPERATOR ) == 0)
  {
    if (mkdir(pathname, mode) != 0)
    {
      // err_msg(errno, "1 usg_common mkdirr");
      return -1;
    }

    return 0;
  }

  parent = strdup(pathname);
LABEL_FIND_PARENT:
  if ( (parent_end = strstr_r(parent, PATH_SEPERATOR)) != NULL)
  {
    *parent_end = '\0';
    if (mkdir_r(parent, mode) == -1)
    {
      free(parent);
      return -1;
    }
  }

  free(parent);
  // printf("pathname=%s\n", pathname);

  //遍历到最头了
  if(strlen(pathname) == 0) {
    return 0;
  }

  if (mkdir(pathname, mode) != 0)
  {
    // err_msg(errno, "2 usg_common mkdirr pathname= %s", pathname);
    return -1;
  }

  return 0;
}



int mkdiratfd_r(int dirfd, const char *pathname, mode_t mode)
{
  char *parent;
  char *parent_end;

  if (faccessat(dirfd, pathname, F_OK, AT_EACCESS) == 0)
  {
    return 0;
  }

  // 根目录
  if (strcmp(pathname, PATH_SEPERATOR ) == 0)
  {
    if (mkdir(pathname, mode) != 0)
    {
      err_msg(errno, "1 usg_common mkdirr");
      return -1;
    }

    return 0;
  }

  parent = strdup(pathname);

LABEL_FIND_PARENT:
  if ( (parent_end = strstr_r(parent, PATH_SEPERATOR)) != NULL)
  {
    if (parent_end + strlen(PATH_SEPERATOR)  == parent + strlen(parent) )
    {
      *parent_end = '\0';
      goto LABEL_FIND_PARENT;
    }

    *parent_end = '\0';
    if (mkdiratfd_r(dirfd, parent, mode) == -1)
    {
      free(parent);
      return -1;
    }
  }

  free(parent);

  if (mkdirat(dirfd, pathname, mode) != 0)
  {
    err_msg(errno, "2 usg_common mkdirr %s", pathname);
    return -1;
  }

  return 0;
}

int mkdirat_r(const char * dir, const char *pathname, mode_t mode)
{
  DIR * baseDir = opendir(dir);
  if (baseDir == NULL)
  {
    err_msg(errno, "open dir %s", dir);
    return -1;
  }

  return mkdiratfd_r(dirfd(baseDir), pathname, DIR_MODE);
}

