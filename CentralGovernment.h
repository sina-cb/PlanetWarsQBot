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

private:
	Logger *logger;
	std::vector<Colony*> colonies;

	/**
	 * Update Colony Parameters at the beginning of each Turn.
	 */
	void UpdateColonies(const PlanetWars &pw);

	/**
	 * This function is used to do Q-learning at the Colony Level
	 * This should select a Colony to attack and call the DoTurn function
	 * in each Colony.
	 */
	void HandleColonies(const PlanetWars &pw);
};

#endif /* CENTRALGOVERNMENT_H_ */
