/*
 * Colony.cpp
 *
 *  Created on: Nov 17, 2014
 *      Author: sina
 */

#include "Colony.h"
#include <iostream>
#include <algorithm>
using namespace std;

Colony::Colony(int id){
	this->id = id;

	char buffer[100];
	sprintf(buffer, "Colony_%d.log", id);
	logger = new Logger(buffer);

	for (int i = 0; i < COLONY_MAX_SIZE; i++){
		planets[i] = -1;
	}
	size = 0;

	srand(time(NULL));
}

Colony::~Colony() {

}

// The DoTurn function is where your code goes. The PlanetWars object contains
// the state of the game, including information about all planets and fleets
// that currently exist. Inside this function, you issue orders using the
// pw.IssueOrder() function. For example, to send 10 ships from planet 3 to
// planet 8, you would say pw.IssueOrder(3, 8, 10).
bool Colony::DoTurn(const PlanetWars &pw, Colony *destination) {
	sprintf(logger->buffer, "START TURN:");
	logger->log();

	sprintf(logger->buffer, "Source Colony: %d , Destination Colony: %d", ID(), destination->ID());
	logger->log();

	// (1) If we currently have a fleet in flight, just do nothing.
	/*if (num_my_fleets >= MAX_NUM_MY_FLEETS) {
		return;
	}*/

	// (2) Find my eligible planets to attack.
	vector<int> sources;
	for (size_t i = 0; i < size; i++){
		const Planet* planet = pw.GetPlanet(planets[i]);
		if (planet->Owner() == ME){
			if (planet->NumShips() > PLANET_ELIGIBILITY_CONSTANT)
				sources.push_back(planets[i]);
		}
	}

	// (3) Find the weakest enemy or neutral planet.
	int dest = -1;
	for (size_t i = 0; i < destination->Size(); i++){
		if (pw.GetPlanet(destination->Planets()[i])->Owner() == ENEMY ||
				pw.GetPlanet(destination->Planets()[i])->Owner() == NEUTRAL){
			dest = destination->Planets()[i];
			break;
		}
	}

	// (4) Send half the ships from my strongest planet to the weakest
	// planet that I do not own.
	if (sources.size() > 0 && dest != -1){
		for (size_t i = 0; i < sources.size(); i++){
			int num_ships = pw.GetPlanet(sources[i])->NumShips() / 2;
			pw.IssueOrder(sources[i], dest, num_ships);
			sprintf(logger->buffer, "Attack %d: Source: %d, Dest.: %d Ships: %d", (i + 1), sources[i], dest, num_ships);
			logger->log();
		}
	}

	sprintf(logger->buffer, "END TURN:\n");
	logger->log();

	return true;
}

vector<int>* Colony::DecideNumShips(const PlanetWars &pw, vector<int> &sources, int dest, Action* selected_action){
	vector<int>* nums = new vector<int>();
	for (size_t i = 0; i < sources.size(); i++){
		nums->push_back(pw.GetPlanet(sources[i])->NumShips() / 2);
	}
	return nums;
}

double Colony::Reward(const PlanetWars &pw, int action_t){
	return 0;
}

void Colony::UpdateColony(const PlanetWars &pw){
	state = 0;
	for (size_t i = 0; i < size; i++){
		const Planet *planet = pw.GetPlanet(planets[i]);
		if (planet->Owner() == ME){
			state += planet->GrowthRate() * planet->NumShips();
		}else if(planet->Owner() == ENEMY){
			state -= planet->GrowthRate() * planet->NumShips();
		}else if(planet->Owner() == NEUTRAL){
			state -= planet->NumShips();
		}

		if (planet->GrowthRate() * planet->NumShips() < PLANET_ELIGIBILITY_CONSTANT){
			eligable[i] = false;
		}else if (planet->Owner() == ME){
			eligable[i] = true;
		}else{
			eligable[i] = false;
		}
	}

	//Discretization of strongness parameter
	if (state < COLONY_STRONGNESS_MIN){
		state = COLONY_STRONGNESS_MIN;
	}else if (state >= COLONY_STRONGNESS_MAX){
		state = COLONY_STRONGNESS_MAX - 1;
	}

	state = (state - COLONY_STRONGNESS_MIN) / COLONY_STRONGNESS_STEP;

	//TODO: We haven't updated the colonyType variable, if you needed it, update it! :D
}

void Colony::UpdateNextStateColony(const PlanetWars& pw){
	strongness_next_state = 0;
	for (size_t i = 0; i < size; i++){
		const Planet *planet = pw.GetPlanetNewState(planets[i]);
		if (planet->Owner() == ME){
			strongness_next_state += planet->GrowthRate() * planet->NumShips();
		}else if(planet->Owner() == ENEMY){
			strongness_next_state -= planet->GrowthRate() * planet->NumShips();
		}else if(planet->Owner() == NEUTRAL){
			strongness_next_state -= planet->NumShips();
		}

		if (planet->GrowthRate() * planet->NumShips() < PLANET_ELIGIBILITY_CONSTANT){
			eligable[i] = false;
		}else{
			eligable[i] = true;
		}
	}

	//Discretization of strongness parameter
	if (strongness_next_state < COLONY_STRONGNESS_MIN){
		strongness_next_state = COLONY_STRONGNESS_MIN;
	}else if (strongness_next_state >= COLONY_STRONGNESS_MAX){
		strongness_next_state = COLONY_STRONGNESS_MAX - 1;
	}

	strongness_next_state = (strongness_next_state - COLONY_STRONGNESS_MIN) / COLONY_STRONGNESS_STEP;
}

bool Colony::addPlanet(Planet *planet, const PlanetWars &pw){
	map<int, int> &planetToColony = *pw.PlanetColony();

	planets[size] = planet->PlanetID();
	planetToColony[planet->PlanetID()] = ID();
	size++;

	return true;
}

bool Colony::IfPlanetHere(const PlanetWars &pw, int planetID){
	for (size_t i = 0; i < size; i++){
		if (pw.GetPlanet(planets[i])->PlanetID() == planetID){
			return true;
		}
	}
	return false;
}

bool Colony::removePlanet(Planet* planet){
	return false;
}

int Colony::ID(){
	return id;
}

int Colony::Size(){
	return size;
}

int Colony::Strongness(){
	return state;
}

int Colony::StrongnessEstimation(){
	return strongness_next_state;
}

bool Colony::HasFriendlyPlanet(const PlanetWars &pw){
	for (size_t i = 0; i < size; i++){
		if (pw.GetPlanet(planets[i])->Owner() == ME){
			return true;
		}
	}
	return false;
}

void Colony::Initialize(const PlanetWars &pw){
	//For each planet in the colony we try to attack
	for (size_t i = 0; i < COLONY_MAX_SIZE; i++){
		actions.push_back(new Action(i));
	}
	//NO-OP Action
	actions.push_back(new Action(-1));
}
