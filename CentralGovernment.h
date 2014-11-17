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
#include <boost/thread.hpp>
#include <vector>

class CentralGovernment {
public:
	CentralGovernment();
	virtual ~CentralGovernment();
	void DoTurn(const PlanetWars &pw);

private:
	std::vector<Colony*> colonies;

	void UpdateColonies(const PlanetWars &pw);
	void HandleColonies(const PlanetWars &pw);
};

#endif /* CENTRALGOVERNMENT_H_ */
