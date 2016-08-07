#ifndef structs_h
#define structs_h

// Struct describing an ADC to distance-pair (an instance of the struct will contain an ADC value
// read with analogRead and its corresponding distance, for a specific IR sensor)
struct ADC_distance_pair
{
  float ADC_value;
  float distance;
};

enum STATE
{
    DEAD_END = 1,
    CORRIDOR = 2,
    OUT_OF_CORRIDOR = 3,
    INTO_JUNCTION = 4,
    DETERMINE_JUNCTION = 5,
    JUNCTION_A_R = 6,
    JUNCTION_A_L = 7,
    JUNCTION_B_R = 8,
    JUNCTION_B_L = 9,
    JUNCTION_C_GO_LEFT = 10,
    JUNCTION_D = 11,
    OUT_OF_JUNCTION = 12,
    DETERMINE_IF_TARGET = 13,
    DETERMINE_IF_SIGN = 14,
    JUNCTION_C_GO_RIGHT = 15,
};

#endif