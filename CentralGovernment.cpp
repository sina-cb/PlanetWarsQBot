/*
 * CentralGovernment.cpp
 *
 *  Created on: Nov 17, 2014
 *      Author: sina
 */

#include "CentralGovernment.h"
#include <map>

using namespace std;

CentralGovernment::CentralGovernment() {
	logger = new Logger("CentralGovernment.log");
}

CentralGovernment::~CentralGovernment() {

}

void CentralGovernment::DoTurn(const PlanetWars &pw){
	UpdateColonies(pw);
	HandleColonies(pw);
}

void CentralGovernment::HandleColonies(const PlanetWars &pw){
	boost::thread first_worker(&Colony::DoTurn, colonies[0], pw);

	std::cout << "Start Turn!" << std::endl;
	first_worker.join();
	std::cout << "End Turn!" << std::endl;
}

void CentralGovernment::UpdateColonies(const PlanetWars &pw){
	PlanetList enemyPlanets = pw.EnemyPlanets();
	PlanetList myPlanets = pw.MyPlanets();

	std::map<int, int> &planetColony = *pw.PlanetColony();
	std::map<int, int> &neutralPlanetColony = *pw.NeutralPlanetColony();

	for (size_t i = 0; i < enemyPlanets.size(); i++){
		// If a planet which we own, is taken by the enemy during the last turn
		if (planetColony[enemyPlanets[i]->PlanetID()] != -1){
			colonies[planetColony[enemyPlanets[i]->PlanetID()]]->removePlanet(enemyPlanets[i]);
		}

		// If a neutral planet which is in one of our colony neighbors, is taken by the enemy during the last turn
		if (neutralPlanetColony[enemyPlanets[i]->PlanetID()] != -1){
			colonies[neutralPlanetColony[enemyPlanets[i]->PlanetID()]]->removeNeighbor(enemyPlanets[i]);
		}

		// Assign -1 to the Planet to Colony Map for the taken planet
		planetColony[enemyPlanets[i]->PlanetID()] = -1;
		neutralPlanetColony[enemyPlanets[i]->PlanetID()] = -1;
	}

	for (size_t i = 0; i < myPlanets.size(); i++){
		// If a neutral planet which is in one of our colony neighbors, is overtaken by our space ships,
		// I remove it from the neighbors because it is in the planets list
		if (neutralPlanetColony[myPlanets[i]->PlanetID()] != -1){
			sprintf(logger->buffer, "index %d", myPlanets[i]->PlanetID());
			logger->log();
			colonies[neutralPlanetColony[myPlanets[i]->PlanetID()]]->removeNeighbor(myPlanets[i]);
		}

		// Assign -1 to the Planet to Colony Map for the taken planet
		neutralPlanetColony[myPlanets[i]->PlanetID()] = -1;
	}

	/*sprintf(logger->buffer, "Planets count: %d", myPlanets.size());
	logger->log();*/

	for (size_t i = 0; i < myPlanets.size(); i++){
		Planet *planet = myPlanets[i];

		// If planet is already in a colony, move on and go check the other planet you own.
		if (planetColony[planet->PlanetID()] >= 0){
			continue;
		}

		/*sprintf(logger->buffer, "Colonies Count Before Adding New Planet: %d", colonies.size());
		logger->log();*/

		// Try to find the nearest colony to the planet we have
		PlanetList ourNearestPlanets = pw.NClosestMinePlanets(planet->PlanetID(), pw.Planets().size() - 1);
		int colonyIndex = -1;
		for (size_t k = 0; k < ourNearestPlanets.size(); k++){
			if (planetColony[ourNearestPlanets[k]->PlanetID()] != -1
					&& colonies[planetColony[ourNearestPlanets[k]->PlanetID()]]->PlanetsCount() < COLONY_MAX_SIZE){
				colonyIndex = colonies[planetColony[ourNearestPlanets[k]->PlanetID()]]->ID();
			}
		}

		if (colonyIndex == -1){
			Colony *new_colony = new Colony(colonies.size());
			colonies.push_back(new_colony);
			colonyIndex = colonies.size() - 1;
		}

/*		sprintf(logger->buffer, "Colonies Count After Adding New Planet: %d", colonies.size());
		logger->log();*/

		colonies[colonyIndex]->addPlanet(planet);

		planetColony[planet->PlanetID()] = colonies[colonyIndex]->ID();

		PlanetList n_nearest_neutral_planets = pw.NClosestNeutralPlanets(planet->PlanetID(), COLONY_MAX_SIZE);

		for (size_t j = 0; j < n_nearest_neutral_planets.size(); j++){
			if (neutralPlanetColony[n_nearest_neutral_planets[j]->PlanetID()] == -1){
				bool res = colonies[colonyIndex]->addNeighbor(n_nearest_neutral_planets[j]);
				if (res)
					neutralPlanetColony[n_nearest_neutral_planets[j]->PlanetID()] = colonyIndex;
			}
		}
	}

}
