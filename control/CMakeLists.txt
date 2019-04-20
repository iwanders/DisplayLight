CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
project(displaylight)

add_library(image image.cpp)
add_library(pixelsniff pixelsniff.cpp)
  

SET(platform_link "")

if (WIN32)
  add_definitions(-DWIN32  -D_WIN32_WINNT=0x0A00)
  add_library(pixelsniffWin pixelsniffWin.cpp)
  target_link_libraries(pixelsniffWin pixelsniff)
  LIST(APPEND platform_link pixelsniffWin image)


  add_executable(snifftestWin snifftestWin.cpp)
  target_link_libraries(snifftestWin platform ${platform_link})
else()
  FIND_PACKAGE(X11)
  add_library(imageX11  imageX11.cpp)
  target_link_libraries(imageX11 image)

  add_library(pixelsniffX11 pixelsniffX11.cpp)
  target_link_libraries(pixelsniffX11 ${X11_LIBRARIES} imageX11)

  add_executable(snifftestX11 snifftestX11.cpp)
  target_link_libraries(snifftestX11 pixelsniffX11)
  
  LIST(APPEND platform_link pixelsniffX11 imageX11)

  add_compile_options(-Werror -Wall -Wextra -std=c++14)
endif()

FIND_PACKAGE(Boost)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

set(CMAKE_BUILD_TYPE RelWithDebInfo)

add_library(platform platform.cpp)
target_link_libraries(platform ${platform_link})

add_library(analyzer analyzer.cpp)
target_link_libraries(analyzer image lights)

add_library(lights lights.cpp)
target_link_libraries(lights  ${Boost_LIBRARIES})

add_executable(analyzer_test analyzer_test.cpp)
target_link_libraries(analyzer_test pixelsniff analyzer platform ${platform_link})


add_executable(main main.cpp)
target_link_libraries(main pixelsniff analyzer platform ${platform_link})

