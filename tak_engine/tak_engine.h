// tak_engine.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#define OPTPARSE_IMPLEMENTATION

#include <iostream>
#include <bitset>
#include <chrono>

#include "search/perft.h"
#include "search/search.h"
#include "tak/eval.h"
#include "tak/magic.h"
#include "tak/tak_board.h"
#include "util/optparse.h"


const std::string version{ "0.0.1" };


class TakEngine {
public:
	TakEngine(const std::string nnue_path);
	void tei_loop();
private:
	void handle_command_tei();
	void handle_command_isready();
	void handle_command_teinewgame(std::stringstream &split);
	void handle_command_position(std::stringstream & split);
	void handle_command_go(std::stringstream & split);
	void handle_command_stop();
	TakBoard board;
	Searcher* searcher;
	bool verbose;
};
