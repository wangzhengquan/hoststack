 
// struct winsize {
//    unsigned short ws_row;
//    unsigned short ws_col;
//    unsigned short ws_xpixel;   /* unused */
//    unsigned short ws_ypixel;   /* unused */
// };
 
		   
// struct termios
// {
// 	unsigned short c_iflag; /* */
// 	unsigned short c_oflag; /* */
// 	unsigned short c_cflag; /* */
// 	unsigned short c_lflag; /**/
// 	unsigned char c_line; /*line discipline */
// 	unsigned char c_cc[NCC]; /* */
// };

#include "usg_common.h"
#include <sys/mount.h>
#include <getopt.h>
#include <uuid.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>

int main() {
	struct termios ttyOrig = {};
	struct winsize ws= {};

	struct termios tty_std_termios = {};

  tty_std_termios.c_iflag = ICRNL | IXON;
  tty_std_termios.c_oflag = OPOST | ONLCR;
  tty_std_termios.c_cflag = B38400 | CS8 | CREAD | HUPCL;
  tty_std_termios.c_lflag = ISIG | ICANON | ECHO | ECHOE | ECHOK | ECHOCTL | ECHOKE | IEXTEN;
  // tty_std_termios.c_cc = INIT_C_CC;
  if (tcgetattr(STDIN_FILENO, &ttyOrig) == -1)
    err_msg(errno, "tcgetattr");
  if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) < 0)
    err_msg(errno, "ioctl-TIOCGWINSZ");

  printf("ws_row = %hu\n", ws.ws_row);
  printf("ws_col = %hu\n", ws.ws_col);

  printf("=============\n");
  printf("c_iflag=%hu\n", ttyOrig.c_iflag);
  printf("c_oflag=%hu\n", ttyOrig.c_oflag);
  printf("c_cflag=%hu\n", ttyOrig.c_cflag);
  printf("c_lflag=%hu\n", ttyOrig.c_lflag);
  printf("c_line=%c\n", ttyOrig.c_line);
  // printf("c_cc=%s\n", ttyOrig.c_cc);

  printf("=============\n");
  printf("c_iflag=%hu\n", tty_std_termios.c_iflag);
  printf("c_oflag=%hu\n", tty_std_termios.c_oflag);
  printf("c_cflag=%hu\n", tty_std_termios.c_cflag);
  printf("c_lflag=%hu\n", tty_std_termios.c_lflag);
  printf("c_line=%c\n", tty_std_termios.c_line);
  // printf("c_cc=%s\n", tty_std_termios.c_cc);
}
