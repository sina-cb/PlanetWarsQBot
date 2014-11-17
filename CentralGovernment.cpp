/*
 * CentralGovernment.cpp
 *
 *  Created on: Nov 17, 2014
 *      Author: sina
 */

#include "CentralGovernment.h"

using namespace std;

CentralGovernment::CentralGovernment() {
	colonies.push_back(new Colony(colonies.size()));
}

CentralGovernment::~CentralGovernment() {

}

void CentralGovernment::DoTurn(const PlanetWars &pw){
	UpdateColonies(pw);
	HandleColonies(pw);
}

void CentralGovernment::HandleColonies(const PlanetWars &pw){
	boost::thread first_worker(&Colony::DoTurn, colonies[0], pw);

	std::cout << "Start Turn!" << std::endl;
	first_worker.join();
	std::cout << "End Turn!" << std::endl;
}

void CentralGovernment::UpdateColonies(const PlanetWars &pw){
	for (unsigned int i = 0; i < colonies.size(); i++){
		if (colonies[i]->IfNeedUpdate()){

			vector<Planet> no_mine = pw.NotMyPlanets();
			for (unsigned int j = 0; j < no_mine.size() / 4; j++){
				colonies[i]->addNeighbor(&no_mine[i]);
			}
			colonies[i]->DoNotNeedUpdate();
		}
	}
}
