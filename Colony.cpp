/*
 * Colony.cpp
 *
 *  Created on: Nov 17, 2014
 *      Author: sina
 */

#include "Colony.h"
#include <iostream>
#include <algorithm>
#include <math.h>
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

	srand(time(NULL));
}

Colony::~Colony() {

}

// The DoTurn function is where your code goes. The PlanetWars object contains
// the state of the game, including information about all planets and fleets
// that currently exist. Inside this function, you issue orders using the
// pw.IssueOrder() function. For example, to send 10 ships from planet 3 to
// planet 8, you would say pw.IssueOrder(3, 8, 10).
int Colony::DoTurn(const PlanetWars &pw, Colony *destination) {
	destination_ = destination;
	int total_ships = 0;

	influenceMap = makeInfluenceMap(pw);

	state = getState(pw);

	//USE LOOKAHEAD!
	shipRequestTable = new vector<int>();
	for (size_t i = 0; i < pw.NumPlanets(); i++){
		shipRequestTable->push_back(0);
	}
	shipAvailableTable = new vector<int>();
	for (size_t i = 0; i < pw.NumPlanets(); i++){
		shipAvailableTable->push_back(0);
	}

	//look at my current planet and update how many ships we can
	//actually send safely
	vector<int> *myPlanets = MyPlanets(pw);
	for(size_t i = 0; i < (*myPlanets).size(); i++) {
		const Planet* planet = pw.GetPlanet((*myPlanets)[i]);

		(*shipRequestTable)[planet->PlanetID()] -= planet->NumShips();
		(*shipAvailableTable)[planet->PlanetID()] += planet->NumShips();

		int time = 0; //beam search through only times of interest
		vector<int>* fleets = getIncomingFleets(pw, planet);
		vector<int>* fleets_sorted = getIncomingFleets(pw, planet);
		for (size_t g = 0; g < fleets->size(); g++){
			int min = (*fleets)[g];
			for (size_t r = g; r < fleets->size(); r++){
				if (pw.GetFleet((*fleets)[r])->TurnsRemaining() < pw.GetFleet(min)->TurnsRemaining()){
					min = (*fleets)[r];
				}
			}
			fleets_sorted->push_back(min);
		}
		fleets = fleets_sorted;

		for (size_t j = 0; j < fleets->size(); j++) {
			const Fleet* fleet = pw.GetFleet((*fleets)[j]);

			vector<int>* myFleets = MyFleets(pw);
			std::vector<int>::iterator it;
			it = find((*myFleets).begin(), (*myFleets).end(), (*fleets)[j]);

			if(it != (*myFleets).end()){ //if it is my fleet...then we need more ships to send as much
				(*shipRequestTable)[planet->PlanetID()] -= fleet->NumShips();
				continue;
			}

			//else we have to check out how much the enemy is sending
			int discreteTime = fleet->TurnsRemaining() - time;
			int planetGain = discreteTime * planet->GrowthRate() - fleet->NumShips();

			(*shipRequestTable)[planet->PlanetID()] -= planetGain;

			// we can only say that we have available ships if the enemy doesn't have enough to take the planet
			if (planetGain < 0) {
				(*shipAvailableTable)[planet->PlanetID()] += planetGain;
			}

			time = fleet->TurnsRemaining();
		}
	}

	//see how many we need to send to the enemy planet to take it
	vector<int> *enemyPlanets = EnemyPlanets(pw);
	for(size_t i = 0; i < enemyPlanets->size(); i++) {
		const Planet* planet = pw.GetPlanet((*enemyPlanets)[i]);

		(*shipRequestTable)[planet->PlanetID()] += planet->NumShips();

		int time = 0;

		vector<int>* fleets = getIncomingFleets(pw, planet);
		vector<int>* fleets_sorted = getIncomingFleets(pw, planet);
		for (size_t g = 0; g < fleets->size(); g++){
			int min = (*fleets)[g];
			for (size_t r = g; r < fleets->size(); r++){
				if (pw.GetFleet((*fleets)[r])->TurnsRemaining() < pw.GetFleet(min)->TurnsRemaining()){
					min = (*fleets)[r];
				}
			}
			fleets_sorted->push_back(min);
		}
		fleets = fleets_sorted;

		for (size_t j = 0; j < fleets->size(); j++) {
			const Fleet* fleet = pw.GetFleet((*fleets)[j]);

			vector<int>* enemyFleets = EnemyFleets(pw);
			std::vector<int>::iterator it;
			it = find((*enemyFleets).begin(), (*enemyFleets).end(), (*fleets)[j]);

			if(it != (*enemyFleets).end()){ //if it is my fleet...then we need more ships to send
				(*shipRequestTable)[planet->PlanetID()] += fleet->NumShips();
				continue;
			}

			int discreteTime = fleet->TurnsRemaining() - time;
			int planetGain = discreteTime * planet->GrowthRate() - fleet->NumShips(); //how many ships we need at that time

			(*shipRequestTable)[planet->PlanetID()] += planetGain;

			time = fleet->TurnsRemaining();
		}
	}

	//see what the neutral planets need
	vector<int> *neutralPlanets = NeutralPlanets(pw);
	for(size_t i = 0; i < neutralPlanets->size(); i++){
		const Planet* planet = pw.GetPlanet((*neutralPlanets)[i]);

		(*shipRequestTable)[planet->PlanetID()] += planet->NumShips();

		vector<int>* fleets = getIncomingFleets(pw, planet);
		vector<int>* myFleets = MyFleets(pw);
		for (size_t j = 0; j < fleets->size(); j++) {
			const Fleet* fleet = pw.GetFleet((*fleets)[j]);

			std::vector<int>::iterator it;
			it = find((*myFleets).begin(), (*myFleets).end(), (*fleets)[j]);

			if(it != (*myFleets).end()){
				(*shipRequestTable)[planet->PlanetID()] -= fleet->NumShips();
			}else{
				(*shipRequestTable)[planet->PlanetID()] += fleet->NumShips();
			}
		}
	}

	//find the average influence
	double avgInf = 0;
	double *influenceDeficit = new double[pw.NumPlanets()];
	for(size_t i = 0; i < myPlanets->size(); i++){
		const Planet* planet = pw.GetPlanet((*myPlanets)[i]);
		avgInf += (*influenceMap)[planet->PlanetID()];
	}
	avgInf /= myPlanets->size();

	for(size_t i = 0; i < myPlanets->size(); i++){
		const Planet* planet = pw.GetPlanet((*myPlanets)[i]);
		influenceDeficit[planet->PlanetID()] = avgInf - (*influenceMap)[planet->PlanetID()];
	}

	//find the minimum distances
	int *minimumDistance = new int[pw.NumPlanets()];

	for(size_t i = 0; i < myPlanets->size(); i++){
		const Planet* mine = pw.GetPlanet((*myPlanets)[i]);
		int minDist = MAX_VALUE;
		for(size_t i = 0; i < enemyPlanets->size(); i++){
			const Planet* e = pw.GetPlanet((*enemyPlanets)[i]);
			int dist = pw.Distance(mine->PlanetID(), e->PlanetID());
			minDist = minDist < dist ? minDist : dist;
		}

		minimumDistance[mine->PlanetID()] = minDist;
	}

	int maxEnemyPlanetSize = 0;
	for(size_t i = 0; i < enemyPlanets->size(); i++){
		const Planet* planet = pw.GetPlanet((*enemyPlanets)[i]);
		int temp = planet->NumShips();
		maxEnemyPlanetSize = maxEnemyPlanetSize > temp ? maxEnemyPlanetSize : temp;
	}

	vector<int> * combinedPlanets = PlanetsCombined(pw);
	for(size_t i = 0; i < myPlanets->size(); i++){
		const Planet* source = pw.GetPlanet((*myPlanets)[i]);
		// midgame to keep things interesting
		if(pw.Turn() > 100 && source->NumShips() > maxEnemyPlanetSize * 2 + 1){
			//if we have a planet greater than all other planets by 2x
			int t = MAX_VALUE;
			int d = -1;
			for(size_t j = 0; j < enemyPlanets->size(); j++){
				const Planet* dest = pw.GetPlanet((*enemyPlanets)[j]);
				if(t > pw.Distance(dest->PlanetID(), source->PlanetID())){
					t = pw.Distance(dest->PlanetID(), source->PlanetID());
					d = dest->PlanetID();
				}
			}
			if(d != -1){
				pw.IssueOrder(source->PlanetID(), d, maxEnemyPlanetSize + 1);
				total_ships += maxEnemyPlanetSize + 1;
				continue;
			}
		}
		int bestDest = -1;
		double bestHeuristic = 0;
		int bestNumShips = 0;
		for(size_t j = 0; j < combinedPlanets->size(); j++){
			const Planet* dest = pw.GetPlanet((*combinedPlanets)[j]);
			if(dest->PlanetID() == source->PlanetID()){
				continue;
			}

			int numShips;
			double heuristic;

			switch (state) {
			case CAREFUL:
				heuristic = getCarefulHeuristics(pw, source, dest);
				numShips = getLeastShipsNeeded(pw, source, dest, 1, 3);
				state = getState(pw);
				break;
			default:
				heuristic = getHeuristics(dest, 0);
				numShips = getLeastShipsNeeded(pw, source, dest, 1, calcConfidence(pw, source));
			}
			if(numShips > 0 && heuristic > bestHeuristic){ //this means we should send ships there
				bestDest = dest->PlanetID();
				bestHeuristic = heuristic;
				bestNumShips = numShips;
			}
		}
		if(bestDest != -1){
			pw.IssueOrder(source->PlanetID(), bestDest, bestNumShips);
			(*shipRequestTable)[source->PlanetID()] += bestNumShips;
			(*shipAvailableTable)[bestDest] -= bestNumShips;
			total_ships += bestNumShips;
		} else {
			//if we did not issue any orders...kick in the influence balancing ai
			bool underAttack = false;

			vector<int>* enemyFleets = EnemyFleets(pw);
			for(size_t j = 0; j < enemyFleets->size(); j++){
				const Fleet* fleet = pw.GetFleet((*enemyFleets)[j]);
				if(fleet->DestinationPlanet() == source->PlanetID()){
					underAttack = true;
					break;
				}
			}
			if(underAttack){
				continue;
			}
			int deficit = - (int)influenceDeficit[source->PlanetID()];
			if(deficit <= 0 || (*shipAvailableTable)[source->PlanetID()] < 1){
				continue;
			}
			int numShips = deficit < ((*shipAvailableTable)[source->PlanetID()] - 1) ? deficit : ((*shipAvailableTable)[source->PlanetID()] - 1);
			if(numShips >= source->NumShips()){
				continue;
			}
			double minDist = MAX_VALUE;
			int maxInd = -1;

			for(size_t k = 0; k < myPlanets->size(); k++){
				const Planet* planet = pw.GetPlanet((*myPlanets)[k]);
				int currind = planet->PlanetID();
				if(minimumDistance[currind] < minDist && planet->PlanetID() != source->PlanetID()){
					minDist = minimumDistance[currind];
					maxInd = currind;
				}
			}
			if(maxInd > -1){
				pw.IssueOrder(source->PlanetID(), maxInd, numShips);
				influenceDeficit[maxInd] = 0;
				total_ships += numShips;
			}
		}
	}

	return total_ships;
}

int Colony::calcConfidence(const PlanetWars &pw, const Planet *planet){
	//find nearest enemy planet
	int nearest = MAX_VALUE;

	vector<int>* enemyPlanets = EnemyPlanets(pw);
	for(size_t i = 0; i < enemyPlanets->size(); i++){
		const Planet* e = pw.GetPlanet((*enemyPlanets)[i]);
		int temp = pw.Distance(planet->PlanetID(), e->PlanetID());
		nearest = nearest < temp ? nearest : temp;
	}

	if(nearest < 15){
		return 3;
	} else{
		return 2;
	}
}

double Colony::getHeuristics(int planet, int growthrate, int numships, int minInfluence){
	double numshipslog = (numships == 0) ? 0 : log((double) numships) + .1;
	return growthrate * (*influenceMap)[planet] / numships;

}

double Colony::getHeuristics(const Planet *planet, int minInfluence){
	return getHeuristics(planet->PlanetID(), planet->GrowthRate(), planet->NumShips(), minInfluence);
}

/**getLeastShipsNeeded
 * this method calculates the least number of ships I need to take over a planet
 * or at least "confidently" take over a planet
 */
int Colony::getLeastShipsNeeded(const PlanetWars &pw, const Planet* source, const Planet* dest, int div, int confidence){
	int sourceIndex = source->PlanetID();
	int destIndex = dest->PlanetID();

	int shipsHave = (int)((*shipAvailableTable)[sourceIndex]);

	vector<int>* neutralPlanets = NeutralPlanets(pw);

	std::vector<int>::iterator it;
	it = find((*neutralPlanets).begin(), (*neutralPlanets).end(), dest->PlanetID());

	int add = (it != (*neutralPlanets).end()) ? 0 : dest->GrowthRate();

	int leastShips = (int)((((double)((*shipRequestTable)[destIndex]))/div) + add);

	int min_temp = shipsHave < (source->NumShips() - 1) ? shipsHave : (source->NumShips() - 1);
	if(leastShips >= min_temp //we don't have enough ships
			|| shipsHave - leastShips < source->GrowthRate() * confidence){ //unwise
		return -1;
	}
	else {
		return leastShips + 1;
	}
}

double Colony::getCarefulHeuristics(const PlanetWars &pw, int source, int dest, int planetNumShips){
	return (double) pw.Distance(source, dest) / planetNumShips;
}

double Colony::getCarefulHeuristics(const PlanetWars &pw, const Planet *source, const Planet *dest){
	vector<int>* enemyPlanets = EnemyPlanets(pw);

	std::vector<int>::iterator it;
	it = find((*enemyPlanets).begin(), (*enemyPlanets).end(), dest->PlanetID());

	if(it != (*enemyPlanets).end()){
		return -1;
	}

	return getCarefulHeuristics(pw, source->PlanetID(), dest->PlanetID(),  dest->NumShips());
}

vector<int>* Colony::getIncomingFleets(const PlanetWars &pw, const Planet* planet){
	vector<int>* ret = new vector<int>();

	for(size_t i = 0; i < ret->size(); i++){
		const Fleet* fleet = pw.GetFleet(i);
		if(fleet->DestinationPlanet() == planet->PlanetID()){
			ret->push_back(i);
		}
	}

	return ret;
}

STATE Colony::getState(const PlanetWars &pw){
	if(MyPlanets(pw)->size() == 1){
		const Planet* myPlanet = pw.GetPlanet((*MyPlanets(pw))[0]);
		const Planet* enemyPlanet;
		if(EnemyPlanets(pw)->size() != 0){
			enemyPlanet = pw.GetPlanet((*EnemyPlanets(pw))[0]);
		} else {
			return DEFAULT;
		}
		int dist = pw.Distance(myPlanet->PlanetID(), enemyPlanet->PlanetID());

		if(enemyPlanet->NumShips() / 1.5 > myPlanet->GrowthRate() * dist){
			return CAREFUL;
		} else {
			return DEFAULT;
		}

	}
	else {
		return DEFAULT;
	}
}

vector<double>* Colony::makeInfluenceMap(const PlanetWars &pw){
	vector<double>* ret = new vector<double>();

	for (size_t i = 0; i < pw.NumPlanets(); i++){
		ret->push_back(0);
	}

	vector<int> *planetsArr = PlanetsCombined(pw);

	for(size_t i = 0; i < planetsArr->size(); i++) {
		const Planet* to = pw.GetPlanet((*planetsArr)[i]);
		int index = to->PlanetID();

		//my planet influence
		vector<int>* planets = MyPlanets(pw);
		for(size_t j = 0; j < planets->size(); j++) {
			const Planet* from = pw.GetPlanet((*planets)[j]);

			double dist = (double) pw.Distance(to->PlanetID(), from->PlanetID());
			double expDist = exp(dist / 5);
			double influence = (double) from->NumShips();

			(*ret)[index] += influence / expDist;
		}

		//enemy planet influence
		planets = EnemyPlanets(pw);
		for(size_t j = 0; j < planets->size(); j++) {
			const Planet* from = pw.GetPlanet((*planets)[j]);

			double dist = (double) pw.Distance(to->PlanetID(), from->PlanetID());
			double expDist = exp(dist / 5);
			double influence = (double) from->NumShips();

			(*ret)[index] -= influence / expDist;
		}
	}

	return ret;
}

vector<int>* Colony::EnemyPlanets(const PlanetWars &pw){
	vector<int>* ret = new vector<int>();
	for (size_t i = 0; i < Size(); i++){
		if (pw.GetPlanet(planets[i])->Owner() == ENEMY){
			const Planet* planet = pw.GetPlanet(planets[i]);
			ret->push_back(planet->PlanetID());
		}
	}
	for (size_t i = 0; i < destination_->Size(); i++){
		if (pw.GetPlanet(destination_->Planets()[i])->Owner() == ENEMY){
			const Planet* planet = pw.GetPlanet(destination_->Planets()[i]);
			ret->push_back(planet->PlanetID());
		}
	}
	return ret;
}

vector<int>* Colony::PlanetsCombined(const PlanetWars &pw){
	vector<int>* ret = new vector<int>();
	for (size_t i = 0; i < Size(); i++){
		const Planet* planet = pw.GetPlanet(planets[i]);
		ret->push_back(planet->PlanetID());
	}
	for (size_t i = 0; i < destination_->Size(); i++){
		const Planet* planet = pw.GetPlanet(destination_->Planets()[i]);
		ret->push_back(planet->PlanetID());
	}
	return ret;
}

vector<int>* Colony::MyPlanets(const PlanetWars &pw){
	vector<int>* ret = new vector<int>();
	for (size_t i = 0; i < Size(); i++){
		if (pw.GetPlanet(planets[i])->Owner() == ME){
			const Planet* planet = pw.GetPlanet(planets[i]);
			ret->push_back(planet->PlanetID());
		}
	}
	for (size_t i = 0; i < destination_->Size(); i++){
		if (pw.GetPlanet(destination_->Planets()[i])->Owner() == ME){
			const Planet* planet = pw.GetPlanet(destination_->Planets()[i]);
			ret->push_back(planet->PlanetID());
		}
	}
	return ret;
}

vector<int>* Colony::NeutralPlanets(const PlanetWars &pw){
	vector<int>* ret = new vector<int>();
	for (size_t i = 0; i < Size(); i++){
		if (pw.GetPlanet(planets[i])->Owner() == NEUTRAL){
			const Planet* planet = pw.GetPlanet(planets[i]);
			ret->push_back(planet->PlanetID());
		}
	}
	for (size_t i = 0; i < destination_->Size(); i++){
		if (pw.GetPlanet(destination_->Planets()[i])->Owner() == NEUTRAL){
			const Planet* planet = pw.GetPlanet(destination_->Planets()[i]);
			ret->push_back(planet->PlanetID());
		}
	}
	return ret;
}

vector<int>* Colony::MyFleets(const PlanetWars &pw){
	vector<int>* ret = new vector<int>();
	for (size_t i = 0; i < pw.NumFleets(); i++){
		const Fleet* fleet = pw.GetFleet(i);
		if (IfPlanetHere(pw, fleet->SourcePlanet()) || IfPlanetHere(pw, fleet->DestinationPlanet())
				|| destination_->IfPlanetHere(pw, fleet->SourcePlanet()) || destination_->IfPlanetHere(pw, fleet->DestinationPlanet())){
			if (fleet->Owner() == ME){
				ret->push_back(i);
			}
		}
	}
	return ret;
}

vector<int>* Colony::EnemyFleets(const PlanetWars &pw){
	vector<int>* ret = new vector<int>();
	for (size_t i = 0; i < pw.NumFleets(); i++){
		const Fleet* fleet = pw.GetFleet(i);
		if (IfPlanetHere(pw, fleet->SourcePlanet()) || IfPlanetHere(pw, fleet->DestinationPlanet())
				|| destination_->IfPlanetHere(pw, fleet->SourcePlanet()) || destination_->IfPlanetHere(pw, fleet->DestinationPlanet())){
			if (fleet->Owner() == ENEMY){
				ret->push_back(i);
			}
		}
	}
	return ret;
}

vector<int>* Colony::DecideNumShips(const PlanetWars &pw, vector<int> &sources, int dest){
	vector<int>* nums = new vector<int>();
	for (size_t i = 0; i < sources.size(); i++){
		nums->push_back(pw.GetPlanet(sources[i])->NumShips() / 2);
	}
	return nums;
}

bool Colony::addPlanet(Planet *planet, const PlanetWars &pw){
	map<int, int> &planetToColony = *pw.PlanetColony();

	planets[size] = planet->PlanetID();
	planetToColony[planet->PlanetID()] = ID();
	size++;

	return true;
}

bool Colony::HasFriendlyPlanet(const PlanetWars &pw){
	for (size_t i = 0; i < size; i++){
		if (pw.GetPlanet(planets[i])->Owner() == ME){
			return true;
		}
	}
	return false;
}

void Colony::Initialize(const PlanetWars &pw){
	//For each planet in the colony we try to attack
	for (size_t i = 0; i < COLONY_MAX_SIZE; i++){
		actions.push_back(new Action(i));
	}
	//NO-OP Action
	actions.push_back(new Action(-1));
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

double Colony::Reward(const PlanetWars &pw, int action_t){
	return 0;
}

void Colony::SetEligable(int index, bool elig){
	eligable[index] = elig;
}
