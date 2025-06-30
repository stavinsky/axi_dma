#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <signal.h>
#include <sched.h>
#include <time.h>
#include <errno.h>
#include <sys/param.h>

#include "dma-proxy.h"

static int test_size = BUFFER_SIZE;
static int num_transfers = 100000;
void output(uint8_t *data, int length) {
	write(STDOUT_FILENO, data, length);
}

int main()
{
	int fd = open("/dev/s2mm", O_RDWR);
	if (fd < 0)
	{
		perror("open");
		return 1;
	}

	struct channel_buffer *buf_ptr = (struct channel_buffer *)mmap(NULL, sizeof(struct channel_buffer) * RX_BUFFER_COUNT, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (buf_ptr == MAP_FAILED)
	{
		fprintf(stderr, "Failed to mmap rx channel\n");
		exit(EXIT_FAILURE);
	}

	int in_progress_count = 0, buffer_id = 0;
	int rx_counter = 0;

	for (buffer_id = 0; buffer_id < RX_BUFFER_COUNT; buffer_id += BUFFER_INCREMENT)
	{
		buf_ptr[buffer_id].length = test_size;
		ioctl(fd, START_XFER, &buffer_id);
		if (++in_progress_count >= num_transfers)
			break;
	}

	buffer_id = 0;
	while (1)
	{
		ioctl(fd, FINISH_XFER, &buffer_id);

		if (buf_ptr[buffer_id].status != PROXY_NO_ERROR)
		{
			fprintf(stderr, "Proxy rx transfer error, # transfers %d, # completed %d, # in progress %d, status: %d\n",
				   num_transfers, rx_counter, in_progress_count, buf_ptr[buffer_id].status);
			exit(1);
		}
		in_progress_count--;
		uint8_t *data = (uint8_t *)buf_ptr[buffer_id].buffer;
		output(data, buf_ptr[buffer_id].received);
		usleep(2550);
		ioctl(fd, START_XFER, &buffer_id);
		in_progress_count++;
		buffer_id += BUFFER_INCREMENT;
		buffer_id %= RX_BUFFER_COUNT;
	}
}
