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
#define SIZE (1024*4096)
#define PAGE_SIZE 4096
#define PAGE_BITS 12
#define CL_BITS 6
#define ACCESSES (SIZE / sizeof(int) * 2) //(16 * 1024)

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

static uint64_t lfsr_fast(uint64_t lfsr)
{
  lfsr ^= lfsr >> 7;
  lfsr ^= lfsr << 9;
  lfsr ^= lfsr >> 13;
  return lfsr;
}

#define TEST_OP(OPERATION) \
    reset_op(test2, test1, size); \
    _mm_mfence();  \
    OPERATION; \
    _mm_mfence(); \
    printf("Dest: %d Src: %d\n", *test2, *test1); \
    _mm_mfence(); \
    m5_reset_stats \
    random_test(test2, test1, size); \
    m5_dump_stats

void reset_op(int* dest, int* src, uint64_t size) {
    int lfsr = 1;
    for(int i = 0; i < size / sizeof(int); i++) {
        lfsr = lfsr_fast(lfsr);
        src[i] = lfsr;
        dest[i] = 0;
    }
}

void random_test(int* dest, int* src, uint64_t size) {
    auto start = TIME_NOW;
    int index = 0;
    for(int i = 0; i < ACCESSES; i++) {
        index = (index + dest[index]) % (size / sizeof(int));
    }
    auto stop = TIME_NOW;
    printf("Verify: %d, time %ld\n", index, TIME_DIFF(stop,start));
}

int main(int argc, char *argv[])
{
    size_t size = SIZE;
    int *test1 = (int*)aligned_alloc(PAGE_SIZE, size);
    int *test2 = (int*)aligned_alloc(PAGE_SIZE, size);
    srand(0);
    printf("%p\n", test1);
    printf("%p\n", test2);
    recv(-2, test1, size, 0xdeadbeef);
    TEST_OP(memcpy(test2, test1, size));
    return 0;
}