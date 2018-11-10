CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
project(displaylight)

FIND_PACKAGE(Boost COMPONENTS system)
FIND_PACKAGE(X11)

add_compile_options(-Wall -Wextra -pedantic)

set(CMAKE_BUILD_TYPE RelWithDebInfo)
#set(CMAKE_BUILD_TYPE Debug)

add_library(pixelsniff pixelsniff.cpp)
target_link_libraries(pixelsniff ${X11_LIBRARIES} )

add_library(screen_analyzer screen_analyzer.cpp)
target_link_libraries(screen_analyzer ${X11_LIBRARIES})

add_executable(main main.cpp)
target_link_libraries(main pixelsniff screen_analyzer ${Boost_LIBRARIES})

add_executable(snifftest snifftest.cpp)
target_link_libraries(snifftest pixelsniff ${Boost_LIBRARIES})

add_executable(analyzer_test analyzer_test.cpp)
target_link_libraries(analyzer_test pixelsniff screen_analyzer ${Boost_LIBRARIES})
