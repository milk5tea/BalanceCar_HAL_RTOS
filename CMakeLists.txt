cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME GPIO_test)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)



# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    # Add user defined libraries
)

# 【关键修复】-u _printf_float 必须放在 target_link_options
# 告诉链接器强制引入浮点 printf 模块
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    -u _printf_float
)

#自定义头文件路径（Hardware + Algorithm）
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE 
    Hardware/MyI2C 
    Hardware/OLED 
    Hardware/MPU6050
    Hardware/Encoder
    Hardware/Motor
    Hardware/PWM
    Hardware/Serial
    Hardware/BlueSerial
    Hardware/Key
    Algorithm/Filter
    Algorithm/PID
    Tasks/task_main
)
#自定义源文件（Hardware + Algorithm）
target_sources(${CMAKE_PROJECT_NAME} PRIVATE 
    Hardware/MyI2C/MyI2C.c 
    Hardware/OLED/OLED.c 
    Hardware/OLED/OLED_Data.c
    Hardware/MPU6050/MPU6050.c
    Hardware/Encoder/encoder.c
    Hardware/Motor/motor.c
    Hardware/PWM/pwm.c
    Hardware/Serial/serial.c
    Hardware/BlueSerial/BlueSerial.c
    Hardware/Key/key.c
    Algorithm/Filter/filter.c
    Algorithm/PID/pid.c
    Tasks/task_main/task_main.c
)