#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <x86intrin.h>
#include <sys/syscall.h>         /* Definition of SYS_* constants */
#include <sys/socket.h>
#include <string.h>
#define PAGE_SIZE 4096
#define PAGE_BITS 12
#define CL_BITS 6

#define TEST_OP(OPERATION) \
    reset_op(test2, test1, size); \
    _mm_mfence();  \
    m5_reset_stats(0, 0); \
    OPERATION; \
    _mm_mfence(); \
    m5_dump_stats(0, 0); \
    printf("Dest: %d Src: %d\n", *test2, *test1); \
    fflush(stdout); \

#include <chrono>
using namespace std::chrono;
#define TIME_NOW high_resolution_clock::now()
#define TIME_DIFF(a, b) duration_cast<microseconds>(a - b).count()

#define m5_reset_stats \
    { \
        auto start = TIME_NOW;

#define m5_dump_stats \
        printf("Time: %lu us\n", TIME_DIFF(TIME_NOW, start)); \
    }

void reset_op(int* dest, int* src, uint64_t size) {
    for(int i = 0; i < size / sizeof(int); i += PAGE_SIZE / sizeof(int)) {
        src[i]  = 500;
        dest[i] = 100;
    }
}

int main(int argc, char *argv[])
{
    size_t size;
    int *test1, *test2;
    size = 4096;
    test1 = (int*)aligned_alloc(PAGE_SIZE, size);
    test2 = (int*)aligned_alloc(PAGE_SIZE, size);
    recv(-2, test1, size, 0xdeadbeef);
    printf("%p\n", test1);
    printf("%p\n", test2);
    TEST_OP(memcpy(test2, test1, size));
    return 0;
}