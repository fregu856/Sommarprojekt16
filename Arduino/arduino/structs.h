#ifndef structs_h
#define structs_h

// Struct describing an ADC to distance-pair (an instance of the struct will contain an ADC value
// read with analogRead and its corresponding distance, for a specific IR sensor)
struct ADC_distance_pair
{
  float ADC_value;
  float distance;
};

#endif