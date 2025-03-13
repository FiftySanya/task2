#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define SEGMENT_SIZE (128 * 1024 * 1024) // 128 MB
#define ITERATIONS 1000000

char data_segment[SEGMENT_SIZE] = {1};

char bss_segment[SEGMENT_SIZE];

double get_time_usec() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec * 1000000 + (double)tv.tv_usec;
}

double test_memory_access(char *memory, size_t size) {
    double start, end;
    int j;
    volatile char temp;
    volatile int dummy = 0;
    
    if (memory != bss_segment) {
        memset(memory, 'A', size);
    }
    
    start = get_time_usec();
    
    for (j = 0; j < ITERATIONS; j++) {
        temp = memory[(j * 16) % size];
        dummy += temp;
    }
    
    end = get_time_usec();
    
    if (dummy == 12345) {
        printf("This will never happen: %d\n", dummy);
    }
    
    return (end - start) / ITERATIONS;
}

double test_stack_segment() {
    char stack_segment[SEGMENT_SIZE];
    return test_memory_access(stack_segment, SEGMENT_SIZE);
}

double test_heap_segment() {
    char *heap_segment = (char *)malloc(SEGMENT_SIZE);
    if (heap_segment == NULL) {
        perror("Error allocating heap memory");
        exit(1);
    }
    
    double result = test_memory_access(heap_segment, SEGMENT_SIZE);
    free(heap_segment);
    return result;
}

double test_data_segment() {
    return test_memory_access(data_segment, SEGMENT_SIZE);
}

double test_bss_segment() {
    return test_memory_access(bss_segment, SEGMENT_SIZE);
}

int main() {
    printf("Measuring access time to main memory segments in FreeBSD\n");
    printf("--------------------------------------------------------\n");
    printf("Number of iterations: %d\n\n", ITERATIONS);
    
    printf("DATA segment (initialized data): %.3f microseconds per access\n", test_data_segment());
    printf("BSS segment (uninitialized data): %.3f microseconds per access\n", test_bss_segment());
    printf("HEAP segment (dynamic memory): %.3f microseconds per access\n", test_heap_segment());
    printf("STACK segment (stack): %.3f microseconds per access\n", test_stack_segment());

    return 0;
}