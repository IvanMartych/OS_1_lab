#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024

// Функция для записи строки
void write_string(int fd, const char* str) {
    int len = 0;
    while (str[len] != '\0') len++;
    write(fd, str, len);
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

int main() {
    int pipe1[2]; // Родитель пишет, ребенок читает (stdin для ребенка)
    int pipe2[2]; // Ребенок пишет, родитель читает
    
    // создание каналов (и обработка ошибки создания пайпов) 
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        const char* error = "Error: pipe creation failed\n";
        write(STDERR_FILENO, error, 30);
        return 1;
    }
    
    // Получаем имя файла от пользователя
    write_string(STDOUT_FILENO, "Enter filename: ");
    char filename[256];
    int bytes_read = read_line(STDIN_FILENO, filename, sizeof(filename));
    if (bytes_read <= 0) {
        const char* error = "Error: reading filename failed\n";
        write(STDERR_FILENO, error, 34);
        return 1;
    }
    
    pid_t pid = fork();
    
    if (pid == -1) {
        const char* error = "Error: fork failed\n";
        write(STDERR_FILENO, error, 20);
        return 1;
    }
    
    if (pid == 0) {
        // Дочерний процесс
        
        // Закрываем неиспользуемые концы каналов
        close(pipe1[1]); // Закрываем запись в pipe1
        close(pipe2[0]); // Закрываем чтение из pipe2
        
        // Перенаправляем stdin на pipe1
        dup2(pipe1[0], STDIN_FILENO);
        close(pipe1[0]);
        
        // Выполняем дочернюю программу
        char* args[] = {"./child", filename, NULL};
        execvp(args[0], args);
        
        // Если exec не удался
        const char* error = "Error: exec failed\n";
        write(STDERR_FILENO, error, 20);
        exit(1);
    } else {
        // Родительский процесс
        
        // Закрываем неиспользуемые концы каналов
        close(pipe1[0]); // Закрываем чтение из pipe1
        close(pipe2[1]); // Закрываем запись в pipe2
        
        write_string(STDOUT_FILENO, "Enter numbers separated by spaces (type 'exit' to quit):\n");
        
        char buffer[BUFFER_SIZE];
        
        while (1) {
            // Читаем ввод пользователя
            write_string(STDOUT_FILENO, "> ");
            bytes_read = read_line(STDIN_FILENO, buffer, BUFFER_SIZE - 1);
            if (bytes_read <= 0) break;
            
            // Проверяем команду выхода
            int is_exit = 0;
            if (bytes_read >= 4) {
                if (buffer[0] == 'e' && buffer[1] == 'x' && 
                    buffer[2] == 'i' && buffer[3] == 't') {
                    is_exit = 1;
                }
            }
            
            if (is_exit) {
                // Отправляем команду выхода ребенку
                write(pipe1[1], "exit\n", 5);
                break;
            }
            
            // Добавляем перевод строки для ребенка
            buffer[bytes_read] = '\n';
            buffer[bytes_read + 1] = '\0';
            
            // Отправляем ребенку через pipe1
            int bytes_written = write(pipe1[1], buffer, bytes_read + 1);
            if (bytes_written == -1) {
                const char* error = "Error: writing to pipe failed\n";
                write(STDERR_FILENO, error, 33);
                break;
            }
        }
        
        // Очистка
        close(pipe1[1]);
        close(pipe2[0]);
        
        // Ждем завершения дочернего процесса
        wait(NULL);
        write_string(STDOUT_FILENO, "Child process finished. Results written to file.\n");
    }
    
    return 0;
}