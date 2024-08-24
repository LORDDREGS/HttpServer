cmake_minimum_required(VERSION 3.10)

# Название проекта
project(SimpleHttpServer)

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Указание пути к toolchain файлу vcpkg
set(CMAKE_TOOLCHAIN_FILE "C:/Users/User/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

# Поиск и подключение пакетов Boost
find_package(Boost 1.85 REQUIRED COMPONENTS filesystem asio beast)

if(Boost_FOUND)
    # Подключение заголовочных файлов Boost
    include_directories(${Boost_INCLUDE_DIRS})

    # Создание исполняемого файла
    add_executable(SimpleHttpServer main.cpp)

    # Линковка с библиотеками Boost
    target_link_libraries(SimpleHttpServer Boost::filesystem Boost::asio Boost::beast)
else()
    message(FATAL_ERROR "Could not find Boost")
endif()
