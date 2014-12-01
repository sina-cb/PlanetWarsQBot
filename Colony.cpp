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
}

Colony::~Colony() {

}

// The DoTurn function is where your code goes. The PlanetWars object contains
// the state of the game, including information about all planets and fleets
// that currently exist. Inside this function, you issue orders using the
// pw.IssueOrder() function. For example, to send 10 ships from planet 3 to
// planet 8, you would say pw.IssueOrder(3, 8, 10).
void Colony::DoTurn(const PlanetWars &pw) {

	sprintf(logger->buffer, "TURN START: %d ", ID());
	logger->log();

	// (1) If we currently have a fleet in flight, just do nothing.
	if (pw.MyFleets().size() >= 2) {
		return;
	}
	// (2) Find my strongest planet.
	int source = pw.MyPlanets()[0]->PlanetID();

	// (3) Find the weakest enemy or neutral planet.
	int dest = 4;

	// (4) Send half the ships from my strongest planet to the weakest
	// planet that I do not own.
	if (source >= 0 && dest >= 0 && ID() == 1) {
		int num_ships = pw.MyPlanets()[0]->NumShips() / 2;

		//LOCK

		pw.IssueOrder(source, dest, num_ships);
		//UNLOCK

		sprintf(logger->buffer, "Source: %d, Dest.: %d ", source, dest);
			logger->log();
	}

	sprintf(logger->buffer, "TURN END: %d ", ID());
	logger->log();
}

bool Colony::addPlanet(Planet *planet, const PlanetWars &pw){
	map<int, int> &planetToColony = *pw.PlanetColony();

	planets[size] = planet->PlanetID();
	planetToColony[planet->PlanetID()] = ID();
	size++;

	return true;
}

bool Colony::removePlanet(Planet* planet){
	bool result = false;
	sprintf(logger->buffer, "Planets Before Remove: %d\t%d\t%d", planets[0], planets[1], planets[2]);
	logger->log();
	for (size_t i = 0; i < COLONY_MAX_SIZE; i++){
		if (planets[i] == planet->PlanetID()){
			planets[i] = -1;
			result = true;
			break;
		}
	}
	sprintf(logger->buffer, "Planets After Remove: %d\t%d\t%d", planets[0], planets[1], planets[2]);
	logger->log();
	sprintf(logger->buffer, "Remove Planet %d ID = %d\n", PlanetsCount(), planet->PlanetID());
	logger->log();
	return result;
}

int Colony::PlanetsCount(){
	int count = 0;
	for (size_t i = 0; i < COLONY_MAX_SIZE; i++){
		if (planets[i] != -1){
			count++;
		}
	}
	return count;
}

int Colony::ID(){
	return id;
}

int Colony::Size(){
	return size;
}
