CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
project(displaylight)

FIND_PACKAGE(Boost COMPONENTS system)


add_executable (main main.cpp)
target_link_libraries(main ${Boost_LIBRARIES} )