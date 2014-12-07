// This file contains helper code that does all the boring stuff for you.
// The code in this file takes care of storing lists of planets and fleets, as
// well as communicating with the game engine. You can get along just fine
// without ever looking at this file. However, you are welcome to modify it
// if you want to.
#ifndef PLANET_WARS_H_
#define PLANET_WARS_H_

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include "Logger.cpp"

enum Players{
	NEUTRAL = 0, ME = 1, ENEMY = 2
};

class Planet;
class Fleet;

typedef std::vector<Planet*> PlanetList;
typedef std::vector<Fleet*> FleetList;

// This is a utility class that parses strings.
class StringUtil {
public:
	// Tokenizes a string s into tokens. Tokens are delimited by any of the
	// characters in delimiters. Blank tokens are omitted.
	static void Tokenize(const std::string& s,
			const std::string& delimiters,
			std::vector<std::string>& tokens);

	// A more convenient way of calling the Tokenize() method.
	static std::vector<std::string> Tokenize(
			const std::string& s,
			const std::string& delimiters = std::string(" "));
};

// This class stores details about one fleet. There is one of these classes
// for each fleet that is in flight at any given time.
class Fleet {
public:
	// Initializes a fleet.
	Fleet();

	Fleet(int owner,
			int num_ships,
			int source_planet = -1,
			int destination_planet = -1,
			int total_trip_length = -1,
			int turns_remaining = -1);

	void SetOwner(int owner)                    {owner_ = owner;}
	void SetNumShips(int num_ships)             {num_ships_ = num_ships;}
	void SetSource(int planet)              {source_planet_ = planet;}
	void SetDestination(int planet)         {destination_planet_ = planet;}
	void SetTripLength(int trip_length)         {total_trip_length_ = trip_length; turns_remaining_ = trip_length;}
	void SetTurnsRemaining(int turns_remaining) {turns_remaining_ = turns_remaining;}

	// Returns the playerID of the owner of the fleet. Your player ID is always
	// 1. So if the owner is 1, you own the fleet. If the owner is 2 or some
	// other number, then this fleet belongs to your enemy.
	int Owner() const {return owner_;}

	// Returns the number of ships that comprise this fleet.
	int NumShips() const {return num_ships_;}

	// Returns the ID of the planet where this fleet originated.
	int SourcePlanet() const {return source_planet_;}

	// Returns the ID of the planet where this fleet is headed.
	int DestinationPlanet() const {return destination_planet_;}

	// Returns the total distance that is being traveled by this fleet. This
	// is the distance between the source planet and the destination planet,
	// rounded up to the nearest whole number.
	int TotalTripLength() const {return total_trip_length_;}

	// Returns the number of turns until this fleet reaches its destination. If
	// this value is 1, then the fleet will hit the destination planet next turn.
	int TurnsRemaining() const {return turns_remaining_;}

	std::string ToMoveOrder() const;

private:
	int owner_;
	int num_ships_;
	int source_planet_;
	int destination_planet_;
	int total_trip_length_;
	int turns_remaining_;
};

// Stores information about one planet. There is one instance of this class
// for each planet on the map.
class Planet {
public:
	// Initializes a planet.
	Planet(int planet_id,
			int owner,
			int num_ships,
			int growth_rate,
			double x,
			double y);

	Planet(const Planet& other);

	// Returns the ID of this planets. Planets are numbered starting at zero.
	int PlanetID() const;

	// Returns the ID of the player that owns this planet. Your playerID is
	// always 1. If the owner is 1, this is your planet. If the owner is 0, then
	// the planet is neutral. If the owner is 2 or some other number, then this
	// planet belongs to the enemy.
	int Owner() const;

	bool IsEnemys() const {return (ENEMY == owner_);}
	bool IsNeutral() const {return (NEUTRAL == owner_);}
	bool IsMine() const {return (ME == owner_);}

	// The number of ships on the planet. This is the "population" of the planet.
	int NumShips() const;

	// Returns the growth rate of the planet. Unless the planet is neutral, the
	// population of the planet grows by this amount each turn. The higher this
	// number is, the faster this planet produces ships.
	int GrowthRate() const;

	// The position of the planet in space.
	double X() const;
	double Y() const;

	// Use the following functions to set the properties of this planet. Note
	// that these functions only affect your program's copy of the game state.
	// You can't steal your opponent's planets just by changing the owner to 1
	// using the Owner(int) function! :-)
	void Owner(int new_owner);
	void NumShips(int new_num_ships);
	void AddShips(int amount);
	void RemoveShips(int amount);

private:
	int planet_id_;
	int owner_;
	int num_ships_;
	int growth_rate_;
	double x_, y_;
};

class PlanetWars {
public:
	// Initializes the game state given a string containing game state data.
	PlanetWars();

	int Initialize(const std::string& game_state);
	int Update(const std::string& game_state);

	// Returns the number of planets on the map. Planets are numbered starting
	// with 0.
	int NumPlanets() const;

	// Returns the planet with the given planet_id. There are NumPlanets()
	// planets. They are numbered starting at 0.
	const Planet* GetPlanet(int planet_id) const;

	Planet* GetPlanetNewState(int planet_id) const;

	// Returns the number of fleets.
	int NumFleets() const;

	// Returns the fleet with the given fleet_id. Fleets are numbered starting
	// with 0. There are NumFleets() fleets. fleet_id's are not consistent from
	// one turn to the next.
	const Fleet* GetFleet(int fleet_id) const;

	// Returns a list of all the planets.
	PlanetList Planets() const;

	// Return a list of all the planets owned by the current player. By
	// convention, the current player is always player number 1.
	PlanetList MyPlanets() const;

	// Return a list of all neutral planets.
	PlanetList NeutralPlanets() const;

	// Return a list of all the planets owned by rival players. This excludes
	// planets owned by the current player, as well as neutral planets.
	PlanetList EnemyPlanets() const;

	// Return a list of all the planets that are not owned by the current
	// player. This includes all enemy planets and neutral planets.
	PlanetList NotMyPlanets() const;

	// Return a list of all the fleets.
	FleetList Fleets() const;

	// Return a list of all the fleets owned by the current player.
	FleetList MyFleets() const;

	// Return a list of all the fleets owned by enemy players.
	FleetList EnemyFleets() const;

	// Writes a string which represents the current game state. This string
	// conforms to the Point-in-Time format from the project Wiki.
	std::string ToString() const;

	// Returns the distance between two planets, rounded up to the next highest
	// integer. This is the number of discrete time steps it takes to get between
	// the two planets.
	int Distance(int source_planet, int destination_planet) const;
	int ClosestPlanet(int source_id) const;
	int MapRadius() const;

	PlanetList NClosestPlanets(int source_id, int n) const;
	PlanetList NClosestMinePlanets(int source_id, int n) const;
	PlanetList NClosestEnemyPlanets(int source_id, int n) const;
	PlanetList NClosestNeutralPlanets(int source_id, int n) const;

	// Sends an order to the game engine. The order is to send num_ships ships
	// from source_planet to destination_planet. The order must be valid, or
	// else your bot will get kicked and lose the game. For example, you must own
	// source_planet, and you can't send more ships than you actually have on
	// that planet.
	void IssueOrder(int source_planet,
			int destination_planet,
			int num_ships) const;

	// Returns true if the named player owns at least one planet or fleet.
	// Otherwise, the player is deemed to be dead and false is returned.
	bool IsAlive(int player_id) const;

	// Returns the number of ships that the given player has, either located
	// on planets or in flight.
	int NumShips(int player_id) const;

	int GrowthRate(int player_id) const;

	// Sends a message to the game engine letting it know that you're done
	// issuing orders for now.
	void FinishTurn() const;

	FleetList FleetsArrivingAt(int destination_planet) const;

	int Turn() const {return turn_;}

	std::map<int, int>* PlanetColony() const {return planetColony;}

private:
	// Parses a game state from a string. On success, returns 1. On failure,
	// returns 0.
	int ParseGameState(const std::string& s);
	PlanetList PlanetsOwnedBy(int player) const;
	PlanetList PlanetsNotOwnedBy(int player) const;

	// Store all the planets and fleets. OMG we wouldn't wanna lose all the
	// planets and fleets, would we!?
	PlanetList planets_;
	PlanetList planets_new_state_;
	FleetList fleets_;

	std::vector<int> planet_distances_;

	//Lists of planets sorted by distance from each other.
	//A list of planets sorted by distance from source_id starts at
	//element (source_id * num_planets) and ends at ((source_id+1) * num_planets - 1).
	PlanetList planets_by_distance_;

	std::map<int, int> *planetColony;
	int num_planets_;
	int turn_;

	Logger *logger;
};

#endif
