CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
project(displaylight)

FIND_PACKAGE(Boost COMPONENTS system)
FIND_PACKAGE(X11)


add_library(pixelsniff pixelsniff.cpp)
target_link_libraries(pixelsniff ${X11_LIBRARIES} )

add_executable(main main.cpp)
target_link_libraries(main pixelsniff ${Boost_LIBRARIES})


add_executable(snifftest snifftest.cpp)
target_link_libraries(snifftest pixelsniff ${Boost_LIBRARIES})
