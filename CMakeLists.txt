# cmake_minimum_required(VERSION 3.10)
# project(OS_1_lab)

# set(CMAKE_C_STANDARD 99)

# add_executable(main src/sum.c parent.c)

# add_executable(child src/child.c src/sum.c)

# target_include_directories(parent PRIVATE include)
# target_include_directories(child PRIVATE include)

# Минимальная требуемая версия CMake
cmake_minimum_required(VERSION 3.10)

# Название проекта
project(lab1 C)

# Установка стандарта C99
set(CMAKE_C_STANDARD 99)

# Включение всех предупреждений
add_compile_options(-Wall -Wextra)

# Исполняемый файл родительского процесса
add_executable(parent 
    parent.c
    src/sum.c
)

# Исполняемый файл дочернего процесса
add_executable(child
    src/child.c
    src/sum.c
)

# Установка путей для заголовочных файлов
target_include_directories(parent PRIVATE include)
target_include_directories(child PRIVATE include)