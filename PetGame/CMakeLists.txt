cmake_minimum_required(VERSION 3.15)

project(PetGame VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB SOURCES "src/*.cpp" "src/*.h")

list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/libs/glad/src/glad.c")
list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/libs/stb/stb_image.cpp")

add_executable(PetGame ${SOURCES})
set_target_properties(PetGame PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:PetGame>"
)
find_package(OpenGL REQUIRED)

target_include_directories(PetGame PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/glm"
)

target_include_directories(PetGame PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/glad/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/stb" # Para encontrar stb/stb_image.h
)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/libs/glfw-3.4")
target_link_libraries(PetGame PUBLIC 
    OpenGL::GL 
    glfw      
)

add_custom_command(TARGET PetGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders" "$<TARGET_FILE_DIR:PetGame>/shaders"
    COMMENT "Copiando shaders para o diretório de saída"
)
add_custom_command(TARGET PetGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:PetGame>/assets"
    COMMENT "Copiando assets para o diretório de saída"
)