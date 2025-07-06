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
#include <sys/mman.h>
#include <poll.h>
#include "dma-proxy.h"

#define MMIO_SIZE 0x10000

static int test_size = BUFFER_SIZE;
static int num_transfers = 100000;

#define AXI_BASE       0x43C00000
#define MAP_SIZE       0x1000  // map one 4KB page
#define OFFSET_ENABLE  0x08
#define OFFSET_DIVIDER 0x20
#define GPIO_NUMBER "512"

void enable_i2s() {
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) { perror("open /dev/mem"); exit(1); }

    void *map_base = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, AXI_BASE);
    if (map_base == MAP_FAILED) { perror("mmap"); close(fd); exit(1); }

    volatile uint32_t *reg_enable = (uint32_t *)((char*)map_base + OFFSET_ENABLE);
    uint32_t val = *reg_enable;
    val |= 0x1;
    *reg_enable = val;

    volatile uint8_t *reg_divider = (uint8_t *)((char*)map_base + OFFSET_DIVIDER);
    *reg_divider = 0x02;

    munmap(map_base, MAP_SIZE);
    close(fd);
    return ;
}

void output(uint8_t *data, int length)
{
	// static uint8_t started = 0;
	// uint32_t *words = (uint32_t*) data;
	// for (int i=0; i<(length/4); i++) {
	// 	uint8_t preamble = words[i] & 0xF;
	// 	if (!started && preamble == 1) {
	// 		started = 1;
	// 	}
	// 	int32_t sample = (int32_t)(((words[i] & 0xffffff0 )>>4)<<8);
	// 	sample = sample >> 8;
	// 	if (started) {

	// 		write(STDOUT_FILENO, &sample, sizeof(sample)) ;
	// 	}


	// }

	

	write(STDOUT_FILENO, data, length);
}
void wait_for_data(volatile uint32_t *regs)
{
	while (1)
	{
		if (regs[0] > 1024)
		{
			return;
		}
		usleep(1000);
	}
};


void set_framer_on_off(char *value) {
    int fd;
    char path[64];

	if (!(value[0] == '0' || value[0] == '1')) {
		fprintf(stderr, "Invalid value: must be \"0\" or \"1\"\n");
		return;
	}

	if (access("/sys/class/gpio/gpio" GPIO_NUMBER, F_OK) == -1) {
	    fd = open("/sys/class/gpio/export", O_WRONLY);
	    if (fd < 0) { perror("export"); return; }
	    write(fd, GPIO_NUMBER, sizeof(GPIO_NUMBER) - 1);
	    close(fd);
	}

    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s/direction", GPIO_NUMBER);
    fd = open(path, O_WRONLY);
    if (fd < 0) { perror("direction"); return ; }
    write(fd, "out", 3);
    close(fd);

    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s/value", GPIO_NUMBER);
    fd = open(path, O_WRONLY);
    if (fd < 0) { perror("value"); return ; }
    write(fd, value, 1);
    close(fd);

}
void wait_for_data2(int uio_fd)
{
	while (1)
	{
		uint32_t info = 1; /* unmask */
		ssize_t nb = write(uio_fd, &info, sizeof(info));
		if (nb != (ssize_t)sizeof(info))
		{
			perror("write\n");
		}

		struct pollfd fds = {
			.fd = uio_fd,
			.events = POLLIN,
		};

		int ret = poll(&fds, 1, -1);
		if (ret >= 1)
		{
			nb = read(uio_fd, &info, sizeof(info));
			if (nb == (ssize_t)sizeof(info))
			{
				return;
			}
		}
		else
		{
			perror("poll()\n");
		}
	}
}

int main()
{
	// enable_i2s();
	usleep(150000); // allow the queue to fill 
	int fd = open("/dev/s2mm", O_RDWR);
	if (fd < 0)
	{
		perror("open");
		return 1;
	}
	int uio_fd = open("/dev/uio0", O_RDWR);
	if (uio_fd < 0)
	{
		perror("open");
		exit(EXIT_FAILURE);
	}
	void *regs = mmap(NULL, MMIO_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, uio_fd, 0);
	if (regs == MAP_FAILED)
	{
		perror("mmap");
		exit(1);
	}
	volatile uint32_t *regs32 = (uint32_t *)regs;
	fprintf(stderr,"0 %d\n", regs32[0]);
	fprintf(stderr,"1 %d\n", regs32[1]);
	fprintf(stderr,"2 %d\n", regs32[2]);
	fprintf(stderr,"3 %d\n", regs32[3]);
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
		// usleep(2550);
		// wait_for_data2(uio_fd);
		wait_for_data(regs32);
		ioctl(fd, START_XFER, &buffer_id);
		in_progress_count++;
		buffer_id += BUFFER_INCREMENT;
		buffer_id %= RX_BUFFER_COUNT;
	}
}
