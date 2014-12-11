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
int Colony::DoTurn(const PlanetWars &pw, Colony *destination) {
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
	int total_ships = 0;
	if (sources.size() > 0 && dest != -1){
		vector<int> *nums = DecideNumShips(pw, sources, dest);
		for (size_t i = 0; i < sources.size(); i++){
			pw.IssueOrder(sources[i], dest, (*nums)[i]);
			total_ships += (*nums)[i];
			sprintf(logger->buffer, "Attack %d: Source: %d, Dest.: %d Ships: %d", (i + 1), sources[i], dest, (*nums)[i]);
			logger->log();
		}
	}else{
		sprintf(logger->buffer, "DO NOTHING");
		logger->log();
	}

	sprintf(logger->buffer, "END TURN:\n");
	logger->log();

	return total_ships;
}

vector<int>* Colony::DecideNumShips(const PlanetWars &pw, vector<int> &sources, int dest){
	vector<int>* nums = new vector<int>();
	for (size_t i = 0; i < sources.size(); i++){
		nums->push_back(pw.GetPlanet(sources[i])->NumShips() / 2);
	}
	return nums;
}

bool Colony::addPlanet(Planet *planet, const PlanetWars &pw){
	map<int, int> &planetToColony = *pw.PlanetColony();

	planets[size] = planet->PlanetID();
	planetToColony[planet->PlanetID()] = ID();
	size++;

	return true;
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

double Colony::Reward(const PlanetWars &pw, int action_t){
	return 0;
}

void Colony::SetEligable(int index, bool elig){
	eligable[index] = elig;
}
