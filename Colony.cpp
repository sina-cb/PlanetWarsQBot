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
}

Colony::~Colony() {

}

// The DoTurn function is where your code goes. The PlanetWars object contains
// the state of the game, including information about all planets and fleets
// that currently exist. Inside this function, you issue orders using the
// pw.IssueOrder() function. For example, to send 10 ships from planet 3 to
// planet 8, you would say pw.IssueOrder(3, 8, 10).
void Colony::DoTurn(const PlanetWars &pw, Colony *destination) {
	sprintf(logger->buffer, "TURN START: %d ", ID());
	logger->log();

	int num_my_fleets = pw.MyFleets().size();

	// (1) If we currently have a fleet in flight, just do nothing.
	if (num_my_fleets >= MAX_NUM_MY_FLEETS) {
		return;
	}

	// (2) Find my eligible planets to attack.
	vector<int> sources;
	for (size_t i = 0; i < size; i++){
		const Planet* planet = pw.GetPlanet(planets[i]);
		if (planet->Owner() == ME){
			if (planet->NumShips() > ELIGIBILITY_THRESHOLD_PERCENT)
				sources.push_back(planets[i]);
		}
	}

	// (3) Find the weakest enemy or neutral planet.
	int dest = -1;
	for (size_t i = 0; i < destination->Size(); i++){
		if (pw.GetPlanet(destination->Planets()[i])->Owner() == ENEMY ||
				pw.GetPlanet(destination->Planets()[i])->Owner() == NEUTRAL){
			dest = destination->Planets()[i];
			break;
		}
	}

	if (sources.size() > 0 && dest != -1){
		// (4) Send half the ships from my strongest planet to the weakest
		// planet that I do not own.
		for (size_t i = 0; i < sources.size(); i++){
			int num_ships = pw.GetPlanet(sources[i])->NumShips() / 2;
			pw.IssueOrder(sources[i], dest, num_ships);
			sprintf(logger->buffer, "Attack %d: Source: %d, Dest.: %d Ships: %d", (i + 1), sources[i], dest, num_ships);
			logger->log();
		}
	}

	//**************************************************//

	/*// (2) Find my eligible planets to attack.
	vector<int> sources;
	for (size_t i = 0; i < size; i++){
		const Planet* planet = pw.GetPlanet(planets[i]);
		if (planet->Owner() == ME){
			if (planet->NumShips() > ELIGIBILITY_THRESHOLD_PERCENT)
				sources.push_back(planets[i]);
		}
	}

	// (3) Discretize strongness of each planet in the colony
	vector<int> planet_strongness;

	for (size_t i = 0; i < COLONY_MAX_SIZE; i++){
		if (i > destination->Size()){
			planet_strongness.push_back(0);
		}else{
			int ships_num = pw.GetPlanet(destination->Planets()[i])->NumShips();
			if (ships_num < PLANET_STRONGNESS_MIN){
				ships_num = PLANET_STRONGNESS_MIN;
			}else if (ships_num >= PLANET_STRONGNESS_MAX){
				ships_num = PLANET_STRONGNESS_MAX;
			}

			ships_num = (ships_num - PLANET_STRONGNESS_MIN) / PLANET_STRONGNESS_STEP;
			planet_strongness.push_back(ships_num);
		}
	}

	bool random_action = (((rand() % 1000) / 1000.0) > EXPLOITATION_PLANET) ? true : false;
	double max_q = -99999999;
	Action* max_action = 0;
	int action_t = -1;

	if (random_action){
		while (true){
			action_t = rand() % q_value_obj->actions.size();
			max_action = q_value_obj->actions[action_t];

			if (max_action->destination == -1){
				return;
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

			if (planets[q_value_obj->actions[i]->destination] != -1 && q_value_obj->q_values[get_index_for(indexes)] > max_q){
				max_q = q_value_obj->q_values[get_index_for(indexes)];
				max_action = q_value_obj->actions[i];
				action_t = i;
			}
		}

		if (max_q == -99999999){
			return;
		}
	}

	if (sources.size() > 0){
		// (4) Send half the ships from my strongest planet to the weakest
		// planet that I do not own.
		for (size_t i = 0; i < sources.size(); i++){
			int num_ships = pw.GetPlanet(sources[i])->NumShips() / 2;
			pw.IssueOrder(sources[i], max_action->destination, num_ships);
			sprintf(logger->buffer, "Attack %d: Source: %d, Dest.: %d Ships: %d", (i + 1), sources[i], max_action->destination, num_ships);
			logger->log();
		}
	}*/

	//**************************************************//

	sprintf(logger->buffer, "TURN END: %d ", ID());
	logger->log();
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
		const Planet *planet = pw.GetPlanet(planets[i]);
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
	lengths[dimension - 1] = MAX_NUM_MY_FLEETS;
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
