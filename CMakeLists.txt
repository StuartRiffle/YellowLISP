cmake_minimum_required( VERSION 3.8 )
project( YellowLISP )

include_directories( Source )

FILE(GLOB SOURCES Source/*.cpp)
add_executable( YellowLISP ${SOURCES} ) 


