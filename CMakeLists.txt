cmake_minimum_required(VERSION 3.16)
project(UniRide)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# === Crow setup ===
set(ASIO_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/crow/deps/asio/asio/include")
set(FMT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/crow/deps/fmt/include")

# === Automatically include all .cpp files in the UniRide folder ===
file(GLOB SOURCES
    "${PROJECT_SOURCE_DIR}/*.cpp"
)

# === Create your executable ===
add_executable(${PROJECT_NAME} ${SOURCES})

# === Include directories ===
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/crow/include
    ${ASIO_INCLUDE_DIR}
    ${FMT_INCLUDE_DIR}
)

# === Windows-specific libraries ===
if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 mswsock)
endif()

