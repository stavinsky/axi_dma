#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint32_t prev, curr;
    size_t nread;
    size_t offset = 0; // track byte offset in the stream


    // Read the first number
    nread = fread(&prev, sizeof(uint32_t), 1, stdin);
    if (nread != 1) {
        fprintf(stderr, "No data to read.\n");
        return 1;
    }

    offset += sizeof(uint32_t);

    while (1) {
        nread = fread(&curr, sizeof(uint32_t), 1, stdin);
        if (nread != 1) {
            break; // EOF reached or error
        }

        // printf("%d %d\n",offset, curr);
        if (curr != prev + 1 && prev != 65535 && curr != 0) {
            printf("Sequence error: offset: %u, prev=%u, curr=%u\n", offset, prev, curr);
        }

        prev = curr;
        offset += sizeof(uint32_t);
    }

    return 0;
}