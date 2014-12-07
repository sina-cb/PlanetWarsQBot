#include "PlanetWars.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <assert.h>

using namespace std;

void StringUtil::Tokenize(const std::string& s, const std::string& delimiters,
		std::vector<std::string>& tokens) {
	std::string::size_type lastPos = s.find_first_not_of(delimiters, 0);
	std::string::size_type pos = s.find_first_of(delimiters, lastPos);
	while (std::string::npos != pos || std::string::npos != lastPos) {
		tokens.push_back(s.substr(lastPos, pos - lastPos));
		lastPos = s.find_first_not_of(delimiters, pos);
		pos = s.find_first_of(delimiters, lastPos);
	}
}

std::vector<std::string> StringUtil::Tokenize(const std::string& s,
		const std::string& delimiters) {
	std::vector<std::string> tokens;
	Tokenize(s, delimiters, tokens);
	return tokens;
}

Fleet::Fleet()
: owner_(ME),
  num_ships_(0),
  source_planet_(NULL),
  destination_planet_(NULL),
  total_trip_length_(-1),
  turns_remaining_(-1) {
}

Fleet::Fleet(int owner, int num_ships, int source_planet,
		int destination_planet, int total_trip_length, int turns_remaining) {
	owner_ = owner;
	num_ships_ = num_ships;
	source_planet_ = source_planet;
	destination_planet_ = destination_planet;
	total_trip_length_ = total_trip_length;
	turns_remaining_ = turns_remaining;
}

std::string Fleet::ToMoveOrder() const {
	assert(source_planet_ != NULL);
	assert(destination_planet_ != NULL);

	std::stringstream moveOrder;
	moveOrder << source_planet_ << " " << destination_planet_ << " " << num_ships_;
	return moveOrder.str();
}

Planet::Planet(int planet_id, int owner, int num_ships, int growth_rate,
		double x, double y) {
	planet_id_ = planet_id;
	owner_ = owner;
	num_ships_ = num_ships;
	growth_rate_ = growth_rate;
	x_ = x;
	y_ = y;
}

Planet::Planet(const Planet& other):
		planet_id_(other.planet_id_), owner_(other.owner_), num_ships_(other.num_ships_), growth_rate_(other.growth_rate_),
		x_(other.x_), y_(other.y_)
{}

int Planet::PlanetID() const {
	return planet_id_;
}

int Planet::Owner() const {
	return owner_;
}

int Planet::NumShips() const {
	return num_ships_;
}

int Planet::GrowthRate() const {
	return growth_rate_;
}

double Planet::X() const {
	return x_;
}

double Planet::Y() const {
	return y_;
}

void Planet::Owner(int new_owner) {
	owner_ = new_owner;
}

void Planet::NumShips(int new_num_ships) {
	num_ships_ = new_num_ships;
}

void Planet::AddShips(int amount) {
	num_ships_ += amount;
}

void Planet::RemoveShips(int amount) {
	num_ships_ -= amount;
}

// ######### PlanetWars #########

PlanetWars::PlanetWars(int iteration) {
	logger = new Logger("PlanetWars.log");
	turn_ = 0;
	num_planets_ = -1;
	planetColony = new std::map<int, int>();
	iteration_ = iteration;
}

int PlanetWars::NumPlanets() const {
	return num_planets_;
}

const Planet* PlanetWars::GetPlanet(int planet_id) const {
	return planets_[planet_id];
}

Planet* PlanetWars::GetPlanetNewState(int planet_id) const {
	return planets_new_state_[planet_id];
}

int PlanetWars::NumFleets() const {
	return fleets_.size();
}

const Fleet* PlanetWars::GetFleet(int fleet_id) const {
	return fleets_[fleet_id];
}

//TODO: Check if we can directly return the planets array
PlanetList PlanetWars::Planets() const {
	PlanetList r;
	for (int i = 0; i < planets_.size(); ++i) {
		Planet* p = planets_[i];
		r.push_back(p);
	}
	return r;
}

PlanetList PlanetWars::MyPlanets() const {
	return PlanetsOwnedBy(ME);
}

PlanetList PlanetWars::NeutralPlanets() const {
	return PlanetsOwnedBy(NEUTRAL);
}

PlanetList PlanetWars::EnemyPlanets() const {
	return PlanetsOwnedBy(ENEMY);
}

PlanetList PlanetWars::PlanetsOwnedBy(const int player) const {
	PlanetList r;
	for (int i = 0; i < planets_.size(); ++i) {
		Planet* p = planets_[i];
		if (p->Owner() == player) {
			r.push_back(p);
		}
	}
	return r;
}

PlanetList PlanetWars::NotMyPlanets() const {
	PlanetList r;
	for (int i = 0; i < planets_.size(); ++i) {
		Planet* p = planets_[i];
		if (p->Owner() != 1) {
			r.push_back(p);
		}
	}
	return r;
}

PlanetList PlanetWars::NClosestPlanets(int origin_id, int n) const{
	const int offset = origin_id * num_planets_;

	PlanetList planets_by_distance;
	planets_by_distance.reserve(n);

	for (int i = 1; i < n; ++i) {
		const int planet_index = offset + i;
		planets_by_distance.push_back(planets_by_distance_[planet_index]);
	}

	return planets_by_distance;
}

PlanetList PlanetWars::NClosestMinePlanets(int origin_id, int n) const{
	const int offset = origin_id * num_planets_;

	sprintf(logger->buffer, "Here %d", n);
	logger->log();

	PlanetList planets_by_distance;
	planets_by_distance.reserve(n);

	int count = 0;
	for (int i = 1; i < planets_.size(); i++) {
		const int planet_index = offset + i;

		if (planets_by_distance_[planet_index]->Owner() == ME){
			planets_by_distance.push_back(planets_by_distance_[planet_index]);
			count ++;
			if (count == n){
				break;
			}
		}
	}

	return planets_by_distance;
}

PlanetList PlanetWars::NClosestEnemyPlanets(int origin_id, int n) const{
	const int offset = origin_id * num_planets_;

	PlanetList planets_by_distance;
	planets_by_distance.reserve(n);

	int count = 0;
	for (int i = 1; i < planets_.size(); i++) {
		const int planet_index = offset + i;

		if (planets_by_distance_[planet_index]->Owner() == ENEMY){
			planets_by_distance.push_back(planets_by_distance_[planet_index]);
			count ++;
			if (count == n){
				break;
			}
		}
	}

	return planets_by_distance;
}

PlanetList PlanetWars::NClosestNeutralPlanets(int origin_id, int n) const{
	const int offset = origin_id * num_planets_;

	PlanetList planets_by_distance;
	planets_by_distance.reserve(n);

	int count = 0;
	for (int i = 1; i < planets_.size() && i < planets_by_distance_.size(); i++) {
		const int planet_index = offset + i;

		if (planets_by_distance_[planet_index]->Owner() == NEUTRAL){
			planets_by_distance.push_back(planets_by_distance_[planet_index]);
			count ++;
			if (count == n){
				break;
			}
		}
	}

	return planets_by_distance;
}

FleetList PlanetWars::Fleets() const {
	FleetList r;
	for (int i = 0; i < fleets_.size(); ++i) {
		Fleet* f = fleets_[i];
		r.push_back(f);
	}
	return r;
}

FleetList PlanetWars::MyFleets() const {
	FleetList r;
	for (int i = 0; i < fleets_.size(); ++i) {
		Fleet* f = fleets_[i];
		if (f->Owner() == ME) {
			r.push_back(f);
		}
	}
	return r;
}

FleetList PlanetWars::EnemyFleets() const {
	FleetList r;
	for (int i = 0; i < fleets_.size(); ++i) {
		Fleet* f = fleets_[i];
		if (f->Owner() > 1) {
			r.push_back(f);
		}
	}
	return r;
}

std::string PlanetWars::ToString() const {
	std::stringstream s;
	for (unsigned int i = 0; i < planets_.size(); ++i) {
		const Planet* p = planets_[i];
		s << "P " << p->X() << " " << p->Y() << " " << p->Owner() << " "
				<< p->NumShips() << " " << p->GrowthRate() << std::endl;
	}
	for (unsigned int i = 0; i < fleets_.size(); ++i) {
		const Fleet* f = fleets_[i];
		s << "F " << f->Owner() << " " << f->NumShips() << " " << f->SourcePlanet()
																												<< " " << f->DestinationPlanet() << " " << f->TotalTripLength()
																												<< " " << f->TurnsRemaining() << std::endl;
	}
	return s.str();
}

int PlanetWars::Distance(int source_planet, int destination_planet) const {
	const Planet* source = planets_[source_planet];
	const Planet* destination = planets_[destination_planet];
	double dx = source->X() - destination->X();
	double dy = source->Y() - destination->Y();
	return (int) ceil(sqrt(dx * dx + dy * dy));
}

int PlanetWars::ClosestPlanet(const int source_id) const {
	const int planet_index = source_id + num_planets_ + 1;
	Planet* closest_planet = planets_by_distance_[planet_index];
	const int closest_planet_id = closest_planet->PlanetID();
	return closest_planet_id;
}

int PlanetWars::MapRadius() const {
	int max_distance = 0;

	for (uint i = 0; i < planet_distances_.size(); ++i) {
		if (planet_distances_[i] > max_distance) {
			max_distance = planet_distances_[i];
		}
	}

	return max_distance;
}

void PlanetWars::IssueOrder(int source_planet, int destination_planet,
		int num_ships) const {
	std::cout << source_planet << " " << destination_planet << " " << num_ships
			<< std::endl;
	std::cout.flush();
	Planet* planet = GetPlanetNewState(source_planet);
	planet->NumShips(planet->NumShips() - num_ships);
}

bool PlanetWars::IsAlive(int player_id) const {
	for (size_t i = 0; i < planets_.size(); ++i) {
		if (planets_[i]->Owner() == player_id) {
			sprintf(logger->buffer, "Alive");
			logger->log();
			return true;
		}
	}
	/*for (size_t i = 0; i < fleets_.size(); ++i) {
		if (fleets_[i]->Owner() == player_id) {
			return true;
		}
	}*/
	return false;
}

int PlanetWars::NumShips(int player_id) const {
	int num_ships = 0;
	for (unsigned int i = 0; i < planets_.size(); ++i) {
		if (planets_[i]->Owner() == player_id) {
			num_ships += planets_[i]->NumShips();
		}
	}
	for (unsigned int i = 0; i < fleets_.size(); ++i) {
		if (fleets_[i]->Owner() == player_id) {
			num_ships += fleets_[i]->NumShips();
		}
	}
	return num_ships;
}

int PlanetWars::GrowthRate(int player_id) const {
	int growth_rate = 0;

	for (unsigned int i = 0; i < planets_.size(); ++i) {
		if (planets_[i]->Owner() == player_id) {
			growth_rate += planets_[i]->GrowthRate();
		}
	}

	return growth_rate;
}

//A functor to be used in the following function.
struct DistanceComparer {
	PlanetWars* planet_wars_;
	int origin_id;

	//Return true if the first planet at least as close to the origin planet
	//than the second planet.
	bool operator() (Planet* first_planet , Planet* second_planet) {
		const int first_planet_id = first_planet->PlanetID();
		const int second_planet_id = second_planet->PlanetID();
		const int first_distance = planet_wars_->Distance(origin_id, first_planet_id);
		const int second_distance = planet_wars_->Distance(origin_id, second_planet_id);
		return (first_distance < second_distance);
	}
};

int PlanetWars::Initialize(const std::string& s) {
	turn_ = 1;

	//Parse the game data.
	planets_.clear();
	fleets_.clear();
	std::vector<std::string> lines = StringUtil::Tokenize(s, "\n");
	int planet_id = 0;

	std::vector<int> fleet_source_ids;
	std::vector<int> fleet_destination_ids;

	for (unsigned int i = 0; i < lines.size(); ++i) {
		std::string& line = lines[i];
		size_t comment_begin = line.find_first_of('#');
		if (comment_begin != std::string::npos) {
			line = line.substr(0, comment_begin);
		}

		std::vector<std::string> tokens = StringUtil::Tokenize(line);
		if (tokens.size() == 0) {
			continue;
		}

		if (tokens[0] == "P") {
			if (tokens.size() != 6) {
				return 0;
			}

			Planet* p = new Planet(planet_id++,              // The ID of this planet
					atoi(tokens[3].c_str()),  // Owner
					atoi(tokens[4].c_str()),  // Num ships
					atoi(tokens[5].c_str()),  // Growth rate
					atof(tokens[1].c_str()),  // X
					atof(tokens[2].c_str())); // Y
			planets_.push_back(p);
			planets_new_state_.push_back(new Planet(*p));
		} else if (tokens[0] == "F") {
			if (tokens.size() != 7) {
				return 0;
			}

			Fleet* fleet = new Fleet();
			fleet->SetOwner(atoi(tokens[1].c_str()));
			fleet->SetNumShips(atoi(tokens[2].c_str()));
			fleet_source_ids.push_back(atoi(tokens[3].c_str()));
			fleet_destination_ids.push_back(atoi(tokens[4].c_str()));
			fleet->SetTripLength(atoi(tokens[5].c_str()));
			fleet->SetTurnsRemaining(atoi(tokens[6].c_str()));

			fleets_.push_back(fleet);

		} else {
			return 0;
		}
	}

	//Pre-calculate the distances between the planets.
	num_planets_ = static_cast<int>(planets_.size());
	planet_distances_.reserve(num_planets_ * num_planets_);

	for (int origin = 0; origin < num_planets_; ++origin) {
		for (int destination = 0; destination < num_planets_; ++destination) {
			const double dx = planets_[origin]->X() - planets_[destination]->X();
			const double dy = planets_[origin]->Y() - planets_[destination]->Y();

			const int distance = static_cast<int>(ceil(sqrt(dx * dx + dy * dy)));

			planet_distances_.push_back(distance);
		}
	}

	/*//Resolve planet references within the fleets and assign fleets
    //to their destinations.
    fleets_by_destination_.resize(planets_.size());

    for (unsigned int i = 0; i < fleets_.size(); ++i) {
        Fleet* fleet = fleets_[i];
        const int destination_id = fleet_destination_ids[i];

        fleet->SetSource(planets_[fleet_source_ids[i]]);
        fleet->SetDestination(planets_[destination_id]);

        fleets_by_destination_[destination_id].push_back(fleet);
    }*/

	//Pre-sort planets by distance from each other.
	//Use a functor defined immediately before this function.
	DistanceComparer distance_comparer;
	distance_comparer.planet_wars_ = this;
	distance_comparer.origin_id = 0;

	planets_by_distance_.clear();
	planets_by_distance_.reserve(num_planets_ * num_planets_);

	for (planet_id = 0; planet_id < num_planets_; ++planet_id) {
		//Sort the planets by distance from the origin planet and
		//append the result to the general vector of planets sorted
		//by distance.
		std::vector<Planet*> planets_to_sort(planets_);
		distance_comparer.origin_id = planet_id;
		std::sort(planets_to_sort.begin(), planets_to_sort.end(), distance_comparer);
		planets_by_distance_.insert(planets_by_distance_.end(),
				planets_to_sort.begin(), planets_to_sort.end());
	}

	return 1;
}

int PlanetWars::Update(const std::string& s) {
	++turn_;

	//Update the planet data; repopulate the fleets.
	fleets_.clear();
	std::vector<std::string> lines = StringUtil::Tokenize(s, "\n");
	int planet_id = 0;

	std::vector<int> fleet_source_ids;
	std::vector<int> fleet_destination_ids;

	for (unsigned int i = 0; i < lines.size(); ++i) {
		std::string& line = lines[i];
		size_t comment_begin = line.find_first_of('#');
		if (comment_begin != std::string::npos) {
			line = line.substr(0, comment_begin);
		}

		std::vector<std::string> tokens = StringUtil::Tokenize(line);
		if (tokens.size() == 0) {
			continue;
		}

		if (tokens[0] == "P") {
			//Read a line describing a planet
			if (tokens.size() != 6) {
				return 0;
			}

			//Update the planet state.
			Planet* planet = planets_[planet_id];
			planet->Owner(atoi(tokens[3].c_str()));
			planet->NumShips(atoi(tokens[4].c_str()));

			planet = planets_new_state_[planet_id];
			planet->Owner(atoi(tokens[3].c_str()));
			planet->NumShips(atoi(tokens[4].c_str()));

			planet_id++;

		} else if (tokens[0] == "F") {
			//Read a line describing a fleet
			if (tokens.size() != 7) {
				return 0;
			}

			Fleet* fleet = new Fleet();
			fleet->SetOwner(atoi(tokens[1].c_str()));
			fleet->SetNumShips(atoi(tokens[2].c_str()));
			fleet_source_ids.push_back(atoi(tokens[3].c_str()));
			fleet_destination_ids.push_back(atoi(tokens[4].c_str()));
			fleet->SetTripLength(atoi(tokens[5].c_str()));
			fleet->SetTurnsRemaining(atoi(tokens[6].c_str()));

			fleets_.push_back(fleet);

		} else {
			return 0;
		}
	}

	/*//Clear the fleets sorted by destination.
    for (uint i = 0; i < fleets_by_destination_.size(); ++i) {
        fleets_by_destination_[i].clear();
    }

    //Resolve planet references within the fleets, and sort the fleets
    //by their destination.
    for (unsigned int i = 0; i < fleets_.size(); ++i) {
        Fleet* fleet = fleets_[i];
        const int destination_id = fleet_destination_ids[i];

        fleet->SetSource(planets_[fleet_source_ids[i]]);
        fleet->SetDestination(planets_[destination_id]);

        fleets_by_destination_[destination_id].push_back(fleet);
    }*/

	return 1;
}

void PlanetWars::FinishTurn() const {
	std::cout << "go" << std::endl;
	std::cout.flush();
}
