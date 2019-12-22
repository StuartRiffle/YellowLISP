cmake_minimum_required( VERSION 3.8 )
project( YellowLISP LANGUAGES CXX )

include_directories( Source )
add_executable( YellowLISP Source/**/*.cpp )
target_compile_options( YellowLISP PUBLIC -std=c++11 )

