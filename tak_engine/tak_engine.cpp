// tak_engine.cpp : Defines the entry point for the application.
//


#include "tak_engine.h"



TakEngine::TakEngine(const std::string nnue_path)
	: searcher(nullptr)
{
	Eval::init(nnue_path.c_str());
}

void TakEngine::handle_command_tei() {
	std::cout << "id name Bredonk author Skolin version " << version << std::endl
		<< "size 6 halfkomi 0" << std::endl
		<< "teiok" << std::endl
		;
}

void TakEngine::handle_command_isready() {
	std::cout << "readyok" << std::endl;
}

void TakEngine::handle_command_teinewgame(std::stringstream &split) {
	std::string size;
	std::string komi;

	// TODO handle args

	board = TakBoard();
}

void TakEngine::handle_command_position(std::stringstream& split) {
	std::string token;
	std::getline(split, token, ' ');

	if (token == "tps") {
		std::string tps;
		std::getline(split, tps, ' ');
		// TODO implement tps parsing
		throw std::runtime_error("tps not implemented!");
	}
	else if (token == "startpos")
		board = TakBoard();
	else
		// invalid command
		throw std::runtime_error(std::string("invalid command: position ") + token);

	// add moves	
	while (std::getline(split, token, ' ')) {
		board.make_move(move_t::from_ptn(token));
	}
}

void TakEngine::handle_command_go(std::stringstream& split) {
	std::string token;

	uint64_t wtime = 0;
	uint64_t btime = 0;
	uint64_t winc = 0;
	uint64_t binc = 0;
	uint64_t movestogo = 0;

	uint64_t depth = 60;
	uint64_t nodes = 0;
	uint64_t movetime = 0;
	bool forced = false;
	bool infinite = false;

	while (std::getline(split, token, ' ')) {
		if (token == "wtime") {
			std::getline(split, token, ' ');
			wtime = std::stoi(token);
		}
		else if (token == "btime") {
			std::getline(split, token, ' ');
			btime = std::stoi(token);
		}
		else if (token == "wtime") {
			std::getline(split, token, ' ');
			winc = std::stoi(token);
		}
		else if (token == "binc") {
			std::getline(split, token, ' ');
			binc = std::stoi(token);
		}
		else if (token == "movestogo") {
			std::getline(split, token, ' ');
			movestogo = std::stoi(token);
		}
		else if (token == "depth") {
			std::getline(split, token, ' ');
			depth = std::stoi(token);
		}
		else if (token == "nodes") {
			std::getline(split, token, ' ');
			nodes = std::stoi(token);
		}
		else if (token == "movetime") {
			std::getline(split, token, ' ');
			movetime = std::stoi(token);
		}
		else if (token == "forced") {
			forced = true;
		}
		else if (token == "infinite") {
			infinite = true;
		}
	}

	// TODO use the parameters, we are already unpacking them!!!
	// TODO this should be seperate threat, to be able to still parse stop command!

	searcher = new Searcher(false);
	searcher->search(board, depth);
}

void TakEngine::handle_command_stop() {
	searcher->stop();
}

void TakEngine::tei_loop() {
	while (true) {
		std::string line{};
		std::getline(std::cin, line);
		
		std::stringstream split(line);
		std::string token;

		std::getline(split, token, ' ');

		if (token == "quit") return;
		else if (token == "tei") handle_command_tei();
		else if (token == "isready") handle_command_isready();
		else if (token == "teinewgame") handle_command_teinewgame(split);
		else if (token == "position") handle_command_position(split);
		else if (token == "go") handle_command_go(split);
		else if (token == "stop") handle_command_stop();
	}
}

int main(int argc, char* argv[]) {
	struct optparse_long longopts[] = {
		{"help", 'h', OPTPARSE_NONE},
		{"nnue", 'm', OPTPARSE_REQUIRED},
		{"version", 'V', OPTPARSE_NONE},
		{0}
	};

	int option;
	struct optparse options;
	std::string nnue_path = "nnue.bin";

	optparse_init(&options, argv);
	while (option = optparse_long(&options, longopts, NULL) != -1) {
		switch (option) {
		case 'm':
			if (options.optarg) {
				nnue_path = options.optarg;
				continue;
			}
			std::cout << "Error: could not parse nnue file path" << std::endl << std::endl;
			[[fallthrough]];
		case 'h':
			std::cout << "Bredonk is a Tak Game Engine by Skolin." << std::endl
				<< "Usage:" << std::endl
				<< "  bredonk [--nnue=<nnue_file>]" << std::endl
				<< "  bredonk -h | --help" << std::endl
				<< "  bredonk --version" << std::endl
				<< std::endl
				<< "Options:" << std::endl
				<< "  -h, --help            Show this screen." << std::endl
				<< "  --nnue=<nnue_file>    Specify path to file that holds nnue weights [default: \"nnue.bin\"]." << std::endl
				<< "  --version             Display version number." << std::endl
				;
			return 0;
		case 'V':
			std::cout << "Bredonk Version " << version << std::endl;
			return 0;
		}
	}

	TakEngine(nnue_path).tei_loop();
}
