cmake_minimum_required(VERSION 3.5)
project(hikang_cap_2)

set(OpenCV_DIR "/home/fran/opencv/build")
find_package(OpenCV REQUIRED)
find_package(Threads)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/cap.h)
add_executable(hikang_cap_2 ${SOURCE_FILES})

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(hikang_cap_2 ${OpenCV_LIBS})
target_link_libraries(hikang_cap_2 ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(hikang_cap_2 ${haikang_LIBS}
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/libHCCore.so
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/libhcnetsdk.so
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/libHCAlarm.so
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/libHCPreview.so
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/libHCGeneralCfgMgr.so
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/libPlayCtrl.so)
