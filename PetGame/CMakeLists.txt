cmake_minimum_required(VERSION 3.15)

project(PetGame VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES 
     src/main.cpp
     src/Application.h
     src/Application.cpp
     src/Digipet.h
     src/DigiPet.cpp
     src/IState.h
     src/IState.cpp
     src/IdleState.h
     src/IdleState.cpp
     src/FeedingState.h
     src/FeedingState.cpp
     src/Shader.h
     src/Shader.cpp
     src/SpriteRenderer.h
     src/SpriteRenderer.cpp
     src/Texture2D.h
     src/Texture2D.cpp
)

set(IMGUI_SOURCES
     libs/imgui/imgui/imgui.h
     libs/imgui/imgui/imgui.cpp
     libs/imgui/imgui/imconfig.h
     libs/imgui/imgui/imgui_demo.cpp
     libs/imgui/imgui/imgui_draw.cpp
     libs/imgui/imgui/imgui_internal.h
     libs/imgui/imgui/imgui_tables.cpp
     libs/imgui/imgui/imgui_widgets.cpp
     libs/imgui/imgui/imstb_rectpack.h
     libs/imgui/imgui/imstb_textedit.h
     libs/imgui/imgui/imstb_truetype.h
     libs/imgui/imgui/backends/imgui_impl_opengl3.h
     libs/imgui/imgui/backends/imgui_impl_opengl3.cpp
     libs/imgui/imgui/backends/imgui_impl_glfw.h
     libs/imgui/imgui/backends/imgui_impl_glfw.cpp
)

list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/libs/glad/src/glad.c")
list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/libs/stb/stb_image.cpp")

add_executable(PetGame ${SOURCES} ${IMGUI_SOURCES})
set_target_properties(PetGame PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:PetGame>"
)
find_package(OpenGL REQUIRED)

target_include_directories(PetGame PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/glm"
)

target_include_directories(PetGame PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/glad/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/stb"
)

target_include_directories(PetGame PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui/imgui"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui/imgui/backends"
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