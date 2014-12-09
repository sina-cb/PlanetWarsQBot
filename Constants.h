#ifndef CONSTANTS_H_
#define CONSTANTS_H_

//********************************************************//
//Colony Related Constants
#define COLONY_MAX_SIZE 4

#define MAX_NUM_MY_FLEETS 5
#define ELIGIBILITY_THRESHOLD_PERCENT 20

//#define STRONGNESS_STEP 50
//#define STRONGNESS_MAX 5

#define ALPHA_PLANET_Q 0.9
#define EXPLOITATION_PLANET 0.5
#define DISCOUNT_PLANET 0.2

#define PLANET_STRONGNESS_MIN 0
#define PLANET_STRONGNESS_MAX 400
#define PLANET_STRONGNESS_STEP 50

#define FLEET_SIZE_CONSTANT 0.8
#define FLEET_SIZE_CONSTANT_STEP 0.1
#define FLEET_SIZE_ITERATIONS 4

#define ENEMY_STRONGNESS_ESTIMATION_CONSTANT 1.1

//********************************************************//

//********************************************************//
//Central Government Related Constants
#define MIN_STRONGNESS -4000
#define MAX_STRONGNESS 4000
//#define STEPS 500
#define STEPS 1600

#define MAX_TURNS_REMAINING_TO_CONSIDER_IN_REWARD 2

#define ALPHA_COLONY_Q 0.9
#define EXPLOITATION_COLONY 0.5
#define DISCOUNT_COLONY 0.2

#define MAP_NAME_COLONY "map07-colonies"
#define MAP_NAME_CENTRAL "map07-central"
#define MAP_FORMAT "%.8f\n"
//********************************************************//


#endif /* CONSTANTS_H_ */
