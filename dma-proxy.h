#define BUFFER_SIZE (256 * 4 )	 	/* must match driver exactly */
#define BUFFER_COUNT 32					/* driver only */

#define TX_BUFFER_COUNT 	1				/* app only, must be <= to the number in the driver */
#define RX_BUFFER_COUNT 	32				/* app only, must be <= to the number in the driver */
#define BUFFER_INCREMENT	1				/* normally 1, but skipping buffers (2) defeats prefetching in the CPU */

#define FINISH_XFER 	_IOW('a','a',int32_t*)
#define START_XFER 		_IOW('a','b',int32_t*)
#define XFER 			_IOR('a','c',int32_t*)

struct channel_buffer {
	unsigned int buffer[BUFFER_SIZE / sizeof(unsigned int)];
	enum proxy_status { PROXY_NO_ERROR = 0, PROXY_BUSY = 1, PROXY_TIMEOUT = 2, PROXY_ERROR = 3 } status;
	unsigned int length;
	size_t received;
} __attribute__ ((aligned (1024)));		/* 64 byte alignment required for DMA, but 1024 handy for viewing memory */
