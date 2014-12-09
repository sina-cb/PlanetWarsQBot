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
bool Colony::DoTurn(const PlanetWars &pw, Colony *destination) {
	sprintf(logger->buffer, "START TURN:");
	logger->log();

	sprintf(logger->buffer, "Source Colony: %d , Destination Colony: %d", ID(), destination->ID());
	logger->log();

	// (1) If there is no Enemy planet left, just help your own planets!
	if (!pw.IsAlivePlanets(ENEMY)){
		int source = pw.MyPlanets()[0]->PlanetID();
		int dest = pw.MyPlanets()[1]->PlanetID();
		pw.IssueOrder(source, dest, pw.GetPlanet(source)->NumShips() / 2);
		return true;
	}

	// (2) Find my eligible planets to attack.
	vector<int> sources;
	for (size_t i = 0; i < size; i++){
		if (eligable[i]){
			sources.push_back(pw.GetPlanet(planets[i])->PlanetID());
		}
	}

	if (sources.size() == 0){
		sprintf(logger->buffer, "END TURN (No-Eligible)\n");
		logger->log();
		return true;
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
				ships_num = PLANET_STRONGNESS_MAX - 1;
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

	// (4) Find the appropriate action
	bool random_action = (((rand() % 1000) / 1000.0) > EXPLOITATION_PLANET) ? true : false;

	// (4.1) If one the players are dead, then it's better not to act randomly!
	if (!pw.IsAlivePlanets(ME) || !pw.IsAlive(ME)){
		random_action = false;
	}

	double max_q = -99999999;
	Action* max_action = 0;
	int action_t = -1;
	int dest = -1;

	//Whether we need to act randomly or based on our Q-values
	if (random_action){
		sprintf(logger->buffer, "Exploitation Step!");
		logger->log();

		while (true){
			action_t = rand() % q_value_obj->actions.size();
			max_action = q_value_obj->actions[action_t];

			//No-Op
			if (max_action->destination == -1){
				sprintf(logger->buffer, "END TURN (NO-OP)\n");
				logger->log();
				return false;
			}

			if (destination->Planets()[max_action->destination] != -1){
				break;
			}

			sprintf(logger->buffer, "While True :: Colony");
			logger->log();
		}
	}else{
		for (size_t i = 0; i < q_value_obj->actions.size(); i++){
			vector<int> indexes;
			for (size_t j = 0; j < planet_strongness.size(); j++){
				indexes.push_back(planet_strongness[j]);
			}
			indexes.push_back(i);

			if (destination->Planets()[q_value_obj->actions[i]->destination] != -1 && q_value_obj->q_values[get_index_for(indexes)] > max_q){
				max_q = q_value_obj->q_values[get_index_for(indexes)];
				max_action = q_value_obj->actions[i];
				action_t = i;
				dest = destination->Planets()[max_action->destination];
			}
		}

		if (action_t == -1){
			sprintf(logger->buffer, "END TURN (No Destination Planet)\n");
			logger->log();
			return false;
		}
	}

	// (5) Decide about how many ships we have to send from each planet
	vector<int> *num_ships = DecideNumShips(pw, sources, dest, max_action);

	// (6) Issue Orders!!!
	if (sources.size() > 0 && dest != -1){
		for (size_t i = 0; i < sources.size(); i++){
			pw.IssueOrder(sources[i], dest, num_ships->data()[i]);
			sprintf(logger->buffer, "Attack %d: Source: %d, Dest.: %d Ships: %d", (i + 1), sources[i], dest, num_ships->data()[i]);
			logger->log();
		}
	}

	// (7) Q-Learning Step Starts Here!

	// (7.1) Estimate next state for the planets!
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
				ships_num = PLANET_STRONGNESS_MAX - 1;
			}

			ships_num = (ships_num - PLANET_STRONGNESS_MIN) / PLANET_STRONGNESS_STEP;
			planet_strongness_new.push_back(ships_num);
		}
	}

	/// (7.2) Calculate and update new Q-Values!
	CalculatedNewQValue(pw, planet_strongness, planet_strongness_new, action_t);

	sprintf(logger->buffer, "END TURN:\n");
	logger->log();

	return true;
}

vector<int>* Colony::DecideNumShips(const PlanetWars &pw, vector<int> &sources, int dest, Action* selected_action){
	vector<int>* nums = new vector<int>();
	for (size_t i = 0; i < sources.size(); i++){
		nums->push_back(pw.GetPlanet(sources[i])->NumShips() / 2);
	}
	return nums;
}

void Colony::CalculatedNewQValue(const PlanetWars &pw, vector<int> &old_strongness, vector<int> &new_strongness, int action_t){
	sprintf(logger->buffer, "CAL NEW Q :: START");
	logger->log();

	// Create previous state
	old_strongness.push_back(action_t);

	for (size_t i = 0; i < old_strongness.size(); i++){
		sprintf(logger->buffer, "old state #%d: %d", i, old_strongness[i]);
		logger->log();
	}

	/// Find the action which makes next state's Q-value to be MAX
	double new_q_max = -99999999;
	for (size_t i = 0; i < q_value_obj->actions.size(); i++){
		vector<int> indexes;
		for (size_t j = 0; j < new_strongness.size(); j++){
			indexes.push_back(new_strongness[j]);
		}
		indexes.push_back(i);

		int index = get_index_for(indexes);
		if (q_value_obj->q_values[index] > new_q_max){
			new_q_max = q_value_obj->q_values[index];
		}
	}

	int old_index = get_index_for(old_strongness);

	sprintf(logger->buffer, "new max Q: %f  |  old_index: %d  |  q_value_size: %d  |  old_q: %f", new_q_max, old_index, q_value_obj->num_q_values, q_value_obj->q_values[old_index]);
	logger->log();

	double new_q = (1 - ALPHA_PLANET_Q) * q_value_obj->q_values[old_index];
	new_q += ((ALPHA_PLANET_Q) * (Reward(pw, action_t) + DISCOUNT_PLANET * new_q_max));

	q_value_obj->q_values[old_index] = new_q;
	sprintf(logger->buffer, "CAL NEW Q :: END");
	logger->log();
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

	if (total == 0){
		return 0;
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
		}else if (planet->Owner() == ME){
			eligable[i] = true;
		}else{
			eligable[i] = false;
		}
	}

	//Discretization of strongness parameter
	if (strongness < MIN_STRONGNESS){
		strongness = MIN_STRONGNESS;
	}else if (strongness >= MAX_STRONGNESS){
		strongness = MAX_STRONGNESS - 1;
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
		strongness_next_state = MAX_STRONGNESS - 1;
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
			lengths[i] = ((PLANET_STRONGNESS_MAX - PLANET_STRONGNESS_MIN) / PLANET_STRONGNESS_STEP);
			sprintf(logger->buffer, "%s%d", logger->buffer, lengths[i]);
		}else{
			lengths[i] = ((PLANET_STRONGNESS_MAX - PLANET_STRONGNESS_MIN) / PLANET_STRONGNESS_STEP);
			sprintf(logger->buffer, "%s - %d", logger->buffer, lengths[i]);
		}
	}
	lengths[dimension - 1] = actions.size();

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

	sprintf(logger->buffer, "Child Process:\nZeros Count: %d", zero_count);
	logger->log();
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
			index++;
		}
		in.close();

		sprintf(logger->buffer, "Q-Values loaded");
		logger->log();
	}
}
