#ifndef BATTER_TESTER_H
# define BATTER_TESTER_H

/*	serial.c	*/
int		init_serial_term(char *dev_path);

/*	files.c	*/
FILE	*open_log_file(char *path);

/*	serial.c	*/
int		write_byte(int fd, char c);
int		read_int16(int fd, int16_t *read_buf);

#endif
