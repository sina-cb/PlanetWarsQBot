#ifndef CONSTANTS_H_
#define CONSTANTS_H_

//********************************************************//
//Colony Related Constants

#define PLANET_ELIGIBILITY_CONSTANT 20


//********************************************************//

//********************************************************//
//Central Government Related Constants
#define COLONY_MAX_SIZE 5

#define COLONY_STRONGNESS_MIN -2000
#define COLONY_STRONGNESS_MAX 2000
#define COLONY_STRONGNESS_STEP 800

//****************LEARNING****************//
#define COLONY_EXPLOITION 0.3
#define COLONY_ALPHA 0.9
#define COLONY_DISCOUNT 0.85

//*****************RUNNING*****************//
/*
#define COLONY_EXPLOITION 0.9
#define COLONY_ALPHA 0.9
#define COLONY_DISCOUNT 0.85
*/
#define COLONY_DISCOUNT 0.8

#define MAP_FORMAT "%.8f\n"

#define COLONY_REWARD_MIN -2000
#define COLONY_REWARD_MAX 2000

#define GAME_WIN_REWARD 1000
#define GAME_LOSE_REWARD -1000

enum GameState {ON_GOING = 0, WIN = 1, LOST = 2};
//********************************************************//

#define MAX_VALUE 9999999999

#endif /* CONSTANTS_H_ */
