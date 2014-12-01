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

#define COLONY_MAX_SIZE 4

class Colony {
public:
	Colony();
	Colony(int id);
	virtual ~Colony();
	void DoTurn(const PlanetWars &pw);

	bool IfNeedUpdate();
	int ID();
	int Size();
	void NeedUpdate();
	void DoNotNeedUpdate();

	bool addPlanet(Planet *planet, const PlanetWars &pw);
	bool removePlanet(Planet *planet);
	int* Planets() {return planets;}

	int PlanetsCount();
	int NeighborsCount();

private:
	int id;
	int planets[COLONY_MAX_SIZE];
	int size;
	std::vector<Fleet*> fleets;

	Logger *logger;
};

#endif /* COLONY_H_ */
