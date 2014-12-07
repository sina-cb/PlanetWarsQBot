/*
 * Colony.cpp
 *
 *  Created on: Nov 17, 2014
 *      Author: sina
 */

#include "Colony.h"
#include <iostream>
#include <algorithm>
using namespace std;

Colony::Colony(int id){
	this->id = id;

	char buffer[100];
	sprintf(buffer, "Colony_%d.log", id);
	logger = new Logger(buffer);

	for (int i = 0; i < COLONY_MAX_SIZE; i++){
		planets[i] = -1;
	}
	size = 0;

	attackThreshold = 30;
	srand(time(NULL));
}

Colony::~Colony() {

}

// The DoTurn function is where your code goes. The PlanetWars object contains
// the state of the game, including information about all planets and fleets
// that currently exist. Inside this function, you issue orders using the
// pw.IssueOrder() function. For example, to send 10 ships from planet 3 to
// planet 8, you would say pw.IssueOrder(3, 8, 10).
void Colony::DoTurn(const PlanetWars &pw, Colony *destination) {
	sprintf(logger->buffer, "START TURN:");
	logger->log();

	// (1) Check if we don't exceed the max number of fleets
	int num_my_fleets = pw.MyFleets().size();
	if (num_my_fleets >= MAX_NUM_MY_FLEETS){
		return;
	}

	//**************************************************//

	sprintf(logger->buffer, "Source Colony: %d , Destination Colony: %d", ID(), destination->ID());
	logger->log();

	// (2) Find my eligible planets to attack.
	vector<int> sources;
	for (size_t i = 0; i < size; i++){
		const Planet* planet = pw.GetPlanet(planets[i]);
		if (planet->Owner() == ME){
			if (planet->NumShips() > ELIGIBILITY_THRESHOLD_PERCENT){
				sources.push_back(planets[i]);
				sprintf(logger->buffer, "Source Planet: %d", planets[i]);
				logger->log();
			}
		}
	}

	if (sources.size() == 0){
		return;
	}

	// (3) Discretize strongness of each planet in the colony
	vector<int> planet_strongness;

	for (size_t i = 0; i < COLONY_MAX_SIZE; i++){
		if (i >= destination->Size()){
			planet_strongness.push_back(0);
		}else{
			int ships_num = pw.GetPlanet(destination->Planets()[i])->NumShips();
			ships_num *= pw.GetPlanet(destination->Planets()[i])->GrowthRate();
			if (ships_num < PLANET_STRONGNESS_MIN){
				ships_num = PLANET_STRONGNESS_MIN;
			}else if (ships_num >= PLANET_STRONGNESS_MAX){
				ships_num = PLANET_STRONGNESS_MAX;
			}

			ships_num = (ships_num - PLANET_STRONGNESS_MIN) / PLANET_STRONGNESS_STEP;
			planet_strongness.push_back(ships_num);
		}
	}

	sprintf(logger->buffer, "Destination Strongness: ");
	for (size_t i = 0; i < planet_strongness.size(); i++){
		sprintf(logger->buffer, "%s%d - ", logger->buffer, planet_strongness[i]);
	}
	logger->log();

	bool random_action = (((rand() % 1000) / 1000.0) > EXPLOITATION_PLANET) ? true : false;
	double max_q = -99999999;
	Action* max_action = 0;
	int action_t = -1;

	if (random_action){
		sprintf(logger->buffer, "Random decision");
		logger->log();

		while (true){
			action_t = rand() % q_value_obj->actions.size();
			max_action = q_value_obj->actions[action_t];

			if (max_action->destination == -1){
				return;
			}

			if (destination->Planets()[max_action->destination] != -1){
				break;
			}
		}
	}else{
		for (size_t i = 0; i < q_value_obj->actions.size(); i++){
			vector<int> indexes;
			for (size_t j = 0; j < planet_strongness.size(); j++){
				indexes.push_back(planet_strongness[j]);
			}
			if (num_my_fleets > MAX_NUM_MY_FLEETS){
				indexes.push_back(MAX_NUM_MY_FLEETS);
			}else{
				indexes.push_back(num_my_fleets);
			}
			indexes.push_back(i);

			if (destination->Planets()[q_value_obj->actions[i]->destination] != -1 && q_value_obj->q_values[get_index_for(indexes)] > max_q){
				max_q = q_value_obj->q_values[get_index_for(indexes)];
				max_action = q_value_obj->actions[i];
				action_t = i;
			}
		}

		if (max_q == -99999999){
			return;
		}
	}

	// (3.1) Find out how many ships should be in the fleet
	vector<int> num_ships;

	int chosen = 0;
	if (pw.GetPlanet(destination->Planets()[max_action->destination])->Owner() == ENEMY){
		for (int j = FLEET_SIZE_ITERATIONS; j >= 0; j--){
			sprintf(logger->buffer, "Checking STEP: %f", FLEET_SIZE_CONSTANT - FLEET_SIZE_CONSTANT_STEP * j);
			logger->log();

			int total_friendly_ships = 0;
			int average_dist = 0;

			for (size_t i = 0; i < sources.size(); i++){
				total_friendly_ships += ((pw.GetPlanet(sources[i])->NumShips()) * (FLEET_SIZE_CONSTANT - FLEET_SIZE_CONSTANT_STEP * j));

				sprintf(logger->buffer, "Distance between %d and %d: %d", sources[i], destination->Planets()[max_action->destination], pw.Distance(sources[i], destination->Planets()[max_action->destination]));
				logger->log();

				average_dist += pw.Distance(sources[i], destination->Planets()[max_action->destination]);
			}
			average_dist = ((int) average_dist / sources.size()) + 1;

			int enemy_strongness = pw.GetPlanet(destination->Planets()[max_action->destination])->NumShips();
			enemy_strongness += (average_dist * pw.GetPlanet(destination->Planets()[max_action->destination])->GrowthRate());

			if (total_friendly_ships > enemy_strongness){
				chosen = j;
				break;
			}
		}
	}else{
		chosen = FLEET_SIZE_ITERATIONS - 1;
	}

	for (size_t i = 0; i < sources.size(); i++){
		num_ships.push_back( (int) ((pw.GetPlanet(sources[i])->NumShips()) * (FLEET_SIZE_CONSTANT - (chosen - 1) * FLEET_SIZE_CONSTANT_STEP)));
	}

	// (4) Send half the ships from my strongest planet to the weakest
	// planet that I do not own.
	if (sources.size() > 0 && max_action->destination != -1){
		for (size_t i = 0; i < sources.size(); i++){
			pw.IssueOrder(sources[i], destination->Planets()[max_action->destination], num_ships[i]);
			sprintf(logger->buffer, "Attack %d: Source: %d, Dest.: %d Ships: %d", (i + 1), sources[i], destination->Planets()[max_action->destination], num_ships[i]);
			logger->log();
		}
	}

	// (5) Discretize strongness of each planet in the colony for the next state
	vector<int> planet_strongness_new;

	for (size_t i = 0; i < COLONY_MAX_SIZE; i++){
		if (i >= destination->Size()){
			planet_strongness_new.push_back(0);
		}else{
			int ships_num = pw.GetPlanetNewState(destination->Planets()[i])->NumShips();
			ships_num *= pw.GetPlanet(destination->Planets()[i])->GrowthRate();
			if (ships_num < PLANET_STRONGNESS_MIN){
				ships_num = PLANET_STRONGNESS_MIN;
			}else if (ships_num >= PLANET_STRONGNESS_MAX){
				ships_num = PLANET_STRONGNESS_MAX;
			}

			ships_num = (ships_num - PLANET_STRONGNESS_MIN) / PLANET_STRONGNESS_STEP;
			planet_strongness_new.push_back(ships_num);
		}
	}

	CalculatedNewQValue(pw, planet_strongness, planet_strongness_new, action_t, num_my_fleets);

	sprintf(logger->buffer, "END TURN:\n");
	logger->log();
}

void Colony::CalculatedNewQValue(const PlanetWars &pw, vector<int> &old_strongness, vector<int> &new_strongness, int action_t, int num_fleets){

	//Create previous state
	old_strongness.push_back(num_fleets);
	old_strongness.push_back(action_t);

	//Create estimated new state
	new_strongness.push_back(num_fleets);

	double new_q_max = -99999999;
	for (size_t i = 0; i < q_value_obj->actions.size(); i++){
		vector<int> indexes;
		for (size_t j = 0; j < new_strongness.size(); j++){
			indexes.push_back(new_strongness[j]);
		}
		indexes.push_back(i);

		if (q_value_obj->q_values[get_index_for(indexes)] > new_q_max){
			new_q_max = q_value_obj->q_values[get_index_for(indexes)];
		}
	}

	double new_q = (1 - ALPHA_PLANET_Q) * q_value_obj->q_values[get_index_for(old_strongness)];
	new_q += ((ALPHA_PLANET_Q) * (Reward(pw, action_t) + DISCOUNT_PLANET * new_q_max));

	q_value_obj->q_values[get_index_for(old_strongness)] = new_q;
}

double Colony::Reward(const PlanetWars &pw, int action_t){
	int result = 0;

	int total = 0;

	FleetList fleets = pw.Fleets();
	for (size_t i = 0; i < fleets.size(); i++){
		Fleet* fleet = fleets[i];
		if (IfPlanetHere(pw, fleet->DestinationPlanet())){
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

void Colony::UpdateColony(const PlanetWars &pw){
	strongness = 0;
	for (size_t i = 0; i < size; i++){
		const Planet *planet = pw.GetPlanet(planets[i]);
		if (planet->Owner() == ME){
			strongness += planet->GrowthRate() * planet->NumShips();
		}else if(planet->Owner() == ENEMY){
			strongness -= planet->GrowthRate() * planet->NumShips();
		}else if(planet->Owner() == NEUTRAL){
			strongness -= planet->NumShips();
		}

		if (planet->GrowthRate() * planet->NumShips() < attackThreshold){
			eligable[i] = false;
		}else{
			eligable[i] = true;
		}
	}

	//Discretization of strongness parameter
	if (strongness < MIN_STRONGNESS){
		strongness = MIN_STRONGNESS;
	}else if (strongness >= MAX_STRONGNESS){
		strongness = MAX_STRONGNESS;
	}

	strongness = (strongness - MIN_STRONGNESS) / STEPS;

	//TODO: We haven't updated the colonyType variable, if you needed it, update it! :D
}

void Colony::UpdateNextStateColony(const PlanetWars& pw){
	strongness_next_state = 0;
	for (size_t i = 0; i < size; i++){
		const Planet *planet = pw.GetPlanetNewState(planets[i]);
		if (planet->Owner() == ME){
			strongness_next_state += planet->GrowthRate() * planet->NumShips();
		}else if(planet->Owner() == ENEMY){
			strongness_next_state -= planet->GrowthRate() * planet->NumShips();
		}else if(planet->Owner() == NEUTRAL){
			strongness_next_state -= planet->NumShips();
		}

		if (planet->GrowthRate() * planet->NumShips() < attackThreshold){
			eligable[i] = false;
		}else{
			eligable[i] = true;
		}
	}

	//Discretization of strongness parameter
	if (strongness_next_state < MIN_STRONGNESS){
		strongness_next_state = MIN_STRONGNESS;
	}else if (strongness_next_state >= MAX_STRONGNESS){
		strongness_next_state = MAX_STRONGNESS;
	}

	strongness_next_state = (strongness_next_state - MIN_STRONGNESS) / STEPS;
}

bool Colony::addPlanet(Planet *planet, const PlanetWars &pw){
	map<int, int> &planetToColony = *pw.PlanetColony();

	planets[size] = planet->PlanetID();
	planetToColony[planet->PlanetID()] = ID();
	size++;

	return true;
}

bool Colony::IfPlanetHere(const PlanetWars &pw, int planetID){
	for (size_t i = 0; i < size; i++){
		if (pw.GetPlanet(planets[i])->PlanetID() == planetID){
			return true;
		}
	}
	return false;
}

bool Colony::removePlanet(Planet* planet){
	return false;
}

int Colony::ID(){
	return id;
}

int Colony::Size(){
	return size;
}

int Colony::Strongness(){
	return strongness;
}

int Colony::StrongnessEstimation(){
	return strongness_next_state;
}

bool Colony::HasFriendlyPlanet(const PlanetWars &pw){
	for (size_t i = 0; i < size; i++){
		if (pw.GetPlanet(planets[i])->Owner() == ME){
			return true;
		}
	}
	return false;
}

void Colony::QValue::Initialize(const PlanetWars &pw){
	Logger *logger = new Logger("Colonies.log");

	//For each planet in the colony we try to attack
	dimension = COLONY_MAX_SIZE;

	// For number of fleets
	dimension++;

	// For number of actions
	dimension++;

	sprintf(logger->buffer, "Q-Value array dimension: %d", dimension);
	logger->log();

	for (size_t i = 0; i < COLONY_MAX_SIZE; i++){
		actions.push_back(new Action(i));
	}
	//NO-OP Action
	actions.push_back(new Action(-1));

	lengths = new int[dimension];
	sprintf(logger->buffer, "");
	for (size_t i = 0; i < COLONY_MAX_SIZE; i++){
		if (i == 0){
			lengths[i] = STRONGNESS_MAX;
			sprintf(logger->buffer, "%s%d", logger->buffer, lengths[i]);
		}else{
			lengths[i] = STRONGNESS_MAX;
			sprintf(logger->buffer, "%s - %d", logger->buffer, lengths[i]);
		}
	}
	lengths[dimension - 2] = MAX_NUM_MY_FLEETS;
	lengths[dimension - 1] = actions.size();

	sprintf(logger->buffer, "%s - %d", logger->buffer, lengths[dimension - 1]);
	logger->log();

	ReadQValues();
}

void Colony::QValue::WriteQValues(){
	snprintf(logger->buffer, 100, "%s.q", MAP_NAME_COLONY);
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

void Colony::QValue::ReadQValues(){
	snprintf(logger->buffer, 100, "%s.q", MAP_NAME_COLONY);
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
