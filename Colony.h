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
#include "Constants.h"

class Colony {
public:
	Colony();
	Colony(int id);
	virtual ~Colony();

	/**
	 * Runs Q-Learning at the Planet Level, to find the destination planet to attack
	 */
	void DoTurn(const PlanetWars &pw, Colony* destination);

	/**
	 * This function should be run at the beginning of each turn to update the colony
	 * parameters.
	 */
	void UpdateColony(const PlanetWars &pw);

	bool addPlanet(Planet *planet, const PlanetWars &pw);
	bool IfPlanetHere(const PlanetWars &pw, int planetID);
	bool removePlanet(Planet *planet);

	int ID();
	int Size();
	int Strongness();

	int* Planets() {return planets;}

private:
	enum ColonyTypes {F = 1, E = 2, N = 3, FE = 4, FN = 5, EN = 6, FEN = 7} colonyType;

	int strongness;
	int attackThreshold;

	int id;
	int planets[COLONY_MAX_SIZE];
	bool eligable[COLONY_MAX_SIZE];
	int size;
	std::vector<Fleet*> fleets;

	Logger *logger;
};

#endif /* COLONY_H_ */
