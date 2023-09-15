#include <termios.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>

int	init_serial_term(char *dev_path)
{
	struct termios	config;
	int 			tty_fd;

	tty_fd = open(dev_path, O_RDWR);
	if (tty_fd == -1)
	{
		perror("init_serial_term: open");
		return (-1);
	}

	cfmakeraw(&config);
	// ignore modem control and enable receiver
	config.c_cflag |= (CLOCAL | CREAD);
	// disable flow control
	config.c_iflag &= ~(IXOFF | IXANY);

	// minimum number of bytes for read
	config.c_cc[VMIN] = 2;
	// how long to wait for input
	config.c_cc[VTIME] = 0;

	// set baud rate
	cfsetispeed(&config, B9600);
	cfsetospeed(&config, B9600);

	// write port configuration to driver
	if (tcsetattr(tty_fd, TCSANOW, &config) == -1)
	{
		perror("init_serial_term: tcsetattr");
		close(tty_fd);
		return (-1);
	}
	return (tty_fd);
}

int	write_byte(int fd, char c)
{
	if (write(fd, &c, 1) == 1)
		return (0);
	perror("write_byte: write");
	return (-1);
}

int	read_int16(int fd, int16_t *read_buf)
{
	if (read(fd, read_buf, 2) == 2)
		return (0);
	perror("read_int16: read");
	return (-1);
}
