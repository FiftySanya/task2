#include <stdio.h>
#include <time.h>
#include <limits.h>

int main() {
    printf("Size of time_t: %zu bytes\n", sizeof(time_t));

    // Визначення максимального значення time_t залежно від його розміру
    time_t max_time;
    if (sizeof(time_t) == 4) {  // 32-бітний режим
        max_time = (time_t)INT_MAX;
    } 
    else if (sizeof(time_t) == 8) {  // 64-бітний режим
        max_time = (time_t)LLONG_MAX;
    } 
    else {
        printf("Unknown size of time_t\n");
        return 1;
    }

    // Конвертація в час
    struct tm *tm_info = localtime(&max_time);
    if (tm_info) {
        printf("Maximum representable time: %s", asctime(tm_info));
    } 
    else {
        printf("Time conversion failed, value might be too large\n");
    }

    return 0;
}
