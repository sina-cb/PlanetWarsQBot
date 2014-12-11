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

CentralGovernment::CentralGovernment(char* map_name) {
	logger = new Logger("CentralGovernment.log");
	srand(time(NULL));
	writeOnce = true;
	map_name_ = map_name;
	game_state = ON_GOING;
}

CentralGovernment::~CentralGovernment() {

}

void CentralGovernment::DoTurn(const PlanetWars &pw){
	sprintf(logger->buffer, "DoTurn (%d):: START", pw.Turn());
	logger->log();

	UpdateState(pw);
	TakeAction(pw);

	if (action_ships != -1){
		UpdateStateP(pw);
		UpdateQValues(pw);
	}

	sprintf(logger->buffer, "DoTurn:: END\n");
	logger->log();
}

void CentralGovernment::TakeAction(const PlanetWars &pw){
	sprintf(logger->buffer, "\tTakeAction:: START");
	logger->log();

	//Start Q-learning
	if ((!pw.IsAlivePlanets(ME) || !pw.IsAlivePlanets(ENEMY)) && writeOnce){
		sprintf(logger->buffer, "\t\tForking to write the Q-Values!");
		logger->log();

		pid_t pid = fork();
		if (pid == 0)
		{
			//Child
			WriteQValues();
			sprintf(logger->buffer, "Child Process: Writing Q-Values complete.");
			logger->log();
			exit(0);
		}else if (pid > 0){
			//Parent
		}else{
			sprintf(logger->buffer, "\t\tFork Failed");
			logger->log();
			exit(0);
		}

		writeOnce = false;
	}

	bool random_action = (((rand() % 1000) / 1000.0) > COLONY_EXPLOITION) ? true : false;

	if (!pw.IsAlivePlanets(ME)){
		game_state = LOST;
		random_action = false;

		sprintf(logger->buffer, "\t\tI died at turn %d.", pw.Turn());
		logger->log();
	}

	if (!pw.IsAlivePlanets(ENEMY)){
		game_state = WIN;
		random_action = false;

		sprintf(logger->buffer, "\t\tI won at turn %d.", pw.Turn());
		logger->log();
	}

	Action* max_action = 0;
	int action_t = -1;

	if (random_action){
		sprintf(logger->buffer, "\t\tExploitation step!!!");
		logger->log();

		while (true){
			action_t = rand() % actions.size();
			max_action = actions[action_t];

			sprintf(logger->buffer, "\t\tWhile::Selected action -> %d", action_t);
			logger->log();

			if (colonies[max_action->source]->HasFriendlyPlanet(pw)){
				break;
			}
		}
	}else{
		double max_q = -99999999;
		for (size_t i = 0; i < actions.size(); i++){
			vector<int> indexes;
			for (size_t j = 0; j < state.size(); j++){
				indexes.push_back(state[j]);
			}
			indexes.push_back(actions[i]->ID);

			double q_value_temp = q_values[get_index_for(indexes)];
			if (colonies[actions[i]->source]->HasFriendlyPlanet(pw) &&  q_value_temp > max_q){
				max_q = q_value_temp;
				max_action = actions[i];
				action_t = i;
			}
		}
	}

	sprintf(logger->buffer, "\t\tSelected Action: %d", action_t);
	logger->log();

	if (action_t != -1){
		sprintf(logger->buffer, "\t\tSource Colony: %d , Destination Colony: %d", max_action->source, max_action->destination);
		logger->log();

		action_ships = colonies[max_action->source]->DoTurn(pw, colonies[max_action->destination]);
		chosen_action = max_action;
	}else{
		action_ships = -1;
		chosen_action = 0;
	}

	sprintf(logger->buffer, "\t\tTotal ships sent: %d", action_ships);
	logger->log();

	sprintf(logger->buffer, "\tTakeAction:: END\n");
	logger->log();
}

void CentralGovernment::UpdateQValues(const PlanetWars &pw){

	sprintf(logger->buffer, "\tUpdateQValues:: START");
	logger->log();

	double alpha = COLONY_ALPHA;

	if (pw.Iteration() != 0){
		alpha = 1.0 / pw.Iteration();
	}

	vector<int> state_temp;
	for (size_t j = 0; j < state.size(); j++){
		state_temp.push_back(state[j]);
	}
	state_temp.push_back(chosen_action->ID);

	int current_state_index = get_index_for(state_temp);

	/*sprintf(logger->buffer, "\t\tCurrent state index: %d  |  Current state:", current_state_index);
	logger->log();
	logger->log_array(state_temp);*/

	double old_q = q_values[current_state_index];

	double max_est_q = -99999999;
	Action* max_est_action = 0;

	for (size_t i = 0; i < actions.size(); i++){
		vector<int> state_p_temp;
		for (size_t j = 0; j < state_p.size(); j++){
			state_p_temp.push_back(state_p[j]);
		}
		state_p_temp.push_back(actions[i]->ID);

		int state_p_index = get_index_for(state_p_temp);

		/*sprintf(logger->buffer, "\t\tState P index: %d  |  State P:", state_p_index);
		logger->log();
		logger->log_array(state_p_temp);*/

		double p_q = q_values[state_p_index];

		if (p_q > max_est_q){
			max_est_q = p_q;
			max_est_action = actions[i];
		}
	}

	/*sprintf(logger->buffer, "\t\tq_old: %f  |  alpha: %f  |  Reward: %f  |  Discount: %d  |  max_est: %f", old_q, alpha, Reward(pw), COLONY_DISCOUNT, max_est_q);
	logger->log();*/

	double q_new = ((1 - alpha) * old_q) + (alpha * (Reward(pw) + COLONY_DISCOUNT * max_est_q));
	q_values[current_state_index] = q_new;

	sprintf(logger->buffer, "\tUpdateQValues:: END\n");
	logger->log();
}

void CentralGovernment::UpdateState(const PlanetWars &pw){
	sprintf(logger->buffer, "\tUpdateState:: START");
	logger->log();

	state.clear();
	for (size_t i = 0; i < colonies.size(); i++){
		Colony* colony = colonies[i];

		int temp_state = 0;
		double average_growth = 0;
		for (size_t i = 0; i < colony->Size(); i++){
			const Planet *planet = pw.GetPlanet(colony->Planets()[i]);
			if (planet->Owner() == ME){
				temp_state += (planet->NumShips() * planet->GrowthRate());
			}else if(planet->Owner() == ENEMY){
				temp_state -= (planet->NumShips() * planet->GrowthRate());
			}else if(planet->Owner() == NEUTRAL){
				temp_state -= planet->NumShips();
			}
		}

		if (temp_state < COLONY_STRONGNESS_MIN){
			temp_state = COLONY_STRONGNESS_MIN;
		}else if (temp_state >= COLONY_STRONGNESS_MAX){
			temp_state = COLONY_STRONGNESS_MAX - 1;
		}

		temp_state = (temp_state - COLONY_STRONGNESS_MIN) / COLONY_STRONGNESS_STEP;
		state.push_back(temp_state);
	}

	sprintf(logger->buffer, "\tCurrent State:");
	logger->log();
	logger->log_array("\t\t", state);

	sprintf(logger->buffer, "\tUpdateState:: END\n");
	logger->log();
}

void CentralGovernment::UpdateStateP(const PlanetWars &pw){
	sprintf(logger->buffer, "\tUpdateStateP:: START");
	logger->log();

	state_p.clear();
	for (size_t i = 0; i < colonies.size(); i++){
		Colony* colony = colonies[i];

		int temp_state = 0;
		double average_growth = 0;
		for (size_t i = 0; i < colony->Size(); i++){
			const Planet *planet = pw.GetPlanetNewState(colony->Planets()[i]);
			if (planet->Owner() == ME){
				temp_state += (planet->NumShips() * planet->GrowthRate());
			}else if(planet->Owner() == ENEMY){
				temp_state -= (planet->NumShips() * planet->GrowthRate());
			}else if(planet->Owner() == NEUTRAL){
				temp_state -= planet->NumShips();
			}
		}

		if (temp_state < COLONY_STRONGNESS_MIN){
			temp_state = COLONY_STRONGNESS_MIN;
		}else if (temp_state >= COLONY_STRONGNESS_MAX){
			temp_state = COLONY_STRONGNESS_MAX - 1;
		}

		temp_state = (temp_state - COLONY_STRONGNESS_MIN) / COLONY_STRONGNESS_STEP;
		state_p.push_back(temp_state);
	}

	sprintf(logger->buffer, "\tEstimated State:");
	logger->log();
	logger->log_array("\t\t", state_p);

	sprintf(logger->buffer, "\tUpdateStateP:: END\n");
	logger->log();
}

double CentralGovernment::Reward(const PlanetWars &pw){
	int result = action_ships;

	if (result > COLONY_REWARD_MAX){
		result = COLONY_REWARD_MAX;
	}

	if (result < COLONY_REWARD_MIN){
		result = COLONY_REWARD_MIN;
	}

	if (game_state == ON_GOING){
		//Nothing YET!!!
	}else if (game_state == WIN){
		//Reward the agent for wining
		result += GAME_WIN_REWARD;
	}else if (game_state == LOST){
		//Punish the agent for losing
		result += GAME_LOSE_REWARD;
	}

	return result;
}

void CentralGovernment::ReadQValues(){
	snprintf(logger->buffer, 100, "q-values/%s.q", map_name_);
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
			index++;
		}
		in.close();

		sprintf(logger->buffer, "Q-Values loaded");
		logger->log();
	}
}

void CentralGovernment::WriteQValues(){
	Logger child_logger("Child.log");

	sprintf(child_logger.buffer, "Child Start");
	child_logger.log();

	char map_name[500];
	sprintf(map_name, "q-values/%s.q", map_name_);
	string file_name = map_name;

	if(remove(file_name.c_str()) != 0){
		sprintf(child_logger.buffer, "Error deleting file");
		child_logger.log();
	}else{
		sprintf(child_logger.buffer, "File Deleted!");
		child_logger.log();
	}

	ofstream out;
	out.open (file_name.c_str());

	int zero_count = 0;
	for (size_t i = 0; i < num_q_values; i++){
		char buffer[100];
		sprintf(buffer, MAP_FORMAT, q_values[i]);
		out << buffer;

		if (q_values[i] == 0){
			zero_count++;
		}
	}
	out.close();

	sprintf(child_logger.buffer, "Child Process:: Zero counts after writing: %d", zero_count);
	child_logger.log();
}

void CentralGovernment::InitializeColonies(const PlanetWars &pw){
	sprintf(logger->buffer, "Initialization Started!");
	logger->log();

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
			if (i != j){
				Action *action = new Action(i, j, count);
				count++;
				actions.push_back(action);
			}
		}
	}

	dimension = colonies.size() + 1;
	sprintf(logger->buffer, "Q-Value array dimension: %d", dimension);
	logger->log();
	lengths = new int[dimension];
	sprintf(logger->buffer, "");
	for (size_t i = 0; i < dimension - 1; i++){
		if (i == 0){
			lengths[i] = ((COLONY_STRONGNESS_MAX - COLONY_STRONGNESS_MIN) / COLONY_STRONGNESS_STEP);
			sprintf(logger->buffer, "%s%d", logger->buffer, lengths[i]);
		}else{
			lengths[i] = ((COLONY_STRONGNESS_MAX - COLONY_STRONGNESS_MIN) / COLONY_STRONGNESS_STEP);
			sprintf(logger->buffer, "%s - %d", logger->buffer, lengths[i]);
		}
	}
	lengths[dimension - 1] = actions.size();

	sprintf(logger->buffer, "%s - %d", logger->buffer, lengths[dimension - 1]);
	logger->log();

	ReadQValues();

	sprintf(logger->buffer, "Initialization Finished!\n");
	logger->log();
}
