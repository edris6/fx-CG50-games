# Arcade games for the Casio fx-CG50 -- fxSDK / gint build.
#
#   fxsdk build-cg          (from this directory)
#
# produces one add-in per game in build-cg/:
#   2048.g3a Tetris.g3a Snake.g3a PacMan.g3a Sokoban.g3a Minesweeper.g3a
#   Flappy.g3a Doodle.g3a
cmake_minimum_required(VERSION 3.15)
project(Arcade C)

include(GenerateG3A)
find_package(Gint 2.1 REQUIRED)

set(COMMON_SOURCES common/cgx.c common/font5x7.c)

# arcade_game(<target> <add-in name> <directory> <logic sources...>)
function(arcade_game target title dir)
  add_executable(${target} ${dir}/main.c ${ARGN} ${COMMON_SOURCES})
  target_include_directories(${target} PRIVATE common ${dir})
  target_compile_options(${target} PRIVATE -Wall -Wextra -O2)
  target_link_libraries(${target} Gint::Gint)
  if("${FXSDK_PLATFORM_LONG}" STREQUAL fxCG50)
    generate_g3a(TARGET ${target} OUTPUT "${title}.g3a" NAME "${title}"
      ICONS ${dir}/assets-cg/icon-uns.png ${dir}/assets-cg/icon-sel.png)
  endif()
endfunction()

arcade_game(g2048    "2048"        g2048    g2048/g2048.c)
arcade_game(tetris   "Tetris"      tetris   tetris/tetris.c)
arcade_game(snake    "Snake"       snake    snake/snake.c)
arcade_game(pacman   "PacMan"      pacman   pacman/pacman.c)
arcade_game(sokoban  "Sokoban"     sokoban  sokoban/sokoban.c)
arcade_game(mines    "Minesweeper" mines    mines/mines.c)
arcade_game(flappy   "Flappy"      flappy   flappy/flappy.c)
arcade_game(doodle   "Doodle"      doodle   doodle/doodle.c)
