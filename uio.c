// #include <fcntl.h>
// #include <poll.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <stdint.h>
// #include <sys/mman.h>

// #define FIFO_SIZE 0x1000

// int main() {
//     int uio_fd = open("/dev/uio0", O_RDWR);
//     if (uio_fd < 0) {
//         perror("open");
//         return -1;
//     }

//     struct pollfd fds;
//     fds.fd = uio_fd;
//     fds.events = POLLIN;

//     printf("Waiting for FIFO interrupt...\n");

//     while (1) {
//         int ret = poll(&fds, 1, -1); // Wait forever for interrupt
//         if (ret < 0) {
//             perror("poll");
//             break;
//         }

//         if (fds.revents & POLLIN) {
//             uint32_t count;
//             int bytes = read(uio_fd, &count, sizeof(count)); // Acknowledge interrupt
//             if (bytes != sizeof(count)) {
//                 perror("read");
//                 break;
//             }

//             printf("FIFO interrupt received! count=%u\n", count);


//             // Clear your FIFO interrupt condition if needed:
//             // *reg0 = ...;
//         }
//     }

//     close(uio_fd);
//     return 0;
// }
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>

int main(void)
{
        printf("start app\n");
    int fd = open("/dev/uio0", O_RDWR);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    while (1) {
        uint32_t info = 1; /* unmask */
        printf("start loop\n");
        ssize_t nb = write(fd, &info, sizeof(info));
        if (nb != (ssize_t)sizeof(info)) {
            perror("write\n");
            close(fd);
            exit(EXIT_FAILURE);
        }
        printf("write done\n");

        struct pollfd fds = {
            .fd = fd,
            .events = POLLIN,
        };

        int ret = poll(&fds, 1, -1);
        if (ret >= 1) {
            nb = read(fd, &info, sizeof(info));
            if (nb == (ssize_t)sizeof(info)) {
                /* Do something in response to the interrupt. */
                printf("Interrupt #%u!\n", info);
            }
        } else {
            perror("poll()\n");
            close(fd);
            exit(EXIT_FAILURE);
        }
    }

    close(fd);
    exit(EXIT_SUCCESS);
}