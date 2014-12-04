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
	//Start Q-learning



}

void CentralGovernment::UpdateColonies(const PlanetWars &pw){
	for (size_t i = 0; i < colonies.size(); i++){
		colonies[i]->UpdateColony(pw);
		/*sprintf(logger->buffer, "Strongness Colony #%d: %d", colonies[i]->ID(), colonies[i]->Strongness());
		logger->log();*/
	}
}

int CentralGovernment::Reward(const PlanetWars &pw, Action *action){
	int result = 0;

	FleetList fleets = pw.Fleets();
	for (size_t i = 0; i < fleets.size(); i++){
		Fleet* fleet = fleets[i];
		if (colonies[action->destination]->IfPlanetHere(pw, fleet->DestinationPlanet()) ||
				colonies[action->source]->IfPlanetHere(pw, fleet->DestinationPlanet())){
			if (fleet->TurnsRemaining() <= MAX_TURNS_REMAINING_TO_CONSIDER_IN_REWARD){
				if (fleet->Owner() == ME){
					result += (pw.GetPlanet(fleet->DestinationPlanet())->GrowthRate() * fleet->NumShips());
				}else{
					result -= (pw.GetPlanet(fleet->DestinationPlanet())->GrowthRate() * fleet->NumShips());
				}
			}
		}
	}

	return result;
}

void CentralGovernment::ReadQValues(){
	string file_name = "colony-q-values.q";
	bool file_check = check_file_exists(file_name);

	num_q_values = 1;
	for (size_t i = 0; i < dimension; i++){
		num_q_values *= lengths[i];
	}
	sprintf(logger->buffer, "Q-Value array size: %d", num_q_values);
	logger->log();

	q_values = new int[num_q_values];

	if (!file_check){
		//Initialize the Q-Values
		sprintf(logger->buffer, "Q-Values file does not exist.");
		logger->log();
		for (size_t i = 0; i < num_q_values; i++){
			q_values[i] = 0;
		}
	}else{
		//Read from file and store in array
		sprintf(logger->buffer, "Q-Values file exists - Not Read Yet!!!");
		logger->log();
	}
}

void CentralGovernment::InitializeColonies(const PlanetWars &pw){
	PlanetList planets = pw.Planets();
	map<int, int> &planetToColony = *pw.PlanetColony();

	for (size_t i = 0; i < planets.size(); i++){
		planetToColony[planets[i]->PlanetID()] = -1;
	}

	size_t index = 0;
	int colonyID = 1;
	while (index < planets.size()){
		if (planetToColony[planets[index]->PlanetID()] != -1){
			index++;
			continue;
		}

		Colony *colony = new Colony(colonyID);
		colony->addPlanet(planets[index], pw);

		PlanetList nearestPlanets = pw.NClosestPlanets(planets[index]->PlanetID(), planets.size());
		for (size_t j = 0; j < nearestPlanets.size() && colony->Size() < COLONY_MAX_SIZE; j++){
			if (planetToColony[nearestPlanets[j]->PlanetID()] == -1){
				colony->addPlanet(nearestPlanets[j], pw);
			}
		}
		colonies.push_back(colony);
		colonyID++;
	}

	sprintf(logger->buffer, "Colonies Size: %d", (int)colonies.size());
	logger->log();
	for (size_t i = 0; i < colonies.size(); i++){
		sprintf(logger->buffer, "Colony %d: %d, %d, %d, %d", i + 1, colonies[i]->Planets()[0],
				colonies[i]->Planets()[1], colonies[i]->Planets()[2], colonies[i]->Planets()[3]);
		logger->log();
	}

	for (size_t i = 0; i < colonies.size(); i++){
		for (size_t j = 0; j < colonies.size(); j++){
			Action *action = new Action(i, j);
			actions.push_back(action);
		}
	}

	UpdateColonies(pw);

	dimension = colonies.size() + 1;
	sprintf(logger->buffer, "Q-Value array dimension: %d", dimension);
	logger->log();
	lengths = new int[dimension];
	sprintf(logger->buffer, "");
	for (size_t i = 0; i < dimension - 1; i++){
		lengths[i] = ((MAX_STRONGNESS - MIN_STRONGNESS) / STEPS);
		sprintf(logger->buffer, "%s - %d", logger->buffer, lengths[i]);
	}
	lengths[dimension - 1] = actions.size();
	sprintf(logger->buffer, "%s - %d", logger->buffer, lengths[dimension - 1]);
	logger->log();

	ReadQValues();
}
