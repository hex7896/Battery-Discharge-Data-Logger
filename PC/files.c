#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

FILE	*open_log_file(char *path)
{
	FILE	*file_stream;

	file_stream = fopen(path, "w");
	if (file_stream != NULL)
	{
		fprintf(file_stream,
			"Timestamp\tVoltage (V)\tCurrent (mA)\tDischarge Capacity (mAh)\n");
		return (file_stream);
	}
	perror("open_log_file: fopen");
	return (NULL);
}
