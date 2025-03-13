#include <stdio.h>
#include <stdlib.h>

int global_var;
static int static_var;

int initialized_global = 42;
static int initialized_static = 100;

void dummy_function() {
    printf("This is a dummy function\n");
}

int main() {
    int local_var;
    
    static int static_local_var;
    
    static int initialized_static_local = 200;
    
    int *heap_var = malloc(sizeof(int));
    *heap_var = 300;
    
    printf("BSS Segment:\n");
    printf("  - Uninitialized global variable: %p\n", (void*)&global_var);
    printf("  - Uninitialized static variable: %p\n", (void*)&static_var);
    printf("  - Uninitialized static local variable: %p\n", (void*)&static_local_var);
    
    printf("\nData Segment:\n");
    printf("  - Initialized global variable: %p\n", (void*)&initialized_global);
    printf("  - Initialized static variable: %p\n", (void*)&initialized_static);
    printf("  - Initialized static local variable: %p\n", (void*)&initialized_static_local);
    
    printf("\nStack Segment:\n");
    printf("  - Local variable: %p\n", (void*)&local_var);
    
    printf("\nHeap Segment:\n");
    printf("  - Dynamically allocated variable: %p\n", (void*)heap_var);
    
    printf("\nText Segment (Code):\n");
    printf("  - Address of main function: %p\n", (void*)main);
    printf("  - Address of dummy_function: %p\n", (void*)dummy_function);
    
    free(heap_var);
    return 0;
}