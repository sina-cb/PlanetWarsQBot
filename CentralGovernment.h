/*
 * CentralGovernment.h
 *
 *  Created on: Nov 17, 2014
 *      Author: sina
 */

#ifndef CENTRALGOVERNMENT_H_
#define CENTRALGOVERNMENT_H_

#include "PlanetWars.h"
#include "Colony.h"
#include "Logger.cpp"
#include <boost/thread.hpp>
#include <vector>
#include <sys/stat.h>
#include "Constants.h"

class CentralGovernment {

public:
	CentralGovernment();
	virtual ~CentralGovernment();

	/**
	 * This function is called in the QBot Game Loop.
	 * This basically updates each Colony, run Q-Learning at the Colony Level
	 * and execute DoTurn of the selected Colony.
	 */
	void DoTurn(const PlanetWars &pw);

	/**
	 * At the beginning of the execution, it determines all the static Colonies
	 */
	void InitializeColonies(const PlanetWars &pw);

	bool WriteOnce(){return writeOnce;}

private:
	class Action;

	Logger *logger;
	std::vector<Colony*> colonies;
	std::vector<Action*> actions;

	bool save_q_values;

	/**
	 * Update Colony Parameters at the beginning of each Turn.
	 */
	void UpdateColonies(const PlanetWars &pw);
	void EstimateNextState(const PlanetWars &pw);

	void CalculatedNewQValue(const PlanetWars &pw, int action_t);
	void ReadQValues();
	void WriteQValues();

	/**
	 * This function is used to do Q-learning at the Colony Level
	 * This should select a Colony to attack and call the DoTurn function
	 * in each Colony.
	 */
	void HandleColonies(const PlanetWars &pw);

	double* q_values;
	int num_q_values;

	int dimension;
	int* lengths;
	bool writeOnce;

	double Reward(const PlanetWars &pw, Action *action);

	class Action{
	public:
		Action(int source, int destication, int ID){
			this->source = source;
			this->destination = destication;
			this->ID = ID;
		}

		int source;
		int destination;
		int ID;
	};

	inline int get_index_for(std::vector<int> indexes){
		int temp = 1;
		int result = 0;
		for (size_t i = 1; i < dimension; i++){
			temp *= lengths[i];
		}

		for (size_t i = 0; i < dimension; i++){
			result += indexes[i] * temp;
			if (i + 1 < dimension){
				temp /= lengths[i + 1];
			}
		}

		return result;
	}

	inline bool check_file_exists (const std::string& name) {
	  struct stat buffer;
	  return (stat (name.c_str(), &buffer) == 0);
	}
};

#endif /* CENTRALGOVERNMENT_H_ */
