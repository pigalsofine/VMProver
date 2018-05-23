cmake_minimum_required(VERSION 3.5.1)
project(VM_client)

set(CMAKE_CXX_STANDARD 11)

add_executable(VM_client client.cpp process_list.cpp process_list.h definition.h Features.cpp Features.h
                getTxt.cpp getTxt.h wrap.h wrap.cpp module.cpp module.h cmp_file.cpp cmp_file.h)

TARGET_LINK_LIBRARIES(VM_client -lvmi -lcapstone -w )