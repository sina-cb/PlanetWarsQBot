/*
 * Colony.h
 *
 *  Created on: Nov 17, 2014
 *      Author: sina
 */

#ifndef COLONY_H_
#define COLONY_H_

#include "PlanetWars.h"
#include <vector>
#include <sys/stat.h>
#include "Logger.cpp"
#include "Constants.h"

class Colony {
public:
	class QValue;
	class Action;

	Colony();
	Colony(int id);
	virtual ~Colony();

	/**
	 * Runs Q-Learning at the Planet Level, to find the destination planet to attack
	 */
	bool DoTurn(const PlanetWars &pw, Colony* destination);

	/**
	 * This function should be run at the beginning of each turn to update the colony
	 * parameters.
	 */
	void UpdateColony(const PlanetWars &pw);
	void UpdateNextStateColony(const PlanetWars &pw);

	bool addPlanet(Planet *planet, const PlanetWars &pw);
	bool IfPlanetHere(const PlanetWars &pw, int planetID);
	bool removePlanet(Planet *planet);

	int ID();
	int Size();
	int Strongness();
	int StrongnessEstimation();
	bool HasFriendlyPlanet(const PlanetWars &pw);

	int* Planets() {return planets;}
	void QValueObj(QValue *q_value_obj){this->q_value_obj = q_value_obj;}
	QValue* QValueObj(){return q_value_obj;}

	double Reward(const PlanetWars &pw, int action_t);
	void CalculatedNewQValue(const PlanetWars &pw, vector<int> &old_strongness, vector<int> &new_strongness, int action_t, int num_fleets);

	class QValue{
	public:
		QValue(){
			logger = new Logger("Colonies.log");
		}

		double* q_values;
		int num_q_values;

		int dimension;
		int* lengths;

		std::vector<Action*> actions;

		void Initialize(const PlanetWars &pw);
		void ReadQValues();
		void WriteQValues();

		Logger *logger;

		inline bool check_file_exists (const std::string& name) {
			struct stat buffer;
			return (stat (name.c_str(), &buffer) == 0);
		}
	};

	class Action{
	public:
		Action(int destication){
			this->destination = destication;
		}

		int destination;
	};

private:
	QValue *q_value_obj;

	int strongness;
	int strongness_next_state;
	int attackThreshold;

	int id;
	int planets[COLONY_MAX_SIZE];
	bool eligable[COLONY_MAX_SIZE];
	int size;
	std::vector<Fleet*> fleets;

	Logger *logger;

	inline int get_index_for(std::vector<int> indexes){
		int temp = 1;
		int result = 0;
		for (size_t i = 1; i < q_value_obj->dimension; i++){
			temp *= q_value_obj->lengths[i];
		}

		for (size_t i = 0; i < q_value_obj->dimension; i++){
			result += indexes[i] * temp;
			if (i + 1 < q_value_obj->dimension){
				temp /= q_value_obj->lengths[i + 1];
			}
		}

		return result;
	}
};

#endif /* COLONY_H_ */
