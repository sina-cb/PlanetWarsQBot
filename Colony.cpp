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
	for (int i = 0; i < COLONY_MAX_NEIGHBORS_FOR_EACH_PLANET; i++){
		neighbors[i] = -1;
	}
}

Colony::~Colony() {

}

// The DoTurn function is where your code goes. The PlanetWars object contains
// the state of the game, including information about all planets and fleets
// that currently exist. Inside this function, you issue orders using the
// pw.IssueOrder() function. For example, to send 10 ships from planet 3 to
// planet 8, you would say pw.IssueOrder(3, 8, 10).
void Colony::DoTurn(const PlanetWars &pw) {
	// (1) If we currently have a fleet in flight, just do nothing.
	if (pw.MyFleets().size() >= 2) {
		return;
	}
	// (2) Find my strongest planet.
	int source = -1;
	double source_score = -999999.0;
	int source_num_ships = 0;
	for (int i = 0; i < COLONY_MAX_SIZE; ++i) {
		if (planets[i] != -1){
			const Planet* p = pw.GetPlanet(planets[i]);
			double score = (double) p->NumShips();
			if (score > source_score) {
				source_score = score;
				source = p->PlanetID();
				source_num_ships = p->NumShips();
			}
		}
	}
	// (3) Find the weakest enemy or neutral planet.
	int dest = -1;
	double dest_score = 999999.0;
	for (int i = 0; i < COLONY_MAX_NEIGHBORS_FOR_EACH_PLANET; ++i) {
		bool found_one = false;
		if (neighbors[i] != -1){
			const Planet* p = pw.GetPlanet(neighbors[i]);
			double score = p->NumShips();
			if (score < dest_score) {
				dest_score = score;
				dest = p->PlanetID();
			}
			found_one = true;
		}

		if (!found_one){
			dest = pw.EnemyPlanets()[0]->PlanetID();
		}
	}
	// (4) Send half the ships from my strongest planet to the weakest
	// planet that I do not own.
	if (source >= 0 && dest >= 0) {
		int num_ships = source_num_ships / 2;
		pw.IssueOrder(source, dest, num_ships);
	}
}

bool Colony::addNeighbor(Planet *planet){
	bool result = false;
	sprintf(logger->buffer, "Neighbors Before Add: %d\t%d\t%d", neighbors[0], neighbors[1], neighbors[2]);
	logger->log();
	for (size_t i = 0; i < COLONY_MAX_NEIGHBORS; i++){
		if (neighbors[i] == -1){
			neighbors[i] = planet->PlanetID();
			result = true;
			break;
		}
	}
	sprintf(logger->buffer, "Neighbors After Add: %d\t%d\t%d", neighbors[0], neighbors[1], neighbors[2]);
	logger->log();
	sprintf(logger->buffer, "Add Neighbor %d ID = %d\n", NeighborsCount(), planet->PlanetID());
	logger->log();
	return result;
}

bool Colony::removeNeighbor(Planet *planet){
	bool result = false;
	sprintf(logger->buffer, "Neighbors Before Remove: %d\t%d\t%d", neighbors[0], neighbors[1], neighbors[2]);
	logger->log();
	for (size_t i = 0; i < COLONY_MAX_NEIGHBORS; i++){
		if (neighbors[i] == planet->PlanetID()){
			neighbors[i] = -1;
			result = true;
			break;
		}
	}
	sprintf(logger->buffer, "Neighbors After Remove: %d\t%d\t%d", neighbors[0], neighbors[1], neighbors[2]);
	logger->log();
	sprintf(logger->buffer, "Remove Neighbor %d ID = %d\n", NeighborsCount(), planet->PlanetID());
	logger->log();
	return result;
}

bool Colony::addPlanet(Planet *planet){
	bool result = false;
	sprintf(logger->buffer, "Planets Before Add: %d\t%d\t%d", planets[0], planets[1], planets[2]);
	logger->log();
	for (size_t i = 0; i < COLONY_MAX_SIZE; i++){
		if (planets[i] == -1){
			planets[i] = planet->PlanetID();
			result = true;
			break;
		}
	}
	sprintf(logger->buffer, "Planets After Add: %d\t%d\t%d", planets[0], planets[1], planets[2]);
	logger->log();
	sprintf(logger->buffer, "Add Planet %d ID = %d\n", PlanetsCount(), planet->PlanetID());
	logger->log();
	return result;
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

int Colony::NeighborsCount(){
	int count = 0;
	for (size_t i = 0; i < COLONY_MAX_NEIGHBORS_FOR_EACH_PLANET; i++){
		if (neighbors[i] != -1){
			count++;
		}
	}
	return count;
}

int Colony::ID(){
	return id;
}
