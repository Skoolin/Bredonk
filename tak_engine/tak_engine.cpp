// tak_engine.cpp : Defines the entry point for the application.
//

#include "tak_engine.h"

int main()
{
	std::cout << "Hello CMake." << std::endl;

	TakBoard board = TakBoard();

	uint64_t empty_bitmap = board.get_bordered_bitmap(Piece::NONE);

	std::cout << std::bitset<64>(empty_bitmap) << std::endl;

	return 0;
}
