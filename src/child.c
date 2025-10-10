#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define MAX_NUMBERS 100

// Функция для записи строки
void write_string(int fd, const char* str) {
    int len = 0;
    while (str[len] != '\0') len++;
    write(fd, str, len);
}

// Функция для преобразования float в строку
void float_to_string(float value, char* buffer) {
    int len = 0;
    
    // Обработка отрицательных чисел
    if (value < 0) {
        buffer[len++] = '-';
        value = -value;
    }
    
    // Целая часть
    int int_part = (int)value;
    float frac_part = value - int_part;
    
    // Преобразование целой части
    char int_buffer[32];
    int int_len = 0;
    
    if (int_part == 0) {
        int_buffer[int_len++] = '0';
    } else {
        int temp = int_part;
        while (temp > 0) {
            int_buffer[int_len++] = '0' + (temp % 10);
            temp /= 10;
        }
        // Реверс целой части
        for (int i = int_len - 1; i >= 0; i--) {
            buffer[len++] = int_buffer[i];
        }
    }
    
    // Дробная часть
    if (frac_part > 0.00001f) {
        buffer[len++] = '.';
        // Преобразуем до 6 знаков после запятой
        for (int i = 0; i < 6; i++) {
            frac_part *= 10;
            int digit = (int)frac_part;
            buffer[len++] = '0' + digit;
            frac_part -= digit;
            if (frac_part < 0.00001f) break;
        }
    }
    
    buffer[len] = '\0';
}

// Функция для записи float
void write_float(int fd, float value) {
    char buffer[32];
    float_to_string(value, buffer);
    write_string(fd, buffer);
}

// Функция для чтения строки
int read_line(int fd, char* buffer, int max_len) {
    int total_read = 0;
    char ch;
    
    while (total_read < max_len - 1) {
        int n = read(fd, &ch, 1);
        if (n <= 0 || ch == '\n') {
            break;
        }
        buffer[total_read++] = ch;
    }
    
    buffer[total_read] = '\0';
    return total_read;
}

// Функция для разбиения строки на токены
char* my_strtok(char* str, const char* delim, char** saveptr) {
    if (str != NULL) {
        *saveptr = str;
    }
    
    if (*saveptr == NULL || **saveptr == '\0') {
        return NULL;
    }
    
    // Пропускаем разделители в начале
    while (**saveptr != '\0') {
        int is_delim = 0;
        for (const char* d = delim; *d != '\0'; d++) {
            if (**saveptr == *d) {
                is_delim = 1;
                break;
            }
        }
        if (!is_delim) break;
        (*saveptr)++;
    }
    
    if (**saveptr == '\0') {
        return NULL;
    }
    
    char* token_start = *saveptr;
    
    // Ищем конец токена
    while (**saveptr != '\0') {
        int is_delim = 0;
        for (const char* d = delim; *d != '\0'; d++) {
            if (**saveptr == *d) {
                is_delim = 1;
                break;
            }
        }
        if (is_delim) {
            **saveptr = '\0';
            (*saveptr)++;
            break;
        }
        (*saveptr)++;
    }
    
    return token_start;
}

// ИСПРАВЛЕННАЯ функция для преобразования строки в float
float string_to_float(const char* str) {
    float result = 0.0f;
    float sign = 1.0f;
    int has_dot = 0;
    float fraction = 1.0f;
    
    // Пропускаем начальные пробелы
    while (*str == ' ' || *str == '\t') str++;
    
    // Обработка знака
    if (*str == '-') {
        sign = -1.0f;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    // Преобразование числа
    while (*str != '\0') {
        if (*str == '.') {
            has_dot = 1;
            fraction = 0.1f;  // Начинаем дробную часть
            str++;
            continue;
        }
        
        if (*str >= '0' && *str <= '9') {
            if (has_dot) {
                // Дробная часть
                result += (*str - '0') * fraction;
                fraction *= 0.1f;  // Уменьшаем множитель для следующей цифры
            } else {
                // Целая часть
                result = result * 10.0f + (*str - '0');
            }
        } else {
            // Если встретили не цифру, выходим
            break;
        }
        str++;
    }
    
    return result * sign;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        const char* error = "Error: filename argument required\n";
        write(STDERR_FILENO, error, 33);
        return 1;
    }
    
    // Открываем файл для вывода
    int output_fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (output_fd == -1) {
        const char* error = "Error: cannot open output file\n";
        write(STDERR_FILENO, error, 35);
        return 1;
    }
    
    char buffer[BUFFER_SIZE];
    float numbers[MAX_NUMBERS];
    
    while (1) {
        // Читаем из stdin (pipe1 от родителя)
        int bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1);
        if (bytes_read <= 0) break;
        
        buffer[bytes_read] = '\0';
        
        // Проверяем команду выхода
        int is_exit = 0;
        for (int i = 0; i < bytes_read - 3; i++) {
            if (buffer[i] == 'e' && buffer[i+1] == 'x' && 
                buffer[i+2] == 'i' && buffer[i+3] == 't') {
                is_exit = 1;
                break;
            }
        }
        
        if (is_exit) {
            break;
        }
        
        // Парсим числа
        int count = 0;
        char* saveptr = NULL;
        char* token = my_strtok(buffer, " \t\n", &saveptr);
        
        while (token != NULL && count < MAX_NUMBERS) {
            numbers[count] = string_to_float(token);
            count++;
            token = my_strtok(NULL, " \t\n", &saveptr);
        }
        
        if (count > 0) {
            // Вычисляем сумму
            float result = 0.0f;
            for (int i = 0; i < count; i++) {
                result += numbers[i];
            }
            
            // Записываем результат в файл
            write_string(output_fd, "Numbers: ");
            for (int i = 0; i < count; i++) {
                write_float(output_fd, numbers[i]);
                if (i < count - 1) write_string(output_fd, " ");
            }
            write_string(output_fd, "\nSum: ");
            write_float(output_fd, result);
            write_string(output_fd, "\n\n");
        }
    }
    
    close(output_fd);
    return 0;
}