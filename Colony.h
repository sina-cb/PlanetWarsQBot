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
#include <sys/stat.h>
#include "Logger.cpp"
#include "Constants.h"

class Colony {
public:
	class Action;

	Colony();
	Colony(int id);
	virtual ~Colony();

	/**
	 * Runs Q-Learning at the Planet Level, to find the destination planet to attack
	 */
	int DoTurn(const PlanetWars &pw, Colony* destination);

	/**
	 * This function should be run at the beginning of each turn to update the colony
	 * parameters.
	 */
	void UpdateNextStateColony(const PlanetWars &pw);

	bool addPlanet(Planet *planet, const PlanetWars &pw);
	bool IfPlanetHere(const PlanetWars &pw, int planetID);
	bool removePlanet(Planet *planet);

	int ID();
	int Size();
	bool HasFriendlyPlanet(const PlanetWars &pw);
	int* Planets() {return planets;}

	void Initialize(const PlanetWars &pw);

	double Reward(const PlanetWars &pw, int action_t);
	std::vector<int>* DecideNumShips(const PlanetWars &pw, vector<int> &sources, int dest);

	void SetEligable(int index, bool eligable);

private:
	Logger *logger;
	std::vector<Action*> actions;

	int id;
	int planets[COLONY_MAX_SIZE];
	bool eligable[COLONY_MAX_SIZE];
	int size;

public:
	class Action{
	public:
		Action(int destication){
			this->destination = destication;
		}

		int destination;
	};
};

#endif /* COLONY_H_ */
