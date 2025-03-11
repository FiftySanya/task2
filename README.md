Завдання 1: Визначення моменту переповнення time_t
Опис
Це завдання досліджує момент переповнення типу time_t, який використовується в UNIX для представлення часу як кількості секунд з 1 січня 1970 року (епохи UNIX). Ми аналізуємо поведінку на 32- та 64-бітних системах.

Вихідний код
Файл: time_overflow.c

c

Collapse

Wrap

Copy
#include <stdio.h>
#include <time.h>

int main() {
    // Перевірка, чи є time_t знаковим
    time_t t = -1;
    if (t > 0) {
        printf("time_t is unsigned\n");
    } else {
        printf("time_t is signed\n");
    }

    // Максимальне значення для 32-бітної системи: 2^31 - 1
    time_t max_32 = 2147483647;
    struct tm *tm_32 = localtime(&max_32);
    printf("32-bit time_t overflows on: %s", asctime(tm_32));

    // Максимальне значення для 64-бітної системи: 2^63 - 1
    time_t max_64 = 9223372036854775807LL;
    struct tm *tm_64 = localtime(&max_64);
    if (tm_64) {
        printf("64-bit time_t overflows on: %s", asctime(tm_64));
    } else {
        printf("64-bit time_t overflow date is too far in the future to compute\n");
    }

    return 0;
}
Команди для компіляції
Для 32-бітної системи (якщо підтримується):
bash

Collapse

Wrap

Copy
gcc -m32 -o time_overflow time_overflow.c
Для 64-бітної системи (стандартна компіляція):
bash

Collapse

Wrap

Copy
gcc -o time_overflow time_overflow.c
Приклади виведення
На 32-бітній системі:
text

Collapse

Wrap

Copy
time_t is signed
32-bit time_t overflows on: Tue Jan 19 03:14:07 2038
64-bit time_t overflow date is too far in the future to compute
На 64-бітній системі:
text

Collapse

Wrap

Copy
time_t is signed
32-bit time_t overflows on: Tue Jan 19 03:14:07 2038
64-bit time_t overflow date is too far in the future to compute
Пояснення
На 32-бітних системах time_t переповниться 19 січня 2038 року через обмеження 32-бітного знакового цілого числа (2^31 - 1 секунд).
На 64-бітних системах переповнення відбудеться через мільярди років, і стандартні функції, такі як localtime, не можуть його обчислити.
Завдання 2.2: Дослідження сегментів виконуваного файлу
Опис
Це завдання аналізує, як різні типи змінних (глобальні, локальні, ініціалізовані та неініціалізовані) впливають на розміри сегментів text, data та bss у виконуваному файлі.

Вихідний код
1. Базова програма "Hello, World"
Файл: hello_base.c

c

Collapse

Wrap

Copy
#include <stdio.h>

int main() {
    printf("Hello, world!\n");
    return 0;
}
2. Додавання неініціалізованого глобального масиву
Файл: hello_global.c

c

Collapse

Wrap

Copy
#include <stdio.h>

int global_array[1000];

int main() {
    printf("Hello, world!\n");
    return 0;
}
3. Ініціалізація глобального масиву
Файл: hello_global_init.c

c

Collapse

Wrap

Copy
#include <stdio.h>

int global_array[1000] = {1};

int main() {
    printf("Hello, world!\n");
    return 0;
}
4. Локальні масиви
Файл: hello_local.c

c

Collapse

Wrap

Copy
#include <stdio.h>

int main() {
    int local_array[1000];
    int local_array2[1000] = {1};
    printf("Hello, world!\n");
    return 0;
}
Команди для компіляції та аналізу
Скомпілюйте кожну програму:
bash

Collapse

Wrap

Copy
gcc -o hello_base hello_base.c
gcc -o hello_global hello_global.c
gcc -o hello_global_init hello_global_init.c
gcc -o hello_local hello_local.c
Перевірте розмір файлу:
bash

Collapse

Wrap

Copy
ls -l hello_*
Перегляньте розміри сегментів:
bash

Collapse

Wrap

Copy
size hello_base
size hello_global
size hello_global_init
size hello_local
Приклади виведення
Базова програма (hello_base):
text

Collapse

Wrap

Copy
$ size hello_base
   text    data     bss     dec     hex filename
   2288     544      16    2848     b20 hello_base
З неініціалізованим масивом (hello_global):
text

Collapse

Wrap

Copy
$ size hello_global
   text    data     bss     dec     hex filename
   2288     544    4016    6848    1ac0 hello_global
З ініціалізованим масивом (hello_global_init):
text

Collapse

Wrap

Copy
$ size hello_global_init
   text    data     bss     dec     hex filename
   2288    4544      16    6848    1ac0 hello_global_init
З локальними масивами (hello_local):
text

Collapse

Wrap

Copy
$ size hello_local
   text    data     bss     dec     hex filename
   2288     544      16    2848     b20 hello_local
Пояснення
Неініціалізовані глобальні змінні йдуть у сегмент bss.
Ініціалізовані глобальні змінні — у сегмент data.
Локальні змінні розміщуються на стеку під час виконання і не впливають на розміри сегментів у файлі.
Завдання 2.3: Визначення розташування стека та сегментів
Опис
Це завдання досліджує адреси різних сегментів пам’яті: text, data, bss, стека та купи.

Вихідний код
Файл: segments.c

c

Collapse

Wrap

Copy
#include <stdio.h>
#include <stdlib.h>

int global_var;
static int static_var;

int main() {
    int local_var;
    static int static_local_var;
    int *heap_var = malloc(sizeof(int));
    
    printf("Global variable (BSS) address: %p\n", &global_var);
    printf("Static variable (BSS) address: %p\n", &static_var);
    printf("Local variable (stack) address: %p\n", &local_var);
    printf("Static local variable (BSS) address: %p\n", &static_local_var);
    printf("Heap variable address: %p\n", heap_var);
    printf("Function (text) address: %p\n", main);
    
    free(heap_var);
    return 0;
}
Команди для компіляції та запуску
bash

Collapse

Wrap

Copy
gcc -o segments segments.c
./segments
Приклади виведення
text

Collapse

Wrap

Copy
Global variable (BSS) address: 0x804a000
Static variable (BSS) address: 0x804a004
Local variable (stack) address: 0x7fffffffde4c
Static local variable (BSS) address: 0x804a008
Heap variable address: 0x801000000
Function (text) address: 0x8048120
Пояснення
text: Низькі адреси (наприклад, 0x8048120).
data/bss: Середні адреси (наприклад, 0x804a000).
Купа: Зазвичай після data/bss (наприклад, 0x801000000).
Стек: Високі адреси (наприклад, 0x7fffffffde4c).
Завдання 2.4: Дослідження стека процесу
Опис
Це завдання аналізує стек процесу за допомогою інструментів gstack або GDB, показуючи ієрархію викликів функцій.

Вихідний код
Файл: stacker.c

c

Collapse

Wrap

Copy
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MSG "In function %20s; &localvar = %p\n"

static void bar_is_now_closed(void) {
    int localvar = 5;
    printf(MSG, __func__, &localvar);
    printf("\n Now blocking on pause()...\n");
    pause();
}

static void bar(void) {
    int localvar = 5;
    printf(MSG, __func__, &localvar);
    bar_is_now_closed();
}

static void foo(void) {
    int localvar = 5;
    printf(MSG, __func__, &localvar);
    bar();
}

int main(int argc, char **argv) {
    int localvar = 5;
    printf(MSG, __func__, &localvar);
    foo();
    exit(EXIT_SUCCESS);
}
Команди для компіляції та запуску
Скомпілюйте та запустіть у фоновому режимі:
bash

Collapse

Wrap

Copy
gcc -o stacker stacker.c
./stacker &
Знайдіть PID процесу:
bash

Collapse

Wrap

Copy
ps aux | grep stacker
Перегляньте стек:
За допомогою gstack:
bash

Collapse

Wrap

Copy
gstack <PID>
Або за допомогою GDB:
bash

Collapse

Wrap

Copy
gdb -q stacker <PID>
(gdb) bt
(gdb) quit
Приклади виведення
Виведення програми:
text

Collapse

Wrap

Copy
In function                 main; &localvar = 0x7fffffffde4c
In function                  foo; &localvar = 0x7fffffffde2c
In function                  bar; &localvar = 0x7fffffffde0c
In function    bar_is_now_closed; &localvar = 0x7fffffffdddc

Now blocking on pause()...
За допомогою gstack <PID>:
text

Collapse

Wrap

Copy
#0  0x00007f359ec7ee7a in pause () from /lib/libc.so.7
#1  0x00000008080481d9 in bar_is_now_closed ()
#2  0x00000008080481b2 in bar ()
#3  0x000000080804818b in foo ()
#4  0x0000000808048164 in main ()
Пояснення
Стек показує послідовність викликів: main → foo → bar → bar_is_now_closed → pause.
Кожна функція додає свій кадр до стека, що видно за адресами локальних змінних.
Завдання 2.5: Аналіз використання лічильника команд (IP)
Опис
Це теоретичне завдання досліджує, чи можна замінити лічильник команд (Instruction Pointer, IP) вершиною стека.

Відповідь
Ні, лічильник команд (IP) не можна замінити вершиною стека. Вершина стека містить не лише адреси повернення, а й локальні змінні та інші дані, тому вона не може точно вказувати на наступну інструкцію для виконання.

Пояснення
IP завжди вказує на адресу наступної інструкції в сегменті text.
Стек, навпаки, є динамічною структурою, яка змінюється під час виконання (наприклад, при виділенні локальних змінних), що робить його ненадійним для цієї мети.
Завдання 2.6: Вимірювання часу доступу до різних сегментів пам’яті
Опис
Це завдання вимірює час доступу до різних сегментів пам’яті: data, bss, стека та купи, щоб порівняти їх продуктивність.

Вихідний код
Файл: mem_access.c

c

Collapse

Wrap

Copy
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1000000
#define ITER 100

int global_array[SIZE];
static int static_array[SIZE];

int main() {
    int local_array[SIZE];
    int *heap_array = malloc(SIZE * sizeof(int));
    clock_t start, end;

    // Ініціалізація
    for (int i = 0; i < SIZE; i++) {
        global_array[i] = static_array[i] = local_array[i] = heap_array[i] = i;
    }

    // Сегмент даних
    start = clock();
    for (int j = 0; j < ITER; j++) {
        for (int i = 0; i < SIZE; i++) global_array[i] += 1;
    }
    end = clock();
    printf("Time for data segment: %f\n", (double)(end - start) / CLOCKS_PER_SEC);

    // Сегмент BSS (але ініціалізований, тому в даних)
    start = clock();
    for (int j = 0; j < ITER; j++) {
        for (int i = 0; i < SIZE; i++) static_array[i] += 1;
    }
    end = clock();
    printf("Time for static array: %f\n", (double)(end - start) / CLOCKS_PER_SEC);

    // Стек
    start = clock();
    for (int j = 0; j < ITER; j++) {
        for (int i = 0; i < SIZE; i++) local_array[i] += 1;
    }
    end = clock();
    printf("Time for stack: %f\n", (double)(end - start) / CLOCKS_PER_SEC);

    // Купа
    start = clock();
    for (int j = 0; j < ITER; j++) {
        for (int i = 0; i < SIZE; i++) heap_array[i] += 1;
    }
    end = clock();
    printf("Time for heap: %f\n", (double)(end - start) / CLOCKS_PER_SEC);

    free(heap_array);
    return 0;
}
Команди для компіляції та запуску
bash

Collapse

Wrap

Copy
gcc -o mem_access mem_access.c
./mem_access
Приклади виведення
text

Collapse

Wrap

Copy
Time for data segment: 0.230000
Time for static array: 0.230000
Time for stack: 0.230000
Time for heap: 0.230000
Пояснення
Час доступу до всіх сегментів майже однаковий через кешування процесора.
Для точніших результатів можна використовувати інструменти з вищою роздільною здатністю, наприклад, rdtsc або бібліотеки типу clock_gettime.
