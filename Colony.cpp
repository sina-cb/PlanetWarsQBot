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

	attackThreshold = 30;
}

Colony::~Colony() {

}

// The DoTurn function is where your code goes. The PlanetWars object contains
// the state of the game, including information about all planets and fleets
// that currently exist. Inside this function, you issue orders using the
// pw.IssueOrder() function. For example, to send 10 ships from planet 3 to
// planet 8, you would say pw.IssueOrder(3, 8, 10).
void Colony::DoTurn(const PlanetWars &pw, Colony *destination) {
	sprintf(logger->buffer, "TURN START: %d ", ID());
	logger->log();

	// (1) If we currently have a fleet in flight, just do nothing.
	if (pw.MyFleets().size() >= 2) {
		return;
	}
	// (2) Find my strongest planet.
	int source = 0;
	for (size_t i = 0; i < size; i++){
		if (pw.GetPlanet(planets[i])->Owner() == ME){
			source = planets[i];
			break;
		}
	}

	// (3) Find the weakest enemy or neutral planet.
	int dest = 0;
	for (size_t i = 0; i < destination->Size(); i++){
		if (pw.GetPlanet(destination->Planets()[i])->Owner() == ENEMY ||
				pw.GetPlanet(destination->Planets()[i])->Owner() == NEUTRAL){
			dest = destination->Planets()[i];
			break;
		}
		sprintf(logger->buffer, "Owner: %d Size: %d", pw.GetPlanet(planets[i])->Owner(), size);
		logger->log();
	}

	// (4) Send half the ships from my strongest planet to the weakest
	// planet that I do not own.
	if (source >= 0 && dest >= 0) {
		int num_ships = pw.GetPlanet(source)->NumShips() / 2;
		pw.IssueOrder(source, dest, num_ships);
		sprintf(logger->buffer, "Source: %d, Dest.: %d Ships: %d", source, dest, num_ships);
		logger->log();
	}

	sprintf(logger->buffer, "TURN END: %d ", ID());
	logger->log();
}

void Colony::UpdateColony(const PlanetWars &pw){
	strongness = 0;
	for (size_t i = 0; i < size; i++){
		const Planet *planet = pw.GetPlanet(planets[i]);
		if (planet->Owner() == ME){
			strongness += planet->GrowthRate() * planet->NumShips();
		}else if(planet->Owner() == ENEMY){
			strongness -= planet->GrowthRate() * planet->NumShips();
		}else if(planet->Owner() == NEUTRAL){
			strongness -= planet->NumShips();
		}

		if (planet->GrowthRate() * planet->NumShips() < attackThreshold){
			eligable[i] = false;
		}else{
			eligable[i] = true;
		}
	}

	//Discretization of strongness parameter
	if (strongness < MIN_STRONGNESS){
		strongness = MIN_STRONGNESS;
	}else if (strongness >= MAX_STRONGNESS){
		strongness = MAX_STRONGNESS;
	}

	strongness = (strongness - MIN_STRONGNESS) / STEPS;

	//TODO: We haven't updated the colonyType variable, if you needed it, update it! :D
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
	return strongness;
}

bool Colony::HasFriendlyPlanet(const PlanetWars &pw){
	for (size_t i = 0; i < size; i++){
		if (pw.GetPlanet(planets[i])->Owner() == ME){
			return true;
		}
	}
	return false;
}
