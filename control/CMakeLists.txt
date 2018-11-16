CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
project(displaylight)

FIND_PACKAGE(Boost COMPONENTS system)
FIND_PACKAGE(X11)

set(CMAKE_BUILD_TYPE RelWithDebInfo)
add_compile_options(-Werror -Wall -Wextra -std=c++14)

add_library(image  image.cpp)

add_library(lights lights.cpp)
target_link_libraries(lights  ${Boost_LIBRARIES})

add_library(pixelsniff pixelsniff.cpp)
target_link_libraries(pixelsniff ${X11_LIBRARIES} image)

add_executable(snifftest snifftest.cpp)
target_link_libraries(snifftest pixelsniff)

add_library(analyzer analyzer.cpp)
target_link_libraries(analyzer ${X11_LIBRARIES} image lights)

add_executable(analyzer_test analyzer_test.cpp)
target_link_libraries(analyzer_test pixelsniff analyzer)

add_executable(main main.cpp)
target_link_libraries(main pixelsniff analyzer)
