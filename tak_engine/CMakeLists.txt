# CMakeList.txt : CMake project for tak_engine, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (tak_engine "tak_engine.cpp" "tak_engine.h"   "tak/tak_board.h" "tak/move.h" "tak/tak_board.cpp" "params.h" "tak/move_iterator.h"  "search/search.h" "tak/piece.h" "tak/move_list.h" "tak/move_list.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET tak_engine PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
