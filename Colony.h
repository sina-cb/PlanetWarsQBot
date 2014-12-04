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

	/**
	 * Runs Q-Learning at the Planet Level, to find the destination planet to attack
	 */
	void DoTurn(const PlanetWars &pw);

	/**
	 * This function should be run at the beginning of each turn to update the colony
	 * parameters.
	 */
	void UpdateColony(const PlanetWars &pw);

	bool addPlanet(Planet *planet, const PlanetWars &pw);
	bool removePlanet(Planet *planet);

	int ID();
	int Size();

	int* Planets() {return planets;}

private:
	int id;
	int planets[COLONY_MAX_SIZE];
	int size;
	std::vector<Fleet*> fleets;

	Logger *logger;
};

#endif /* COLONY_H_ */
