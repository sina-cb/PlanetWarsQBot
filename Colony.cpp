/*
 * Colony.cpp
 *
 *  Created on: Nov 17, 2014
 *      Author: sina
 */

#include "Colony.h"
#include <iostream>
using namespace std;

Colony::Colony() {
	this->id = -1;
	changed = false;
}

Colony::Colony(int id){
	this->id = id;
	changed = true;
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
	if (pw.MyFleets().size() >= 1) {
		return;
	}
	// (2) Find my strongest planet.
	int source = -1;
	double source_score = -999999.0;
	int source_num_ships = 0;
	std::vector<Planet> my_planets = pw.MyPlanets();
	for (unsigned int i = 0; i < planets.size(); ++i) {
		const Planet& p = my_planets[i];
		double score = (double) p.NumShips();
		if (score > source_score) {
			source_score = score;
			source = p.PlanetID();
			source_num_ships = p.NumShips();
		}
	}
	// (3) Find the weakest enemy or neutral planet.
	int dest = -1;
	double dest_score = -999999.0;
	std::vector<Planet> not_my_planets = pw.NotMyPlanets();
	for (unsigned int i = 0; i < neighbors.size(); ++i) {
		const Planet& p = not_my_planets[i];
		double score = 1.0 / (1 + p.NumShips());
		if (score > dest_score) {
			dest_score = score;
			dest = p.PlanetID();
		}
	}
	// (4) Send half the ships from my strongest planet to the weakest
	// planet that I do not own.
	if (source >= 0 && dest >= 0) {
		int num_ships = source_num_ships / 2;
		pw.IssueOrder(source, dest, num_ships);
	}
}

void Colony::addNeighbor(Planet *planet){
	neighbors.push_back(planet);
}

bool Colony::IfNeedUpdate(){
	return changed;
}

void Colony::NeedUpdate(){
	changed = true;
}

void Colony::DoNotNeedUpdate(){
	changed = false;
}
