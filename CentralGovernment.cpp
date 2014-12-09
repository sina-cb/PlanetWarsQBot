/*
 * CentralGovernment.cpp
 *
 *  Created on: Nov 17, 2014
 *      Author: sina
 */

#include "CentralGovernment.h"
#include <map>
#include <stdlib.h>     /* atof */
#include <time.h>
using namespace std;

CentralGovernment::CentralGovernment() {
	logger = new Logger("CentralGovernment.log");
	srand(time(NULL));
	writeOnce = true;
}

CentralGovernment::~CentralGovernment() {

}

void CentralGovernment::DoTurn(const PlanetWars &pw){
	sprintf(logger->buffer, "Start DoTurn Central");
	logger->log();

	UpdateColonies(pw);
	HandleColonies(pw);

	sprintf(logger->buffer, "End DoTurn Central\n");
	logger->log();
}

void CentralGovernment::HandleColonies(const PlanetWars &pw){
	//Start Q-learning

	if ((!pw.IsAlivePlanets(ME) || !pw.IsAlivePlanets(ENEMY)) && writeOnce){

		pid_t pid = fork();
		if (pid == 0)
		{
			WriteQValues();
			colonies[0]->QValueObj()->WriteQValues();
			sprintf(logger->buffer, "Child Process: Writing Q-Values complete.");
			logger->log();

			exit(0);
		}else if (pid > 0){
			//Parent
		}else{
			sprintf(logger->buffer, "Fork Faild");
			logger->log();
			exit(0);
		}

		writeOnce = false;
	}

	bool random_action = (((rand() % 1000) / 1000.0) > EXPLOITATION_COLONY) ? true : false;

	if (!pw.IsAlivePlanets(ME)){
		sprintf(logger->buffer, "I died at turn %d.", pw.Turn());
		logger->log();
		random_action = false;
	}

	if (!pw.IsAlivePlanets(ENEMY)){
		sprintf(logger->buffer, "I won at turn %d.", pw.Turn());
		logger->log();
		random_action = false;
	}

	double max_q = -99999999;
	Action* max_action = 0;
	int action_t = -1;

	if (random_action){
		sprintf(logger->buffer, "Exploitation step!!!");
		logger->log();

		while (true){
			action_t = rand() % actions.size();
			max_action = actions[action_t];

			sprintf(logger->buffer, "While True :: Central :: Selected action -> %d", action_t);
			logger->log();

			if (colonies[max_action->source]->HasFriendlyPlanet(pw)){
				break;
			}
		}
	}else{
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
	}

	sprintf(logger->buffer, "Selected Action: %d", action_t);
	logger->log();

	if (action_t != -1){
		sprintf(logger->buffer, "Source Colony: %d , Destination Colony: %d", max_action->source, max_action->destination);
		logger->log();

		bool result = colonies[max_action->source]->DoTurn(pw, colonies[max_action->destination]);

		EstimateNextState(pw);
		CalculatedNewQValue(pw, action_t);
	}
}

void CentralGovernment::CalculatedNewQValue(const PlanetWars &pw, int action_t){
	double alpha = ALPHA_COLONY_Q;

	if (pw.Iteration() != 0){
		alpha = 1.0 / pw.Iteration();
	}

	vector<int> state;
	for (size_t j = 0; j < colonies.size(); j++){
		state.push_back(colonies[j]->Strongness());
		sprintf(logger->buffer, "Strongness #%d: %d", j, colonies[j]->Strongness());
		logger->log();
	}
	state.push_back(action_t);

	/*sprintf(logger->buffer, "action: %d", action_t);
	logger->log();

	sprintf(logger->buffer, "Old state: %d, State size: %d", get_index_for(state), num_q_values);
	logger->log();*/

	double q_old = q_values[get_index_for(state)];

	double max_est_q = -99999999;
	Action* max_est_action = 0;

	if (!pw.IsAlivePlanets(ME)){
		max_est_q = -1;
	}else if (!pw.IsAlivePlanets(ENEMY)){
		max_est_q = 1;
	}else{
		for (size_t i = 0; i < actions.size(); i++){
			vector<int> state_est;
			for (size_t j = 0; j < colonies.size(); j++){
				state_est.push_back(colonies[j]->StrongnessEstimation());
			}
			state_est.push_back(actions[i]->ID);

			if (q_values[get_index_for(state_est)] > max_est_q){
				max_est_q = q_values[get_index_for(state_est)];
				max_est_action = actions[i];
			}
		}
	}

/*	sprintf(logger->buffer, "q_old: %f  |  alpha: %f  |  Reward: %f  |  Discount: %d  |  max_est: %f", q_old, alpha, Reward(pw, actions[action_t]), DISCOUNT_COLONY, max_est_q);
	logger->log();

	sprintf(logger->buffer, "Index: %d", get_index_for(state));
	logger->log();*/

	double q_new = ((1 - alpha) * q_old) + (alpha * (Reward(pw, actions[action_t]) + DISCOUNT_COLONY * max_est_q));
	q_values[get_index_for(state)] = q_new;
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

	int total = 0;

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

	if (total == 0){
		return 0;
	}

	return (result / (double)total);
}

void CentralGovernment::ReadQValues(){
	snprintf(logger->buffer, 100, "%s.q", MAP_NAME_CENTRAL);
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
	snprintf(logger->buffer, 100, "%s.q", MAP_NAME_CENTRAL);
	string file_name = logger->buffer;

	if(remove(file_name.c_str()) != 0)
		perror("Error deleting file");
	else
		puts("File successfully deleted");

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
	int colonyID = 0;
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

	Colony::QValue *q_value_obj = new Colony::QValue();
	q_value_obj->Initialize(pw);

	for(size_t i = 0; i < colonies.size(); i++){
		colonies[i]->QValueObj(q_value_obj);
	}
}
