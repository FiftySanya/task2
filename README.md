# Дослідження сегментів пам'яті та архітектурних особливостей у FreeBSD

## Зміст
1. [Завдання 1: Визначення моменту переповнення time_t](#завдання-1-визначення-моменту-переповнення-time_t)  
2. [Завдання 2: Дослідження сегментів виконуваного файлу](#завдання-2-дослідження-сегментів-виконуваного-файлу)  
3. [Завдання 3: Визначення розташування стека та сегментів](#завдання-3-визначення-розташування-стека-та-сегментів)  
4. [Завдання 4: Дослідження стека процесу](#завдання-4-дослідження-стека-процесу)  
5. [Завдання 5: Можливість обійтися без лічильника команд (IP)](#завдання-5-можливість-обійтися-без-лічильника-команд-ip)  
6. [Завдання 6: Вимірювання часу доступу до різних сегментів пам'яті](#завдання-6-вимірювання-часу-доступу-до-різних-сегментів-памяті) 

---

## Завдання 1: Визначення моменту переповнення `time_t`

### Опис
Це завдання досліджує момент, коли відбудеться переповнення типу `time_t` на 32- та 64-бітних системах. Тип `time_t` використовується для представлення часу в секундах і є знаковим цілим числом. Його розмір (32 або 64 біти) залежить від архітектури системи, що впливає на дату переповнення.

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

### Аналіз сегментів виконуваного файлу для програми дослідження максимальних значень time_t
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
  ```
  size task2
    text   data   bss    dec     hex   filename
    1079    552    16   1647   0x66f   task2
  ```

### 2. Додавання неініціалізованого глобального масиву з 1000 int
- ```bash
  ls -l task2
  -rwxr-xr-x  1 root wheel 8240 Mar 12 01:18 task2
  ```
  ```text
  size task2
    text   data    bss    dec      hex   filename
    1079    552   4032   5663   0x161f   task2
  ```
- Сегмент bss збільшився на 1000 × 4 = 4000 байт + вирівнювання пам'яті: На 64-бітних системах часто застосовується вирівнювання даних для оптимізації доступу. Компілятор міг вирівняти масив до 16-байтної межі, що додало б 16 байт (з 4000 до 4016).

### 3. Ініціалізація глобального масиву
- ```bash
  ls -l task2
    -rwxr-xr-x  1 root wheel 12280 Mar 12 01:34 task2
  ```
  ```text
  size task2
    text   data   bss    dec      hex   filename
    1079   4568    16   5663   0x161f   task2
  ```
- Масив перемістився з bss до data, збільшивши data на 4016 байт, а bss повернувся до початкового розміру.

### 4. Додавання локальних масивів
* Перевірка розмірів сегментів:
```bash
size hello
```
```text
 text    data     bss     dec     hex filename
 2288     544      16    2848     b20 hello
```

### 5. Компіляція з налагодженням та оптимізацією
* З прапором `-g`:
```bash
size hello
```
```text
 text    data     bss     dec     hex filename
 2288     544      16    2848     b20 hello
```
* З прапором `-O3`:
```bash
size hello
```
```text
 text    data     bss     dec     hex filename
 2272     544      16    2832     b10 hello
```

## Висновки
* Сегмент `data` містить ініціалізовані глобальні змінні і зберігається у файлі.
* Сегмент `bss` містить неініціалізовані глобальні змінні, його розмір враховується, але він не займає місця у файлі.
* Локальні масиви розміщуються в стеку і не впливають на розміри сегментів у файлі.
* Оптимізація (`-O3`) зменшує розмір сегмента `text`, а налагодження (`-g`) додає метаінформацію, але не впливає на основні сегменти.
