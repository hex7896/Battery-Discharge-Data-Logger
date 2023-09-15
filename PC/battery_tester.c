#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include "battery_tester.h"

#define DEFAULT_TTY_DEV_PATH "/dev/ttyUSB0"
#define DEFAULT_LOG_FILE_PATH "./data.log"
#define OPCODE_START 0x11
#define OPCODE_POLL_V 0x22
#define OPCODE_POLL_I 0x33
#define OPCODE_STOP 0x36
#define ADC_BITS 12
#define MAX_ADC_READING 4095.0
#define CURRENT_SENSOR_SENSITIVITY 180 // in mV/A
#define VCC_VAL 5.13 // in V
#define V_VAL_CAL_FACTOR 0.98
#define I_VAL_CAL_FACTOR 1.2

char	*read_path_from_stdin(const char *default_path)
{
	char	path[4097] = {0};
	size_t	path_len;
	char	*ret_buf;
	char	*test;

	test = fgets(path, 4096, stdin);
	if (*test == '\n')
	{
		if (default_path == NULL)
		{
			perror("read_path_from_stdin: fgets");
			return (NULL);
		}
		memcpy(path, default_path, strlen(default_path));
	}
	fflush(stdin);
	printf("%s\n", path);
	path_len = strlen(path);
	ret_buf = malloc((path_len + 1) * sizeof(*ret_buf));
	if (ret_buf == NULL)
	{
		perror("read_path_from_stdin: malloc");
		return (NULL);
	}
	memcpy(ret_buf, path, path_len);
	ret_buf[path_len] = '\0';
	return (ret_buf);
}

void	execute_discharge_routine(int tty_fd, FILE *log_file_stream)
{
	char	input_buf[2] = {0};
	int16_t	v_reading;
	int16_t	i_reading;
	double	v_value;
	double	i_value;
	double	cap_value;
	time_t	time_start;
	time_t	time_start_prev;

	printf("Starting discharge. Enter 'q' to quit. \n");
	fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK); // make the reads non-blocking
	if (write_byte(tty_fd, OPCODE_START) == -1)
		return ;
	time_start_prev = 0;
	cap_value = 0;
	while (input_buf[0] != 'q' && input_buf[1] != '\n')
	{
		time_start = time(NULL);
		if (time_start_prev != 0)	// Not the first row, write capacity to file
		{
			cap_value += i_value * (time_start - time_start_prev) / 3600; // in mAh
			fprintf(log_file_stream, "%.3lf\n", cap_value); // in mAh
			printf("%.3lf\n", cap_value);
		}
		if (write_byte(tty_fd, OPCODE_POLL_V) == -1)
			break ;
		if (read_int16(tty_fd, &v_reading) == -1)
			break ;
		if (write_byte(tty_fd, OPCODE_POLL_I) == -1)
			break ;
		if (read_int16(tty_fd, &i_reading) == -1)
			break ;
		v_value = VCC_VAL * V_VAL_CAL_FACTOR * v_reading / MAX_ADC_READING; // in V
		i_value = ((VCC_VAL * 1000 * i_reading / MAX_ADC_READING) - (VCC_VAL * 1000 / 2) + 0.8) *
			1000 * I_VAL_CAL_FACTOR / CURRENT_SENSOR_SENSITIVITY; // in mA
		fprintf(log_file_stream, "%lu\t%.3lf\t%.0lf\t", time_start, v_value, i_value);
		fflush(log_file_stream);
		printf("%.3lf\t%.0lf\t", v_value, i_value);
		fflush(stdout);
		time_start_prev = time_start;
		if (v_value < 2.5)
			break ;
		while (time(NULL) - time_start < 5)
		{
			sleep(1);
			if (read(STDIN_FILENO, &input_buf, 2) != 2)
				input_buf[0] = '\0';
			else if (input_buf[0] == 'q' && input_buf[1] == '\n')
				break ;
		}
		fflush(stdin);
	}
	write_byte(tty_fd, OPCODE_STOP);
	fprintf(log_file_stream, "\n");
	printf("\n");
}

int	main(void)
{
	char	*dev_path;
	char	*log_file_path;
	FILE	*log_file_stream;
	int 	tty_fd;
	char	input_buf[2];

	printf("Please enter path to TTY device[%s]: ", DEFAULT_TTY_DEV_PATH);
	dev_path = read_path_from_stdin(DEFAULT_TTY_DEV_PATH);
	if (dev_path == NULL)
		return (-1);

	printf("Please enter path to log file[%s]: ", DEFAULT_LOG_FILE_PATH);
	log_file_path = read_path_from_stdin(DEFAULT_LOG_FILE_PATH);
	if (log_file_path == NULL)
	{
		free(dev_path);
		return (-1);
	}

	tty_fd = init_serial_term(dev_path);
	if (tty_fd == -1)
		return (-1);
	log_file_stream = open_log_file(log_file_path);

	printf("Please enter 'y' when ready: ");
	fflush(stdout);
	while (1)
	{
		if (read(STDIN_FILENO, &input_buf, 2) == 2
			&& input_buf[0] == 'y' && input_buf[1] == '\n')
			break ;
		fflush(stdin);
	}
	execute_discharge_routine(tty_fd, log_file_stream);
	close(tty_fd);
	fclose(log_file_stream);
	free(dev_path);
	free(log_file_path);
	return (0);
}
