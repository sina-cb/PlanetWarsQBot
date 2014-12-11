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

enum STATE {
	CAREFUL = 0, //in this state the ai is really careful with its moves
	DEFAULT = 1  //this is the default state of the AI
};

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

	vector<double>* makeInfluenceMap(const PlanetWars &pw);
	STATE getState(const PlanetWars &pw);

	vector<int>* MyPlanets(const PlanetWars &pw);
	vector<int>* MyFleets(const PlanetWars &pw);

	vector<int>* EnemyPlanets(const PlanetWars &pw);
	vector<int>* EnemyFleets(const PlanetWars &pw);

	vector<int>* NeutralPlanets(const PlanetWars &pw);
	vector<int>* PlanetsCombined(const PlanetWars &pw);

	vector<int>* getIncomingFleets(const PlanetWars &pw, const Planet* planet);

	int getLeastShipsNeeded(const PlanetWars &pw, const Planet* source, const Planet* dest, int div, int confidence);

	double getCarefulHeuristics(const PlanetWars &pw, int source, int dest, int planetNumShips);
	double getCarefulHeuristics(const PlanetWars &pw, const Planet *source, const Planet *dest);

	double getHeuristics(const Planet *planet, int minInfluence);
	double getHeuristics(int planet, int growthrate, int numships, int minInfluence);

	int calcConfidence(const PlanetWars &pw, const Planet *planet);

private:
	Logger *logger;
	std::vector<Action*> actions;

	int id;
	int planets[COLONY_MAX_SIZE];
	bool eligable[COLONY_MAX_SIZE];
	int size;

	Colony* destination_;

	vector<double> *influenceMap; //influence map
	vector<int> *shipRequestTable; // this will be a table that has all the ships we request to every location
	vector<int> *shipAvailableTable; //this table will have all the ships that is available to us

	STATE state;

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
