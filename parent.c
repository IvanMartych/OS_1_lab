#include <unistd.h>   // Для системных вызовов: read, write, pipe, fork, close, STDIN_FILENO, STDOUT_FILENO
#include <sys/wait.h>
#include <stdlib.h>  
#include <fcntl.h>


// Функция для записи строки в файловый дескриптор

void write_str(int fd, const char* str) {
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    write(fd, str, len);
}



int main() {
    int pipe1[2]; // массив для создания пайпа : pipe1[0] - чтение, pipe1[1] - запись
    
    // создание pipe для межпроцессного взаимодействия
    if (pipe(pipe1) == -1) {
        write_str(STDERR_FILENO, "Error: pipe creation failed\n");
        return 1; // Завершаем программу с ошибкой
    }
    
    // запрос имени файла у пользователя для сохранения результатов
    write_str(STDOUT_FILENO, "Enter filename: ");
    char filename[256]; // буфер для хранения имени файла
    int bytes = read(STDIN_FILENO, filename, sizeof(filename) - 1);
    filename[bytes] = '\0';
    

    for (int i = 0; filename[i] != '\0'; i++) {
        if (filename[i] == '\n') {
            filename[i] = '\0';
            break;
        }
    }
    
    // создание дочернего процесса с помощью fork()
    pid_t pid = fork();
    
    if (pid == -1) {
        write_str(STDERR_FILENO, "Error: fork failed\n");
        return 1;
    }
    
    // дочерний процесс
    if (pid == 0) {
        // Дочерний процесс наследует открытые файловые дескрипторы родителя
        
        // Закрываем конец pipe для ЗАПИСИ, так как ребенок только читает
        close(pipe1[1]);
        
        // Перенаправляем стандартный ввод (stdin) дочернего процесса на чтение из pipe
        // Теперь все, что родитель пишет в pipe, ребенок будет читать из stdin
        dup2(pipe1[0], STDIN_FILENO);
        
        // Закрываем оригинальный дескриптор чтения из pipe
        close(pipe1[0]);
        
        // Заменяем образ процесса на программу child
        // execl загружает и выполняет программу ./child с аргументом filename
        execl("./child", "child", filename, NULL);
        
        // если программа child.c не выполнилась
        write_str(STDERR_FILENO, "Error: exec failed\n");
        exit(1); 
    } 
    // дочерний процесс
    else {
        
        
        // Закрываем конец pipe для ЧТЕНИЯ, так как родитель только пишет
        close(pipe1[0]);
        
        // Вывод инструкций для пользователя
        write_str(STDOUT_FILENO, "Enter numbers:\n");
        
        char buffer[1024]; // Буфер для хранения ввода пользователя
        
        // Основной цикл взаимодействия с пользователем
        while (1) {
            // Вывод приглашения для ввода
            write_str(STDOUT_FILENO, "> ");
            
            // Чтение ввода пользователя с клавиатуры
            bytes = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
            if (bytes <= 0) {
                break;
            }; 
            
            buffer[bytes] = '\0'; // Завершаем строку нулевым символом
            
            // Отправляем введенные данные дочернему процессу через pipe
            write(pipe1[1], buffer, bytes);
            
        }
        
        // Закрываем конец pipe для записи - это сигнал дочернему процессу о завершении
        close(pipe1[1]);
        
        // Ожидаем завершения дочернего процесса
        // wait приостанавливает выполнение родителя пока ребенок не завершится
        wait(NULL);
        
        // Сообщение об успешном завершении
        write_str(STDOUT_FILENO, "Program finished\n");
    }
    
    return 0; // Успешное завершение программы
}