# CMake generated Testfile for 
# Source directory: /Users/leogogichaishvili/ArchChess/arch-chess/GosChess/server
# Build directory: /Users/leogogichaishvili/ArchChess/arch-chess/GosChess/cmake-build-debug/server
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[server_selftest]=] "/Users/leogogichaishvili/ArchChess/arch-chess/GosChess/cmake-build-debug/server/server_selftest")
set_tests_properties([=[server_selftest]=] PROPERTIES  _BACKTRACE_TRIPLES "/Users/leogogichaishvili/ArchChess/arch-chess/GosChess/server/CMakeLists.txt;32;add_test;/Users/leogogichaishvili/ArchChess/arch-chess/GosChess/server/CMakeLists.txt;0;")
add_test([=[server_e2e_selftest]=] "/Users/leogogichaishvili/ArchChess/arch-chess/GosChess/cmake-build-debug/server/server_e2e_selftest")
set_tests_properties([=[server_e2e_selftest]=] PROPERTIES  _BACKTRACE_TRIPLES "/Users/leogogichaishvili/ArchChess/arch-chess/GosChess/server/CMakeLists.txt;40;add_test;/Users/leogogichaishvili/ArchChess/arch-chess/GosChess/server/CMakeLists.txt;0;")
