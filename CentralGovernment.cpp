/*
 * CentralGovernment.cpp
 *
 *  Created on: Nov 17, 2014
 *      Author: sina
 */

#include "CentralGovernment.h"
#include <map>
#include <stdlib.h>     /* atof */

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

	if (!pw.IsAlive(ME)){
		sprintf(logger->buffer, "I am dead at turn %d: \n\tWriting Q-Values to file.", pw.Turn());
		logger->log();

		WriteQValues();

		sprintf(logger->buffer, "\tWriting Q-Values complete.");
		logger->log();

		return;
	}

	double max_q = -99999999;
	Action* max_action = 0;
	int action_t = -1;
	for (size_t i = 0; i < actions.size(); i++){
		vector<int> indexes;
		for (size_t j = 0; j < colonies.size(); j++){
			indexes.push_back(colonies[j]->Strongness());
		}
		indexes.push_back(actions[i]->ID);

		if (colonies[actions[i]->source]->HasFriendlyPlanet(pw) && q_values[get_index_for(indexes)] > max_q){
			max_q = q_values[get_index_for(indexes)];
			max_action = actions[i];
			action_t = i;
		}
	}

	if (max_q == -99999999){
		return;
	}

	colonies[max_action->source]->DoTurn(pw, colonies[max_action->destination]);

	EstimateNextState(pw);

	CalculatedNewQValue(pw, action_t);

}

void CentralGovernment::CalculatedNewQValue(const PlanetWars &pw, int action_t){
	double alpha = ALPHA_COLONY_Q;

	if (pw.Iteration() != 0){
		alpha = 1.0 / pw.Iteration();
	}

	sprintf(logger->buffer, "Alpha: %f", alpha);
	logger->log();

	vector<int> state;
	for (size_t j = 0; j < colonies.size(); j++){
		state.push_back(colonies[j]->Strongness());
	}
	state.push_back(action_t);

	double q_old = q_values[get_index_for(state)];

	if (!pw.IsAlive(ME)){
		q_old = -1;
	}

	if (!pw.IsAlive(ENEMY)){
		q_old = 1;
	}

	double max_est_q = -99999999;
	Action* max_est_action = 0;
	for (size_t i = 0; i < actions.size(); i++){
		vector<int> state_est;
		for (size_t j = 0; j < colonies.size(); j++){
			state_est.push_back(colonies[j]->StrongnessEstimation());
		}
		state_est.push_back(actions[i]->ID);

		if (q_values[get_index_for(state_est)] > max_est_q){
			max_est_q = q_values[get_index_for(state_est)];
			max_est_action = actions[i];
			action_t = i;
		}
	}

	double q_new = ((1 - alpha) * q_old) + (alpha * (Reward(pw, actions[action_t]) + DISCOUNT_COLONY * max_est_q));
	q_values[get_index_for(state)] = q_new;

	sprintf(logger->buffer, "New Q Value: %f", q_new);
	logger->log();
}

void CentralGovernment::UpdateColonies(const PlanetWars &pw){
	for (size_t i = 0; i < colonies.size(); i++){
		colonies[i]->UpdateColony(pw);
	}
}

void CentralGovernment::EstimateNextState(const PlanetWars &pw){
	for (size_t i = 0; i < colonies.size(); i++){
		colonies[i]->UpdateNextStateColony(pw);
	}
}

double CentralGovernment::Reward(const PlanetWars &pw, Action *action){
	int result = 0;

	int total;

	FleetList fleets = pw.Fleets();
	for (size_t i = 0; i < fleets.size(); i++){
		Fleet* fleet = fleets[i];
		if (colonies[action->destination]->IfPlanetHere(pw, fleet->DestinationPlanet()) ||
				colonies[action->source]->IfPlanetHere(pw, fleet->DestinationPlanet())){
			if (fleet->TurnsRemaining() <= MAX_TURNS_REMAINING_TO_CONSIDER_IN_REWARD){
				if (fleet->Owner() == ME){
					result += (pw.GetPlanet(fleet->DestinationPlanet())->GrowthRate() * fleet->NumShips());
					total += (pw.GetPlanet(fleet->DestinationPlanet())->GrowthRate() * fleet->NumShips());
				}else{
					result -= (pw.GetPlanet(fleet->DestinationPlanet())->GrowthRate() * fleet->NumShips());
					total += (pw.GetPlanet(fleet->DestinationPlanet())->GrowthRate() * fleet->NumShips());
				}
			}
		}
	}

	return (result / (double)total);
}

void CentralGovernment::ReadQValues(){
	snprintf(logger->buffer, 100, "%s.q", MAP_NAME);
	string file_name = logger->buffer;
	bool file_check = check_file_exists(file_name);

	num_q_values = 1;
	for (size_t i = 0; i < dimension; i++){
		num_q_values *= lengths[i];
	}
	sprintf(logger->buffer, "Q-Value array size: %d", num_q_values);
	logger->log();

	q_values = new double[num_q_values];

	if (!file_check){
		//Initialize the Q-Values
		sprintf(logger->buffer, "Q-Values file does not exist.");
		logger->log();

		ofstream out;
		out.open (file_name.c_str());

		for (size_t i = 0; i < num_q_values; i++){
			char buffer[100];
			sprintf(buffer, MAP_FORMAT, 0.0);
			q_values[i] = 0.0;
			out << buffer;
		}
		out.close();

		sprintf(logger->buffer, "Q-Values file created.");
		logger->log();
	}else{
		//Read from file and store in array
		ifstream in;
		in.open(file_name.c_str());
		char inStr[100];

		int index = 0;
		while(!in.eof()){
			in >> inStr;
			q_values[index] = atof(inStr);
		}
		in.close();

		sprintf(logger->buffer, "Q-Values loaded");
		logger->log();
	}
}

void CentralGovernment::WriteQValues(){
	snprintf(logger->buffer, 100, "%s.q", MAP_NAME);
	string file_name = logger->buffer;

	if( remove(file_name.c_str()) != 0 )
		perror( "Error deleting file" );
	else
		puts( "File successfully deleted" );

	ofstream out;
	out.open (file_name.c_str());

	for (size_t i = 0; i < num_q_values; i++){
		char buffer[100];
		sprintf(buffer, MAP_FORMAT, q_values[i]);
		out << buffer;
	}
	out.close();
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

	int count = 0;
	for (size_t i = 0; i < colonies.size(); i++){
		for (size_t j = 0; j < colonies.size(); j++){
			Action *action = new Action(i, j, count);
			count++;
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
		if (i == 0){
			lengths[i] = ((MAX_STRONGNESS - MIN_STRONGNESS) / STEPS);
			sprintf(logger->buffer, "%s%d", logger->buffer, lengths[i]);
		}else{
			lengths[i] = ((MAX_STRONGNESS - MIN_STRONGNESS) / STEPS);
			sprintf(logger->buffer, "%s - %d", logger->buffer, lengths[i]);
		}
	}
	lengths[dimension - 1] = actions.size();
	sprintf(logger->buffer, "%s - %d", logger->buffer, lengths[dimension - 1]);
	logger->log();

	ReadQValues();
}
