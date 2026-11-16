# CMake generated Testfile for 
# Source directory: /Users/leogogichaishvili/ArchChess/arch-chess/GosChess/chess
# Build directory: /Users/leogogichaishvili/ArchChess/arch-chess/GosChess/build-mac/chess
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(chess_selftest "/Users/leogogichaishvili/ArchChess/arch-chess/GosChess/build-mac/chess/chess_selftest")
set_tests_properties(chess_selftest PROPERTIES  _BACKTRACE_TRIPLES "/Users/leogogichaishvili/ArchChess/arch-chess/GosChess/chess/CMakeLists.txt;32;add_test;/Users/leogogichaishvili/ArchChess/arch-chess/GosChess/chess/CMakeLists.txt;0;")
add_test(chess_perft "/Users/leogogichaishvili/ArchChess/arch-chess/GosChess/build-mac/chess/chess_perft")
set_tests_properties(chess_perft PROPERTIES  _BACKTRACE_TRIPLES "/Users/leogogichaishvili/ArchChess/arch-chess/GosChess/chess/CMakeLists.txt;41;add_test;/Users/leogogichaishvili/ArchChess/arch-chess/GosChess/chess/CMakeLists.txt;0;")
