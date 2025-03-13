#include <stdio.h>

void recurse(int depth) {
    int arr[1000];
    printf("Depth: %d, Address of arr: %p\n", depth, (void*)&arr);
    
    if (depth > 0) {
        recurse(depth - 1);
    }
}

int main() {
    recurse(5);
    return 0;
}
