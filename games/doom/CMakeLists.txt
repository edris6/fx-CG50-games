# Doom for the Casio fx-CG50 -- fxSDK / gint build.
#
#   fxsdk build-cg          (from this directory)
#
# produces build-cg/Doom.g3a
cmake_minimum_required(VERSION 3.15)
project(Doom C)

include(GenerateG3A)
find_package(Gint 2.1 REQUIRED)

file(GLOB DOOM_SOURCES CONFIGURE_DEPENDS src/doom/*.c)
file(GLOB PLATFORM_SOURCES CONFIGURE_DEPENDS src/platform/*.c)

add_executable(doom ${DOOM_SOURCES} ${PLATFORM_SOURCES})

target_include_directories(doom PRIVATE src/doom src/platform)

# cg50_compat.h routes the few libc calls that fxlibc may lack to our own
# versions; it is force-included into every file of this target.
target_compile_options(doom PRIVATE
  -O2 -w
  -ffunction-sections -fdata-sections
  -include ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/cg50_compat.h)

target_compile_definitions(doom PRIVATE
  NORMALUNIX
  DOOM_SHOW_FPS)          # remove this line to hide the FPS counter

target_link_libraries(doom Gint::Gint)
target_link_options(doom PRIVATE -Wl,--gc-sections)

if("${FXSDK_PLATFORM_LONG}" STREQUAL fxCG50)
  generate_g3a(TARGET doom OUTPUT "Doom.g3a"
    NAME "Doom" ICONS assets-cg/icon-uns.png assets-cg/icon-sel.png)
endif()
