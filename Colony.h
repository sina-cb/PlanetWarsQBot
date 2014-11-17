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

class Colony {
public:
	Colony();
	Colony(int id);
	virtual ~Colony();
	void DoTurn(const PlanetWars &pw);

	bool IfNeedUpdate();
	void NeedUpdate();
	void DoNotNeedUpdate();

	void addNeighbor(Planet *planet);

private:
	int id;
	std::vector<Planet*> planets;
	std::vector<Planet*> neighbors;
	std::vector<Fleet*> fleets;
	bool changed;
};

#endif /* COLONY_H_ */
