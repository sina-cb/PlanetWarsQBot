#include <iostream>
#include "PlanetWars.h"
#include "CentralGovernment.h"

using namespace std;

// This is just the main game loop that takes care of communicating with the
// game engine for you. You don't have to understand or change the code below.
int main(int argc, char *argv[]) {

	CentralGovernment federal;
	int turn = 0;

	std::string current_line;
	std::string map_data;

	PlanetWars pw;

	while (true) {
		int c = std::cin.get();
		current_line += (char) c;
		if (c == '\n') {
			if (current_line.length() >= 2 && current_line.substr(0, 2) == "go") {
				turn++;

				if (turn == 1) {
					pw.Initialize(map_data);

				} else {
					pw.Update(map_data);
				}
				map_data = "";

				federal.DoTurn(pw);

				pw.FinishTurn();
			} else {
				map_data += current_line;
			}
			current_line = "";
		}
	}
	return 0;
}
