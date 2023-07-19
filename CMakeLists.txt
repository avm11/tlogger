cmake_minimum_required(VERSION 3.16)
project(tlogger)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SRC_FILES
    src/coinbasetickersubscriber.cpp
    src/csvtickerprocessor.cpp
    tlogger.m.cpp
)

find_package(Boost REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(websocketpp REQUIRED)
find_package(glog REQUIRED)


include_directories(
    ${PROJECT_SOURCE_DIR}/src 
    ${websocketpp_INCLUDE_DIRS} 
    ${Boost_INCLUDE_DIRS}
    ${OpenSSL_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} openssl::openssl glog::glog pthread)
