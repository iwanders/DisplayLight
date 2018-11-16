CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
project(displaylight)

FIND_PACKAGE(Boost COMPONENTS system)
FIND_PACKAGE(X11)

add_compile_options(-Wall -Werror -Wextra -std=c++14)

set(CMAKE_BUILD_TYPE RelWithDebInfo)
#set(CMAKE_BUILD_TYPE Debug)

add_library(image  image.cpp)
add_library(lights lights.cpp)
target_link_libraries(lights  ${Boost_LIBRARIES})

add_library(pixelsniff pixelsniff.cpp)
target_link_libraries(pixelsniff ${X11_LIBRARIES} image)

add_library(analyzer analyzer.cpp)
target_link_libraries(analyzer ${X11_LIBRARIES} image lights)

add_executable(main main.cpp)
target_link_libraries(main pixelsniff analyzer)

add_executable(snifftest snifftest.cpp)
target_link_libraries(snifftest pixelsniff)

add_executable(analyzer_test analyzer_test.cpp)
target_link_libraries(analyzer_test pixelsniff analyzer)
