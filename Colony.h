/*
 * Colony.h
 *
 *  Created on: Nov 17, 2014
 *      Author: sina
 */

#ifndef COLONY_H_
#define COLONY_H_

#include "PlanetWars.h"
#include <vector>
#include "Logger.cpp"

#define COLONY_MAX_SIZE 3
#define COLONY_MAX_NEIGHBORS_FOR_EACH_PLANET 3
#define COLONY_MAX_NEIGHBORS 5

class Colony {
public:
	Colony();
	Colony(int id);
	virtual ~Colony();
	void DoTurn(const PlanetWars &pw);

	bool IfNeedUpdate();
	int ID();
	void NeedUpdate();
	void DoNotNeedUpdate();

	bool addNeighbor(Planet *planet);
	bool addPlanet(Planet *planet);
	bool removePlanet(Planet *planet);
	bool removeNeighbor(Planet *planet);

	int* Planets() {return planets;}
	int* Neighbors() {return neighbors;}

	int PlanetsCount();
	int NeighborsCount();

private:
	int id;
	int planets[COLONY_MAX_SIZE];
	int neighbors[COLONY_MAX_NEIGHBORS];
	std::vector<Fleet*> fleets;

	Logger *logger;
};

#endif /* COLONY_H_ */
