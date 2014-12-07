#ifndef CONSTANTS_H_
#define CONSTANTS_H_

//********************************************************//
//Colony Related Constants
#define COLONY_MAX_SIZE 4
#define MIN_STRONGNESS -4000
#define MAX_STRONGNESS 4000
//#define STEPS 500
#define STEPS 1600

#define MAX_NUM_MY_FLEETS 2
#define ELIGIBILITY_THRESHOLD_PERCENT 20

//********************************************************//

//********************************************************//
//Central Government Related Constants
#define MAX_TURNS_REMAINING_TO_CONSIDER_IN_REWARD 2

#define ALPHA_COLONY_Q 0.9
#define EXPLOITATION 0.8
#define DISCOUNT_COLONY 0.2

#define MAP_NAME "map07"
#define MAP_FORMAT "%.8f\n"
//********************************************************//


#endif /* CONSTANTS_H_ */
