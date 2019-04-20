CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
project(displaylight)

add_library(image image.cpp)
add_library(pixelsniff pixelsniff.cpp)
  

if (WIN32)
  add_definitions(-DWIN32  -D_WIN32_WINNT=0x0A00)
  

else()
  FIND_PACKAGE(X11)
  add_library(imageX11  imageX11.cpp)
  target_link_libraries(imageX11 image)

  add_library(pixelsniffX11 pixelsniffX11.cpp)
  target_link_libraries(pixelsniffX11 ${X11_LIBRARIES} imageX11)
  add_compile_options(-Werror -Wall -Wextra -std=c++14)

  add_executable(snifftest snifftest.cpp)
  target_link_libraries(snifftest pixelsniff)

endif()

FIND_PACKAGE(Boost)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

set(CMAKE_BUILD_TYPE RelWithDebInfo)

add_library(platform platform.cpp)

add_library(analyzer analyzer.cpp)
target_link_libraries(analyzer image lights)

add_library(lights lights.cpp)
target_link_libraries(lights  ${Boost_LIBRARIES})

add_executable(analyzer_test analyzer_test.cpp)
target_link_libraries(analyzer_test pixelsniff analyzer)

add_executable(main main.cpp)
target_link_libraries(main pixelsniff analyzer platform)
#set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT main)
