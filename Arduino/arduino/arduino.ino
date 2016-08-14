// include all structs: (this is needed because the arduino IDE automatically generate function 
// declarations and places them just after the pre-processor statements (after the includes),
// without this any code where any type of struct is used as an argument to some function would not 
// compile, since the structs has yet to be defined)  
#include "structs.h"

// pin numbers for the two motor controllers:
const int EN_R = 10;
const int EN_L = 3;
const int IN1_R = 7;
const int IN2_R = 8;
const int IN1_L = 4;
const int IN2_L = 2;

// definitions/mappings to ints:
const int stop_int = 0;
const int forward_int = 1;
const int backward_int = 2;
const int right_int = 3;
const int left_int = 4;
const int manual_int = 5;
const int auto_int = 6;

const double IR_sensor_distance_right = 12.5; // distance between the two IR-sensors on the right side [cm]
const float IR_sensor_distance_left = 12.5; // distance between the two IR-sensors on the left side [cm]

float IR_Yaw_right, IR_Yaw_left, Yaw, Yaw_rad;
float p_part;
float alpha;

uint8_t Kp = 4;
uint16_t Kd = 250;

uint8_t blue_percentage = 0; // the percentage of pixels in the current camera frame that are blue (or at least 'blue' enough)

uint8_t manual_state = stop_int;
uint8_t mode = manual_int;
STATE AUTO_STATE = CORRIDOR; // current auto state is "CORRIDOR" per default

int next_direction = -1;

int blue_counter = 0;
int blue_sum = 0;
float blue_average;

#define CORRIDOR_SIDE_DISTANCE 35 // distance for determining whether in corridor or not
#define DEAD_END_DISTANCE 12 // distance to the front wall when it's time to turn around in a dead end

int cycle_count; // counter that keeps track of the number of cycles (loops) in a specific state

// IR0: IR sensor Front
// IR1: IR sensor Right-Front 
// IR2: IR sensor Right-Back
// IR3: IR sensor Left-Back
// IR4: IR sensor Left-Front

int IR_latest_reading[5]; // the most recent reading, for each of the 5 IR sensors (array of 5 ints)
int IR_reading[5][10]; // 2D-array with the 10 latest readings, for each of the 5 IR-sensors (an int array of int arrays)
float IR_median[5]; // median value of the 10 latest readings, for each of the 5 IR sensors
float IR_distance[5]; // current distance for each of the 5 IR sensors (after filtering and conversion)  

// look up table (ADC value from analogRead -> distance) for IR sensor 0 (front sensor)
// (an array of ADC_distance_pairs)
ADC_distance_pair IR0_table[] =
{
  {55, 150},
  {73, 100},
  {82, 90},
  {87, 80},
  {97, 70},
  {107, 60},
  {124, 50},
  {145, 40},
  {161, 35},
  {183, 30},
  {215, 25},
  {260, 20},
  {333, 15},
  {475, 10},
  {665, 6}
};

// look up table (ADC value from analogRead -> distance) for IR sensor 1 (right front sensor)
// (an array of ADC_distance_pairs)
ADC_distance_pair IR1_table[] =
{
  {94, 90},
  {95, 80},
  {99, 70},
  {106, 60},
  {120, 50},
  {143, 40},
  {162, 35},
  {185, 30},
  {214, 25},
  {264, 20},
  {335, 15},
  {475, 10},
  {665, 6}
};

// look up table (ADC value from analogRead -> distance) for IR sensor 2 (right back sensor)
// (an array of ADC_distance_pairs)
ADC_distance_pair IR2_table[] =
{
  {28, 150},
  {70, 100},
  {79, 90},
  {87, 80},
  {97, 70},
  {108, 60},
  {125, 50},
  {152, 40},
  {165, 35},
  {183, 30},
  {214, 25},
  {260, 20},
  {328, 15},
  {455, 10},
  {669, 6}
};

// look up table (ADC value from analogRead -> distance) for IR sensor 3 (left back sensor)
// (an array of ADC_distance_pairs)
ADC_distance_pair IR3_table[] =
{
  {100, 80},
  {102, 70},
  {106, 60},
  {118, 50},
  {138, 40},
  {154, 35},
  {180, 30},
  {210, 25},
  {256, 20},
  {335, 15},
  {476, 10},
  {669, 6}
};

// look up table (ADC value from analogRead -> distance) for IR sensor 4 (left front sensor)
// (an array of ADC_distance_pairs)
ADC_distance_pair IR4_table[] =
{
  {39, 150},
  {82, 100},
  {90, 90},
  {98, 80},
  {110, 70},
  {122, 60},
  {138, 50},
  {164, 40},
  {182, 35},
  {202, 30},
  {235, 25},
  {279, 20},
  {349, 15},
  {482, 10},
  {670, 6}
};

// top speed: analogWrite(*some pin*, 255)
// no speed: analogWrite(*some pin*, 0)

void move_forward()
{
  // right side forward
  digitalWrite(IN1_R, LOW);
  digitalWrite(IN2_R, HIGH);
 
  // left side forward
  digitalWrite(IN1_L, HIGH);
  digitalWrite(IN2_L, LOW);
 
  analogWrite(EN_R, 150); // motor speed right
  analogWrite(EN_L, 150); // motor speed left 
}

void move_backward()
{
  // right side backward
  digitalWrite(IN1_R, HIGH);
  digitalWrite(IN2_R, LOW);
 
  // left side backward
  digitalWrite(IN1_L, LOW);
  digitalWrite(IN2_L, HIGH);
 
  analogWrite(EN_R, 100); // motor speed right
  analogWrite(EN_L, 100); // motor speed left 
}

void move_right()
{
  // right side backward
  digitalWrite(IN1_R, HIGH);
  digitalWrite(IN2_R, LOW);
 
  // left side forward
  digitalWrite(IN1_L, HIGH);
  digitalWrite(IN2_L, LOW);
 
  analogWrite(EN_R, 150); // motor speed right
  analogWrite(EN_L, 150); // motor speed left 
}

void move_left()
{
  // right side forward
  digitalWrite(IN1_R, LOW);
  digitalWrite(IN2_R, HIGH);
 
  // left side backward
  digitalWrite(IN1_L, LOW);
  digitalWrite(IN2_L, HIGH);
 
  analogWrite(EN_R, 150); // motor speed right
  analogWrite(EN_L, 150); // motor speed left 
}

void stop()
{
  digitalWrite(IN1_R, LOW);
  digitalWrite(IN2_R, LOW);
 
  digitalWrite(IN1_L, LOW);
  digitalWrite(IN2_L, LOW);
}

// when in manuel mode, move the robot in the direction specified by "manual_state":
void run_manual_state()
{
  switch (manual_state)
 {
   case stop_int:
   {
     stop();
     break;
   }
   
   case forward_int:
   {
     move_forward();
     break;
   }
   
   case backward_int:
   {
     move_backward();
     break;
   }
   
   case right_int:
   {
     move_right();
     break;
   }
   
   case left_int:
   {
     move_left();
     break;
   }

    default:
    {
        stop();
        break;
    }
 }
}

// read all serial data sent from the RPI:
void read_serial()
{
    // (the RPI always send the same number of bytes once it sends something, eventhough there's never more than three data bytes that actually should be read. We do this in order to get a fix protocol, which just makes life easier IMO)
    int no_of_bytes_waiting = Serial.available();
    if (no_of_bytes_waiting > 7) // the RPI sends 8 bytes at the time (6 data, 2 control)
    {
        // read the first byte:
        uint8_t first_byte = Serial.read();
        
        // read all data bytes if the first byte was the start byte:
        if (first_byte == 100)
        {           
            // read all data bytes:
            uint8_t manual_state_byte = Serial.read();
            uint8_t mode_byte = Serial.read();
            uint8_t Kp_byte = Serial.read();
            uint8_t Kd_low_byte = Serial.read();
            uint8_t Kd_high_byte = Serial.read();
            uint8_t blue_percentage_byte= Serial.read();
            
            // read the received checksum:
            uint8_t checksum = Serial.read();
            
            // calculate checksum for the received data bytes:
            uint8_t calc_checksum = manual_state_byte + mode_byte + Kp_byte + 
                Kd_low_byte + Kd_high_byte + blue_percentage_byte;
                
            // update the variables with the read serial data only if the checksums match:
            if (calc_checksum == checksum)
            {
                if (manual_state_byte != 0xFF) // 0xFF (255) is sent if it's not supposed to be read
                {
                    if (mode == manual_int) // only allow change of manual state in manual mode
                    {
                        manual_state = manual_state_byte;
                    }
                }
                
                if (mode_byte != 0xFF) // 0xFF is sent if it's not supposed to be read
                {
                    if ((mode_byte == auto_int) && (mode == manual_int)) // if mode is changed from manual to auto:
                    {
                        mode = mode_byte;
                        AUTO_STATE = CORRIDOR; // always start in CORRIDOR when we switch to auto mode
                    }
                    
                    else
                    {
                        mode = mode_byte;
                    }
                }
                
                if (Kp_byte != 0xFF) // 0xFF is sent if it's not supposed to be read
                {
                    if (mode == manual_int) // only allow change of parameters in manual mode
                    {
                        Kp = Kp_byte; 
                    }
                }

                uint16_t Kd_16 = (Kd_low_byte + Kd_high_byte*256);
                if (Kd_16 != 0xFFFF) // 0xFFFF (65535) is sent if it's not supposed to be read
                {
                    if (mode == manual_int) // only allow change of parameters in manual mode
                    {
                        Kd = Kd_16;
                    }
                }   

                if (blue_percentage_byte != 0xFF) // 0xFF (255) is sent if it's not supposed to be read
                {
                    blue_percentage = blue_percentage_byte;
                }
                
            }
            
            else // if the checksums doesn't match: something weird has happend during transmission: flush input buffer and start over
            {
                while (Serial.available())
                {
                    Serial.read();
                }
            }
        }
        
        else // if the first byte isn't the start byte: we're not in sync: just start over and read the next byte until we get in sync (until we reach the start byte) 
        {
            
        }
    }
}

// read the raw ADC values for all 5 IR sensor:
void read_IR_sensors()
{
    // read analog pin 0 - 4 (A0 - A4) (analogRead returns result of 10-bit ADC, an int between 0 and 1023)
    for (int sensor_ID = 0; sensor_ID <= 4; ++sensor_ID)
    {
        IR_latest_reading[sensor_ID] = analogRead(sensor_ID);
    }
}

// get the median value of the values in "array" (which is an array containing "no_of_elements" ints)
float get_median(int array[], int no_of_elements)
{
    float median;
    
    // sort array using Bubble sort:
    for (int i = 0; i < no_of_elements - 1; ++i) // for no_of_elements - 1 times: (number of needed cycles in the worst case)
    {
        for (int k = 0; k < no_of_elements - (i + 1); ++k)
        {
            if (array[k] > array[k + 1])
            {
                // swap positions:
                int temp_container = array[k];
                array[k] = array[k + 1];
                array[k + 1] = temp_container;
            }
        }
    }
    
    // get median value (two different cases depending on whether even number of elements or not)
    if (no_of_elements % 2 != 0) // if ODD no of elements: (if the remainder when dividing with 2 is non-zero)
    {
        int median_index = no_of_elements/2 - (no_of_elements/2 % 2); // median_index = floor(no_of_elements/2)
        median = array[median_index];
    }
    
    else // if EVEN no of elements: (if there is no remainder when dividing with 2)
    {
        int median_index = no_of_elements/2;
        median = (array[median_index] + array[median_index - 1])/2.f; // "2.f" makes sure we don't do integer division (which eg would give ous 5 instead of 5.5)
    }

    return median;
}

// get the median of the 10 latest read ADC values, for all 5 IR sensors: (digital low pass filter)
void filter_IR_values()
{
    for (int sensor_ID = 0; sensor_ID <= 4; ++sensor_ID) // for IR0 to IR4:
    {
        // shift out the oldest reading:
        for (int reading_index = 9; reading_index >= 1; --reading_index)
        {
            IR_reading[sensor_ID][reading_index] = IR_reading[sensor_ID][reading_index - 1];
        }
        
        // insert the latest reading:
        IR_reading[sensor_ID][0] = IR_latest_reading[sensor_ID];
        
        // get the median value of the 10 latest readings: (filter sensor readings)
        int size = sizeof(IR_reading[sensor_ID])/sizeof(int); // we get the number of elements (ints) in the array by dividing the size of the array (in number of bytes) with the size of one int (in number of bytes)
        int array_copy[] = {IR_reading[sensor_ID][0], IR_reading[sensor_ID][1], // need to make a copy here, "get_median" apparently does something weird with its passed array 
                            IR_reading[sensor_ID][2], IR_reading[sensor_ID][3], 
                            IR_reading[sensor_ID][4], IR_reading[sensor_ID][5], 
                            IR_reading[sensor_ID][6], IR_reading[sensor_ID][7], 
                            IR_reading[sensor_ID][8], IR_reading[sensor_ID][9]}; 
        IR_median[sensor_ID] = get_median(array_copy, size);
    }
}

// convert the raw ADC value "ADC_value" to its corresponding distance with the help of the lookup table "ADC_dist_table" and linear interpolation:
float lookup_distance(ADC_distance_pair ADC_dist_table[], float ADC_value, int table_size)
{
    // return minimum value if "ADC_value" is smaller than the smallest table value:
	if(ADC_value <= ADC_dist_table[0].ADC_value)	
    {
        return ADC_dist_table[0].distance;
    }

    // return maximum value if "ADC_value" is greater than the biggest table value:
	if(ADC_value >= ADC_dist_table[table_size-1].ADC_value)
	{
        return ADC_dist_table[table_size-1].distance;
    }

    // linear interpolation:
	for(int i = 0; i < table_size-1; i++)
	{
        // interpolate (linearly) if "ADC_value" is between two table values:
		if (ADC_dist_table[i].ADC_value <= ADC_value && ADC_value <= ADC_dist_table[i+1].ADC_value) // Linjärinterpolera om ADC-värdet ligger mellan två tabell-värden
		{
			float diff_ADC = ADC_value - ADC_dist_table[i].ADC_value;
			float step_length = ADC_dist_table[i+1].ADC_value - ADC_dist_table[i].ADC_value;

			return ADC_dist_table[i].distance + (ADC_dist_table[i+1].distance - ADC_dist_table[i].distance)*(diff_ADC/step_length);
		}
	}
    
    // Return -1 if anything went wrong (we should never reach this section):
	return -1;
}

// convert the median ADC value to its corresponding distance, for all 5 IR sensors:
void convert_IR_values()
{
    IR_distance[0] = lookup_distance(IR0_table, IR_median[0], 15);
  	IR_distance[1] = lookup_distance(IR1_table, IR_median[1], 13);
  	IR_distance[2] = lookup_distance(IR2_table, IR_median[2], 15);
  	IR_distance[3] = lookup_distance(IR3_table, IR_median[3], 12);
  	IR_distance[4] = lookup_distance(IR4_table, IR_median[4], 15);
}

// use the "alpha_value" from the PD controller to move the robot forward (in auto mode) while keeping it centered in the corridor:
void move_forward_with_control(float speed, float alpha_value)
{
  // right side forward:
  digitalWrite(IN1_R, LOW);
  digitalWrite(IN2_R, HIGH);
 
  // left side forward:
  digitalWrite(IN1_L, HIGH);
  digitalWrite(IN2_L, LOW);

  // adjust the motor speeds to steer the robot toward the center of the corridor: (once the robot is centered and aligned, "alpha_value" will equal 0 and the robot will just move straight forward)
  float right_speed = speed - alpha_value; 
  float left_speed = speed + alpha_value;
  
  // limit the motor speeds:
  if (right_speed > 250)
  {
    right_speed = 250;
  }
  if (right_speed < 0)
  {
    right_speed = 0;
  }
  if (left_speed > 250)
  {
    left_speed = 250;
  }
  if (left_speed < 0)
  {
    left_speed = 0;
  }
  
  analogWrite(EN_R, right_speed); // set motor speed right
   analogWrite(EN_L, left_speed); // set motor speed left
}

// calculate the angle of the robot to the corridor walls: (this is used in the "D" part of the PD controller)
void calculate_Yaw()
{
	float l_delta_right = IR_distance[1] - IR_distance[2];
	float l_delta_left = IR_distance[3] - IR_distance[4];

    // calculate the angle relative the corridor walls using trigonometry, based on the right side IR sensors:
    IR_Yaw_right = (atan(l_delta_right/IR_sensor_distance_right)/3.1415926)*180;	
    // calculate the angle relative the corridor walls using trigonometry, based on the left side IR sensors:
    IR_Yaw_left = (atan(l_delta_left/IR_sensor_distance_left)/3.1415926)*180;
    
    // take "Yaw" to be the mean:
    Yaw = (IR_Yaw_right + IR_Yaw_left)/2;
    // convert to radians (to use in the control algorithm, in degrees for display to user):
    Yaw_rad = (Yaw/180)*3.1415926;
}

// calculate the offset of the robot from the center of the corridor: (this is used in the "P" part if the PD controller) 
void calculate_p_part()
{
    p_part = IR_distance[2] - IR_distance[4];
}

// calculate the "steering angle" from the PD controller: (when the robot is centered and aligned with the corridor, alpha will equal 0 and the robot will just move straight ahead, otherwise the robot will steer toward the center of the corridor and align itself with the corridor walls)
void calculate_alpha()
{
    if ((IR_Yaw_right - IR_Yaw_left > 15) || (IR_Yaw_right - IR_Yaw_left < -15)) // if they differ too much we're probably just on our way out of a juction and should not do any adjustments
    {
        alpha = 0;
    }
    
    else
    {
        alpha = Kp*p_part + Kd*Yaw_rad;
    }
}

// when in auto mode, update the AUTO_STATE based on the current sensor values and previous states:
void update_state()
{
    // get the current sensor values:
    float IR_0 = IR_distance[0];
    float IR_1 = IR_distance[1];
    float IR_2 = IR_distance[2];
    float IR_3 = IR_distance[3];
    float IR_4 = IR_distance[4];
    
    switch (AUTO_STATE)
	{
        // when in a dead end, turn until the road ahead is open again:
        case DEAD_END:
        {
            if ((IR_0 > 35) && (IR_1 < CORRIDOR_SIDE_DISTANCE-10) && (IR_2 < CORRIDOR_SIDE_DISTANCE-10)
            && (IR_3 < CORRIDOR_SIDE_DISTANCE-10) && (IR_4 < CORRIDOR_SIDE_DISTANCE-10))
            {
                AUTO_STATE = OUT_OF_DEAD_END;
                cycle_count = 0;
            }
            
            break;
        }
        
        // when the road ahead is open again after a dead end, stop shortly to give the sensor values some time to get back to normal, then go back to CORRIDOR:
        case OUT_OF_DEAD_END:
        {
            if (cycle_count > 10)
            {
                AUTO_STATE = CORRIDOR;
                cycle_count = 0;
            }
            
            break;
        }
        
        // when in a corridor, look for dead ends (walls in front) or side openings:
		case CORRIDOR:
		{
			if ((IR_0 < DEAD_END_DISTANCE) && (IR_1 < CORRIDOR_SIDE_DISTANCE)
            && (IR_2 < CORRIDOR_SIDE_DISTANCE) && (IR_3 < CORRIDOR_SIDE_DISTANCE)
            && (IR_4 < CORRIDOR_SIDE_DISTANCE))
            {
                AUTO_STATE = DEAD_END;
                cycle_count = 0;
            }
            
            else if ((IR_1 > CORRIDOR_SIDE_DISTANCE) || (IR_2 > CORRIDOR_SIDE_DISTANCE) 
            || (IR_3 > CORRIDOR_SIDE_DISTANCE - 10) || (IR_4 > CORRIDOR_SIDE_DISTANCE))
            {
                AUTO_STATE = DETERMINE_IF_SIGN;
                cycle_count = 0;
            }
            
			break;
		}
        
        // when we have discovered a side opening, the robot is just about to get into a junction, and it's at a good distance to look at the camera frames and check if there is a blue sign (meaning that the robot should turn right) on the junction wall: (stop for a while, look at a number of frames and then check if (on average) there is a lot of blue in them)
        case DETERMINE_IF_SIGN:
        {
            if (cycle_count > 15)
            {
                AUTO_STATE = OUT_OF_CORRIDOR;
                cycle_count = 0;
                blue_average = blue_sum/blue_counter;
                blue_sum = 0;
                blue_counter = 0;
                
                if (blue_average > 10)
                {
                    next_direction = right_int;
                }
                
                else
                {
                    next_direction = left_int;
                }
            }
            
            break;
        } 
        
        // move forward until the entire side of the robot is out of the corridor:
        case OUT_OF_CORRIDOR:
        {
            if (((IR_1 > CORRIDOR_SIDE_DISTANCE) && (IR_2 > CORRIDOR_SIDE_DISTANCE) 
            && (IR_3 < CORRIDOR_SIDE_DISTANCE) && (IR_4 < CORRIDOR_SIDE_DISTANCE)) 
            || ((IR_1 < CORRIDOR_SIDE_DISTANCE) && (IR_2 < CORRIDOR_SIDE_DISTANCE) 
            && (IR_3 > CORRIDOR_SIDE_DISTANCE-10) && (IR_4 > CORRIDOR_SIDE_DISTANCE)) 
            || ((IR_1 > CORRIDOR_SIDE_DISTANCE) && (IR_2 > CORRIDOR_SIDE_DISTANCE) 
            && (IR_3 > CORRIDOR_SIDE_DISTANCE-10) && (IR_4 > CORRIDOR_SIDE_DISTANCE))) 
            {
                AUTO_STATE = INTO_JUNCTION;
                cycle_count = 0;
            }
            
            break;
        }
        
        // move forward for a fix number of cycles, in order to position the robot in the center of the junction: 
        case INTO_JUNCTION:
        {
            if (cycle_count > 5)
            {
                AUTO_STATE = DETERMINE_JUNCTION;
                cycle_count = 0;
            }
            
            break;
        }
        
        // when the robot is right in the junction, stop for a while (to give the sensor values some time to become steady), look at the sensor values and determine which type of junction the robot currently is in:
        case DETERMINE_JUNCTION:
        {            
            if ((cycle_count > 15) && (IR_0 < CORRIDOR_SIDE_DISTANCE)
            && (IR_1 > CORRIDOR_SIDE_DISTANCE) && (IR_2 > CORRIDOR_SIDE_DISTANCE) 
            && (IR_3 < CORRIDOR_SIDE_DISTANCE) && (IR_4 < CORRIDOR_SIDE_DISTANCE))
            {
                AUTO_STATE = JUNCTION_A_R;
                cycle_count = 0;
            }
            
            else if ((cycle_count > 15) && (IR_0 < CORRIDOR_SIDE_DISTANCE-10)
            && (IR_1 < CORRIDOR_SIDE_DISTANCE) && (IR_2 < CORRIDOR_SIDE_DISTANCE) 
            && (IR_3 > CORRIDOR_SIDE_DISTANCE-10) && (IR_4 > CORRIDOR_SIDE_DISTANCE))
            {
                AUTO_STATE = JUNCTION_A_L;
                cycle_count = 0;
            }
            
            else if ((cycle_count > 15) && (IR_0 > CORRIDOR_SIDE_DISTANCE)
            && (IR_1 < CORRIDOR_SIDE_DISTANCE) && (IR_2 < CORRIDOR_SIDE_DISTANCE) 
            && (IR_3 > CORRIDOR_SIDE_DISTANCE-10) && (IR_4 > CORRIDOR_SIDE_DISTANCE))
            {
                AUTO_STATE = JUNCTION_B_L;
                cycle_count = 0;
            }

            else if ((cycle_count > 15) && (IR_0 > CORRIDOR_SIDE_DISTANCE)
            && (IR_1 > CORRIDOR_SIDE_DISTANCE) && (IR_2 > CORRIDOR_SIDE_DISTANCE) 
            && (IR_3 < CORRIDOR_SIDE_DISTANCE) && (IR_4 < CORRIDOR_SIDE_DISTANCE))
            {
                AUTO_STATE = JUNCTION_B_R;
                cycle_count = 0;
            }
            
            else if ((cycle_count > 15) && (IR_0 < CORRIDOR_SIDE_DISTANCE)
            && ((IR_1 > CORRIDOR_SIDE_DISTANCE) || (IR_2 > CORRIDOR_SIDE_DISTANCE)) 
            && ((IR_3 > CORRIDOR_SIDE_DISTANCE-10) || (IR_4 > CORRIDOR_SIDE_DISTANCE)))
            {   
                if (next_direction == right_int) // if the robot is in a C junction and there was a lot of blue on the junction wall (there was a blue sign), it should go to the right:
                {
                    AUTO_STATE = JUNCTION_C_GO_RIGHT;
                    cycle_count = 0;
                }
                
                else if (next_direction == left_int)
                {
                    AUTO_STATE = JUNCTION_C_GO_LEFT;
                    cycle_count = 0;
                }

                else // (this should never even happen)
                {
                    break;
                }
            }
            
            else if ((cycle_count > 15) && (IR_0 > CORRIDOR_SIDE_DISTANCE)
            && ((IR_1 > CORRIDOR_SIDE_DISTANCE) || (IR_2 > CORRIDOR_SIDE_DISTANCE)) 
            && ((IR_3 > CORRIDOR_SIDE_DISTANCE-10) || (IR_4 > CORRIDOR_SIDE_DISTANCE)))
            {   
                AUTO_STATE = END_OF_COURSE;
                cycle_count = 0;       
            }
            
            break;
        }
        
        // turn until there is an opening straight ahead and to the right:
        case JUNCTION_A_R:
        {
            if ((IR_0 > 35) && ((IR_1> 30) || (IR_2 > 30)))
            {
                AUTO_STATE = OUT_OF_JUNCTION;
                cycle_count = 0;
            }
            
            break;
        }
        
        // turn until there is an opening straight ahead and to the left:
        case JUNCTION_A_L:
        {
            if ((IR_0 > 30) && ((IR_3 > 30) || (IR_4 > 30)))
            {
                AUTO_STATE = OUT_OF_JUNCTION;
                cycle_count = 0;
            }
            
            break;
        }
        
        // move forward until the robot is in the corridor again:
        case JUNCTION_B_R:
        {
            if ((IR_1 < CORRIDOR_SIDE_DISTANCE) && (IR_2 < CORRIDOR_SIDE_DISTANCE) 
            && (IR_3 < CORRIDOR_SIDE_DISTANCE) && (IR_4 < CORRIDOR_SIDE_DISTANCE))
            {
                AUTO_STATE = CORRIDOR;
                cycle_count = 0;
            }
            
            break;
        }
        
        // turn until all sides are open:
        case JUNCTION_B_L:
        {
            if ((IR_0 > 30) && ((IR_1 > 30) || (IR_2 > 30)) && ((IR_3 > 30) || (IR_4 > 30)))
            {
                AUTO_STATE = OUT_OF_JUNCTION;
                cycle_count = 0;
            }
            
            break;
        }
        
        // turn until there is an opening straight ahead and there is a wall to the right:
        case JUNCTION_C_GO_LEFT:
        {
            if ((IR_0 > 30) && (IR_1 < CORRIDOR_SIDE_DISTANCE) && (IR_2 < CORRIDOR_SIDE_DISTANCE))
            {
                AUTO_STATE = OUT_OF_JUNCTION;
                cycle_count = 0;
            }

            break;
        }
        
        // turn until there is an opening straight ahead and there is a wall to the left:
        case JUNCTION_C_GO_RIGHT:
        {
            if ((IR_0 > 30) && (IR_3 < CORRIDOR_SIDE_DISTANCE) && (IR_4 < CORRIDOR_SIDE_DISTANCE))
            {
                AUTO_STATE = OUT_OF_JUNCTION;
                cycle_count = 0;
            }

            break;
        }
        
        // move forward until the robot is back in the corridor:
        case OUT_OF_JUNCTION:
        {
            if ((IR_1 < CORRIDOR_SIDE_DISTANCE) && (IR_2 < CORRIDOR_SIDE_DISTANCE) 
            && (IR_3 < CORRIDOR_SIDE_DISTANCE - 10) && (IR_4 < CORRIDOR_SIDE_DISTANCE))
            {
                AUTO_STATE = CORRIDOR;
                cycle_count = 0;
            }
            break;
        }
		
        // do nothing if the robot has reached the end of the course:
        case END_OF_COURSE:
        {
            break;
        }
        
		default:
        {
            break;
        }
	}   
}

// execute the current AUTO_STATE:
void run_state()
{
    switch (AUTO_STATE)
	{
        case DEAD_END:
        {
            move_right();
            ++cycle_count;

            break;
        }
        
        case OUT_OF_DEAD_END:
        {
            stop();
            ++cycle_count;

            break;
        }
        
		case CORRIDOR:
		{
			move_forward_with_control(100, alpha);
            ++cycle_count;
			break;
		}
        
        case OUT_OF_CORRIDOR:
        {
            move_forward_with_control(100, 0);
            ++cycle_count;
            break;
        }
			
        case INTO_JUNCTION:
        {
            move_forward_with_control(110, 0);
            ++cycle_count;
            break;
        }
        
        case DETERMINE_JUNCTION:
        { 
            stop();
            ++cycle_count;
            break;
        }
        
        case JUNCTION_A_R:
        {
            move_right();
            ++cycle_count;
            break;
        }
        
        case JUNCTION_A_L:
        {
            move_left();
            ++cycle_count;
            break;
        }
        
        case JUNCTION_B_R:
        {
            move_forward_with_control(100, 0); // always take the path most to the left (prio: left, forward, right)
            ++cycle_count;
            break;
        }
        
        case JUNCTION_B_L:
        {
            move_left();
            ++cycle_count;
            break;
        }
        
        case JUNCTION_C_GO_LEFT:
        {
            move_left();
            ++cycle_count;
            break;
        }
        
        case JUNCTION_C_GO_RIGHT:
        {
            move_right();
            ++cycle_count;
            break;
        }
        
        case OUT_OF_JUNCTION:
        {
            move_forward_with_control(100, 0);
            ++cycle_count;
            break;
        }
        
        case DETERMINE_IF_SIGN:
        {
            stop();
            ++cycle_count;
            ++blue_counter;
            blue_sum = blue_sum + blue_percentage;
            break;
        }
		
        case END_OF_COURSE:
        {
            stop();
            break;
        }
        
		default:
        {
            stop();
            break;
        }
	}
}

// restrict "value" to fit in an uint8_t:
float restrict_to_unsigned_size(float value)
{
    if (value > 255)
    {
        return 255;
    }
    
    else
    {
        return value;
    }
}

// restrict "value" to fit in an int8_t:
float restrict_to_signed_size(float value)
{
    if (value > 127)
    {
        return 127;
    }
    
    else if (value < -128)
    {
        return -128;
    }
    
    else
    {
        return value;
    }
}

// send all data to the RPI over serial:
void send_serial()
{  
    // restrict data to be sent and convert to one byte: (restrict to 0 - 255 or -128 - 127)
    uint8_t IR_0_byte = restrict_to_unsigned_size(IR_distance[0]);
    uint8_t IR_1_byte = restrict_to_unsigned_size(IR_distance[1]);
    uint8_t IR_2_byte = restrict_to_unsigned_size(IR_distance[2]);
    uint8_t IR_3_byte = restrict_to_unsigned_size(IR_distance[3]);
    uint8_t IR_4_byte = restrict_to_unsigned_size(IR_distance[4]);
    int8_t IR_Yaw_right_byte = restrict_to_signed_size(IR_Yaw_right);
    int8_t IR_Yaw_left_byte = restrict_to_signed_size(IR_Yaw_left);
    int8_t Yaw_byte = restrict_to_signed_size(Yaw);
    int8_t p_part_byte = restrict_to_signed_size(p_part);
    int16_t alpha_two_byte = alpha; // (don't have to restrict it to fit 16 bits)
    uint8_t alpha_low_byte = (alpha_two_byte & 0x00FF); // get the low byte and treat is as an uint8_t (yes, it should be this way, test it with pen and paper and some simple example (try sending -1, that shows very clearly why it has to be done this way))
    uint8_t alpha_high_byte = (alpha_two_byte & 0xFF00)/256; // get the high byte and treat is as an uint8_t
    uint8_t Kp_byte = restrict_to_unsigned_size(Kp);
    uint16_t Kd_two_byte = Kd; // (you don't have to restrict it to fit 16 bits since it can represent such big numbers)
    uint8_t Kd_low_byte = (Kd_two_byte & 0x00FF); // Get the low byte by bitwise AND with 0000 0000 1111 1111
    uint8_t Kd_high_byte = (Kd_two_byte & 0xFF00)/256; // Get the high byte by bitwise AND with 1111 1111 0000 0000 and 8 bit right shift (division by 256)
   
    // calculate checksum for the data bytes to be sent: (this will obviously overflow the uint8_t sometimes, but that's not a problem since we will do the exact same calculation on the receiving end (on the RPI) and then compare the two)
    uint8_t checksum = IR_0_byte + IR_1_byte + IR_2_byte + IR_3_byte + IR_4_byte + IR_Yaw_right_byte + 
        IR_Yaw_left_byte + Yaw_byte + p_part_byte + alpha_low_byte + alpha_high_byte + Kp_byte + 
        Kd_low_byte + Kd_high_byte + AUTO_STATE + manual_state + mode + blue_percentage;
    
    // indicate start of transmission:
    Serial.write(100);
    
    // send all data:
    Serial.write(IR_0_byte);
    Serial.write(IR_1_byte);
    Serial.write(IR_2_byte);
    Serial.write(IR_3_byte);
    Serial.write(IR_4_byte);
    Serial.write(IR_Yaw_right_byte);
    Serial.write(IR_Yaw_left_byte);
    Serial.write(Yaw_byte);
    Serial.write(p_part_byte);
    Serial.write(alpha_low_byte);
    Serial.write(alpha_high_byte);
    Serial.write(Kp_byte);
    Serial.write(Kd_low_byte);
    Serial.write(Kd_high_byte);
    Serial.write(AUTO_STATE);
    Serial.write(manual_state);
    Serial.write(mode);
    Serial.write(blue_percentage);
    
    // send checksum:
    Serial.write(checksum);
}

void setup()
{
 pinMode(EN_R, OUTPUT);  
 pinMode(EN_L, OUTPUT);
 pinMode(IN1_R, OUTPUT);  
 pinMode(IN2_R, OUTPUT);
 pinMode(IN1_L, OUTPUT);
 pinMode(IN2_L, OUTPUT);
 
 Serial.begin(9600);  // baudrate = 9600 bps
}

void loop()
{  
    read_serial();
    
    read_IR_sensors();
    filter_IR_values();
    convert_IR_values();
    
    calculate_Yaw();
    calculate_p_part();
    calculate_alpha();
    
    if (mode == manual_int)
    {
        run_manual_state();
    }
    
    else if (mode == auto_int)
    {
        update_state();
        run_state();
    }
    
    send_serial();
    
    delay(50);  // delay for loop frequency of roughly 20 Hz
}
