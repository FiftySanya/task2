# Дослідження сегментів пам'яті та архітектурних особливостей у FreeBSD

## Зміст
[Завдання 1: Визначення моменту переповнення time_t](#завдання-1-визначення-моменту-переповнення-time_t)  
[Завдання 2: Дослідження сегментів виконуваного файлу](#завдання-2-дослідження-сегментів-виконуваного-файлу)  
[Завдання 3: Визначення розташування стека та сегментів](#завдання-3-визначення-розташування-стека-та-сегментів)  
[Завдання 4: Дослідження стека процесу](#завдання-4-дослідження-стека-процесу)  
[Завдання 5: Можливість обійтися без лічильника команд (IP)](#завдання-5-можливість-обійтися-без-лічильника-команд-ip)  
[Завдання 6: Вимірювання часу доступу до різних сегментів пам'яті](#завдання-6-вимірювання-часу-доступу-до-різних-сегментів-памяті) 

---

## Завдання 1: Визначення моменту переповнення time_t

### Опис
Це завдання досліджує момент, коли відбудеться переповнення типу time_t на 32- та 64-бітних системах. Тип time_t використовується для представлення часу в секундах і є знаковим цілим числом. Його розмір (32 або 64 біти) залежить від архітектури системи, що впливає на дату переповнення.

### Команди для компіляції
- Для 32-бітної системи (вказуємо прапор `-m32` для примусової 32-бітної компіляції):
  ```bash
  gcc -m32 -Wall task1.c -o task1
  ```
- Для 64-бітної системи (вказуємо прапор `-m64`, хоча це стандартна поведінка):
  ```bash
  gcc -m64 -Wall task1.c -o task1
  ```

### Приклади виведення
- На 32-бітній системі:
  ```text
  Size of time_t: 4 bytes
  Maximum representable time: Tue Jan 19 05:14:07 2038
  ```
- На 64-бітній системі:
  ```text
  Size of time_t: 8 bytes
  Time conversion failed, value might be too large
  ```

### Аналіз сегментів виконуваного файлу
- 32-бітна компіляція
  ```
  size task1
    text   data   bss    dec     hex   filename
    1330    288     8   1626   0x65a   task1
  ```
- 64-бітна компіляція
  ```
  size task1
    text   data   bss    dec     hex   filename
    1512    576    16   2104   0x838   task1
  ```
  
  | Сегмент | 32-бітна версія | 64-бітна версія | Різниця |
  |---------|-----------------|-----------------|---------|
  | text    | 1330 байт       | 1512 байт       | +182    |
  | data    | 288 байт        | 576 байт        | +288    |
  | bss     | 8 байт          | 16 байт         | +8      |
  | Всього  | 1626 байт       | 2104 байт       | +478    |

### Сегмент text
- Призначення: містить інструкції (машинний код) програми.
- Причини різниці: в 64-бітній версії сегмент text більший на 182 байти через:
  - Використання 64-бітних інструкцій, які мають більший розмір опкодів
  - Оптимізацію та вирівнювання коду під 64-бітну архітектуру
  - Різні інструкції для роботи з 64-бітними регістрами та адресами

### Сегмент data
- Призначення: містить ініціалізовані глобальні та статичні змінні.
- Причини різниці: в 64-бітній версії сегмент data більший на 288 байт через:
  - Збільшені розміри внутрішніх структур даних стандартної бібліотеки
  - Збільшений розмір вказівників з 4 до 8 байт
  - Додаткове вирівнювання даних для 64-бітної архітектури

### Сегмент bss
- Призначення: містить неініціалізовані глобальні та статичні змінні.
- Причини різниці: в 64-бітній версії сегмент bss більший на 8 байт через:
  - Хоча в програмі явно не визначено неініціалізованих глобальних змінних, 
    компілятор і стандартна бібліотека використовують цей сегмент для власних потреб
  - Розмір адрес вказівників збільшився вдвічі (з 4 до 8 байт)
  - Різні вимоги до вирівнювання даних

### Висновки
- На 32-бітних системах переповнення time_t відбудеться 19 січня 2038 року через обмеження 32-бітного знакового числа (максимальне значення: 2,147,483,647 секунд від 1 січня 1970 року).
- На 64-бітних системах переповнення настане через мільярди років.
- Збільшення загального розміру: 64-бітна версія програми більша на 478 байт порівняно з 32-бітною версією.
- Найбільше зростання в сегменті data: відносне збільшення розміру сегменту data (на 100%) найбільше серед усіх сегментів, що пов'язано зі збільшенням розміру вказівників та внутрішніх структур даних.

## Завдання 2: Дослідження сегментів виконуваного файлу

### Опис
Завдання присвячене аналізу сегментів виконуваного файлу (`text`, `data`, `bss`) та їхньої поведінки при додаванні глобальних і локальних масивів, ініціалізації змінних, а також при компіляції з різними прапорами (налагодження `-g` та оптимізація `-O3`). Використовується утиліта `size` для перевірки розмірів сегментів.

### Команди для компіляції
- Базова компіляція:
  ```bash
  gcc -Wall task2.c -o task2
  ```
- З увімкненим налагодженням:
  ```bash
  gcc -Wall -g task2.c -o task2
  ```
- З оптимізацією:
  ```bash
  gcc -Wall -O3 task2.c -o task2
  ```

### 1. Базова програма "Hello, world!"
- ```bash
  ls -l task2
    -rwxr-xr-x  1 root wheel 8200 Mar 12 01:00 task2
  ```
  ```text
  size task2
    text   data   bss    dec     hex   filename
    1079    552    16   1647   0x66f   task2
  ```

### 2. Додавання неініціалізованого глобального масиву з 1000 int
- ```text
  size task2
    text   data    bss    dec      hex   filename
    1079    552   4032   5663   0x161f   task2
  ```
- Сегмент bss збільшився на 1000 × 4 = 4000 байт + вирівнювання пам'яті: На 64-бітних системах часто застосовується вирівнювання даних для оптимізації доступу. Компілятор міг вирівняти масив до 16-байтної межі, що додало б 16 байт (з 4000 до 4016).

### 3. Ініціалізація глобального масиву
- ```text
  size task2
    text   data   bss    dec      hex   filename
    1079   4568    16   5663   0x161f   task2
  ```
- Масив перемістився з bss до data, збільшивши data на 4016 байт, а bss повернувся до початкового розміру.

### 4. Додавання локальних масивів
- ```bash
  ls -l task2
    -rwxr-xr-x  1 root wheel 12312 Mar 12 01:44 task2
  ```
  ```text
    size task2
      text   data   bss    dec      hex   filename
      1119   4568    16   5703   0x1647   task2
  ```
- Локальні масиви зберігаються на стеку під час виконання, тому розміри сегментів у файлі змінюються лише для text.

### 5. Компіляція з налагодженням та оптимізацією
- З прапором `-g`:
  ```bash
  ls -l task2
    -rwxr-xr-x  1 root wheel 13096 Mar 12 01:48 task2
  ```
  ```text
  size task2
    text   data   bss    dec      hex   filename
    1119   4568    16   5703   0x1647   task2
  ```
- Розмір файлу збільшився через налагоджувальну інформацію, але сегменти залишилися незмінними.

- З прапором `-O3`:
  ```bash
  ls -l task2
    -rwxr-xr-x  1 root wheel 12248 Mar 12 01:53 task2
  ```
  ```text
  size task2
    text   data   bss    dec      hex   filename
    1074   4568    16   5658   0x161a   task2
  ```
- Сегмент text зменшився завдяки оптимізації коду.

### Висновки
- Сегмент `data` містить ініціалізовані глобальні змінні.
- Сегмент `bss` містить неініціалізовані глобальні та статичні змінні, його розмір враховується, але він не займає місця у файлі.
- Локальні масиви розміщуються в стеку і не впливають на розміри сегментів у файлі.
- Оптимізація (`-O3`) зменшує розмір сегмента `text`, а налагодження (`-g`) додає метаінформацію, але не впливає на основні сегменти.

## Завдання 3: Визначення розташування стека та сегментів

### Опис
Це завдання досліджує адреси стека, сегментів text, data, bss та купи у віртуальному адресному просторі процесу. Також аналізується зміна адреси вершини стека при виклику функцій із великими локальними масивами.

### 1. Розташування стека
- ```bash
  ./task3_1
    The stack top is on 0x82116d46c
  ```

### 2. Розташування сегментів
- ```bash
  ./task3_2
    BSS Segment:
      - Uninitialized global variable: 0x401cd4
      - Uninitialized static variable: 0x401cd8
      - Uninitialized static local variable: 0x401cdc
    
    Data Segment:
      - Initialized global variable: 0x401cb8
      - Initialized static variable: 0x401cbc
      - Initialized static local variable: 0x401cc0
    
    Stack Segment:
      - Local variable: 0x8213e5bf4
    
    Heap Segment:
      - Dynamically allocated variable: 0xc0167608008
    
    Text Segment (Code):
      - Address of main function: 0x400663
      - Address of dummy_function: 0x400652
  ```

### 3. Зміна розміру стека
- ```bash
  ./task3_3
    Depth: 5, Address of arr: 0x820418100
    Depth: 4, Address of arr: 0x820417140
    Depth: 3, Address of arr: 0x820416180
    Depth: 2, Address of arr: 0x8204151c0
    Depth: 1, Address of arr: 0x820414200
    Depth: 0, Address of arr: 0x820413240
  ```
- Бачимо, що стек зростає вниз, тому адреса зменшується порівняно з початковою

### Висновки
- Сегмент text розташовується на нижчих адресах.
- Сегменти data та bss розміщуються після text (спочатку data, потім bss).
- Стек починається з вищих адрес і зростає вниз (зменшення адреси при додаванні локальних змінних).
- Купа (heap) — після stack.

## Завдання 4: Дослідження стека процесу

### Опис
Завдання полягає в аналізі стека процесу за допомогою утиліт gstack або GDB. Використовується тестова програма з вкладеними викликами функцій, яка блокується на системному виклику pause() для зручності аналізу.

### Команди для компіляції
- ```bash
  gcc -Wall -g task4.c -o task4
  ```

### 1. Запуск програми
- ```bash
  ./task4
    In function                 main; &localvar = 0x820e04a0c
    In function                  foo; &localvar = 0x820e049dc
    In function                  bar; &localvar = 0x820e049bc
    In function    bar_is_now_closed; &localvar = 0x820e0499c
    Now blocking on pause()...
  ```

### 2. Аналіз стека за допомогою GDB
- ```bash
  gdb -p 3060
  (gdb) bt
    #0  0x0000000822b4d77a in _sigsuspend () from /lib/libc.so.7
    #1  0x0000000822ac3c35 in pause () from /lib/libc.so.7
    #2  0x000000000040068b in bar_is_now_closed () at task4.c:11
    #3  0x00000000004006bd in bar () at task4.c:17
    #4  0x00000000004006ef in foo () at task4.c:23
    #5  0x0000000000400728 in main (argc=1, argv=0x820c259f0) at task4.c:29
  ```

### Висновки
- GDB дозволяє дослідити, на якому саме системному виклику заблокувався процес (наприклад, pause()), що корисно для діагностики зависань.
- gstack утиліти не існує на FreeBSD

## Завдання 5: Можливість обійтися без лічильника команд (IP)  

### Що таке лічильник команд?
Лічильник команд (Instruction Pointer, IP) — спеціальний регістр процесора, який зберігає адресу наступної інструкції для виконання.

### Як працює виклик функцій?
При виклику функції відбуваються такі процеси:
  1. Поточне значення IP (наступний рядок коду) зберігається на стеку
  2. IP отримує нове значення — адресу викликаної функції
  3. Після завершення функції, адреса повернення (рядок коду після виклику функції) вилучається зі стеку та знову записується в IP

### Висновок
Без IP процесор не зміг би автоматично визначати, яку інструкцію виконувати наступною після того, як функція закінчилась, бо стек би не зміг зберігати цю інструкцію.

## Завдання 6: Вимірювання часу доступу до різних сегментів пам'яті

### Опис
Метою даного завдання є вимірювання та порівняння швидкості доступу до різних сегментів пам'яті процесу в операційній системі FreeBSD. Програма тестує наступні сегменти:

- DATA сегмент - містить ініціалізовані глобальні та статичні змінні.
- BSS сегмент - містить неініціалізовані глобальні та статичні змінні (автоматично заповнюються нулями).
- HEAP сегмент - динамічно виділена пам'ять за допомогою malloc.
- STACK сегмент - локальні змінні функцій, адреси повернення та аргументи функцій.

### Команда для компіляції
- ```bash
  gcc -Wall -O0 task6.c -o task6
  ```

Використання опції `-O0` гарантує відключення оптимізації компілятора, що може вплинути на результати вимірювання.

### Результати виконання
- ```
  Measuring access time to main memory segments in FreeBSD
  --------------------------------------------------------
  Number of iterations: 1000000
  
  DATA segment (initialized data): 0.005 microseconds per access
  BSS segment (uninitialized data): 0.006 microseconds per access
  HEAP segment (dynamic memory): 0.004 microseconds per access
  STACK segment (stack): 0.004 microseconds per access
  ```

### Висновки
На основі отриманих результатів можна зробити такі висновки:
- Швидкість доступу є дуже близькою для всіх сегментів, що вказує на ефективність системи віртуальної пам'яті FreeBSD та кешування процесором.

- BSS сегмент показав найповільніший доступ (0.006 мкс), що може бути пов'язано з тим, що він заповнюється нулями "на вимогу" (lazy initialization) при першому доступі.

- HEAP та STACK сегменти показали найкращі результати (0.004 мкс), що вказує на ефективну оптимізацію цих часто використовуваних областей пам'яті.

- DATA сегмент має середній показник (0.005 мкс), що відповідає його характеристикам як попередньо ініціалізованої пам'яті.

- Надзвичайно малий час доступу (порядку мікросекунд) для всіх сегментів свідчить про:
   - Високу ефективність кешування пам'яті процесором (L1/L2/L3 кеші)
   - Хорошу локальність доступу в тестовому алгоритмі
   - Ефективну реалізацію менеджменту пам'яті в FreeBSD

- Невеликі відмінності в часі доступу між різними сегментами (в межах 0.002 мкс) можуть бути пов'язані з:
   - Різними патернами розміщення в фізичній пам'яті
   - Особливостями кешування різних ділянок пам'яті
   - Розміщенням сторінок (page alignment) та фрагментацією

Ці результати підтверджують, що в сучасних системах з багаторівневою ієрархією пам'яті та кешування, архітектурні відмінності між сегментами пам'яті мають мінімальний вплив на швидкість доступу для випадків, коли дані вже знаходяться в кеші процесора.
