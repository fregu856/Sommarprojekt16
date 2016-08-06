// Include all structs: (this is needed because the arduino IDE automatically generate function 
// declarations and places them just after the pre-processor statements (after the includes),
// without this any code where any type of struct is used as an argument to some function would not 
// compile, since the structs has yet to be defined)  
#include "structs.h"

const int EN_R = 10;
const int EN_L = 3;
const int IN1_R = 7;
const int IN2_R = 8;
const int IN1_L = 4;
const int IN2_L = 2;

const int stop_int = 0;
const int forward_int = 1;
const int backward_int = 2;
const int right_int = 3;
const int left_int = 4;

const int manual_int = 5;
const int auto_int = 6;

const int Kp_and_Kd_int = 7;
const int Kp_int = 8;
const int Kd_int = 9;

const double IR_sensor_distance_right = 12.5;		// Distance between the two IR-sensors on the right side [cm]
const float IR_sensor_distance_left = 12.5;		// Distance between the two IR-sensors on the left side [cm]

float IR_Yaw_right, IR_Yaw_left, Yaw, Yaw_rad;
float p_part;
float alpha;

uint8_t Kp = 4;
uint16_t Kd = 250;

uint8_t manual_state = stop_int;
uint8_t mode = manual_int;
STATE AUTO_STATE = CORRIDOR; // current auto state is "CORRIDOR" per default

int next_direction = -1;

#define CORRIDOR_SIDE_DISTANCE 35 // Distance for determining whether in corridor or not
#define DEAD_END_DISTANCE 12 // Distance to the front wall when it's time to turn around in a dead end

int cycle_count; // counter that keeps track of the number of cycles (loops) in a specific state
int startup_counter; // counter for giving the sensor values some time to get to normal after start of auto mode
// IR0: IR sensor Front
// IR1: IR sensor Right-Front 
// IR2: IR sensor Right-Back
// IR3: IR sensor Left-Back
// IR4: IR sensor Left-Front

int IR_latest_reading[5]; // The most recent reading, for each of the 5 IR sensors (array of 5 elements)
int IR_reading[5][10]; // 2D-array with the 10 latest readings, for each the 5 IR-sensors (a int array of int arrays)
float IR_median[5]; // Median value of the 10 latest readings, for each of the 5 IR sensors
float IR_distance[5]; // Current distance for each of the 5 IR sensors (after filtering and conversion)  

// Look up table (ADC value from analogRead -> distance) for IR sensor 0 (front sensor)
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
// Look up table (ADC value from analogRead -> distance) for IR sensor 1 (right front sensor)
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
// Look up table (ADC value from analogRead -> distance) for IR sensor 2 (right back sensor)
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
// Look up table (ADC value from analogRead -> distance) for IR sensor 3 (left back sensor)
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
// Look up table (ADC value from analogRead -> distance) for IR sensor 4 (left front sensor)
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

// Top speed: analogWrite(*some pin*, 255)
// No speed: analogWrite(*some pin*, 0)

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
 
  analogWrite(EN_R, 150); // motor speed right
  analogWrite(EN_L, 150); // motor speed left 
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
 }
}

void read_serial()
{
    // (the RPI always send the same number of bytes once it sends something, eventhough there's never more than three data bytes that actually should be read. We do this in order to get a fix protocol, which just makes life easier IMO)
    int no_of_bytes_waiting = Serial.available();
    if (no_of_bytes_waiting > 6) // the RPI sends 7 bytes at the time (5 data, 2 control)
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
            
            // read the received checksum:
            uint8_t checksum = Serial.read();
            
            // calculate checksum for the received data bytes:
            uint8_t calc_checksum = manual_state_byte + mode_byte + Kp_byte + 
                Kd_low_byte + Kd_high_byte;
                
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
                        AUTO_STATE = CORRIDOR;
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
            }
            
            else // if the checksums doesn't match: something weird has happend during transmission: flush input bufer and stat over
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

void read_IR_sensors()
{
    // Read analog pin 0 - 4 (A0 - A4) (analogRead returns result of 10-bit ADC, an int between 0 and 1023)
    for (int sensor_ID = 0; sensor_ID <= 4; ++sensor_ID)
    {
        IR_latest_reading[sensor_ID] = analogRead(sensor_ID);
    }
}

float get_median(int array[], int no_of_elements)
{
    float median;
    
    // Sort array using Bubble sort:
    for (int i = 0; i < no_of_elements - 1; ++i) // for no_of_elements - 1 times: (no of needed cycles in the worst case)
    {
        for (int k = 0; k < no_of_elements - (i + 1); ++k)
        {
            if (array[k] > array[k + 1])
            {
                // Swap positions:
                int temp_container = array[k];
                array[k] = array[k + 1];
                array[k + 1] = temp_container;
            }
        }
    }
    
    // Get median value (two different cases depending on whether even no of elements or not)
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

void filter_IR_values()
{
    for (int sensor_ID = 0; sensor_ID <= 4; ++sensor_ID) // for IR0 to IR4:
    {
        // Shift out the oldest reading:
        for (int reading_index = 9; reading_index >= 1; --reading_index)
        {
            IR_reading[sensor_ID][reading_index] = IR_reading[sensor_ID][reading_index - 1];
        }
        
        // Insert the latest reading:
        IR_reading[sensor_ID][0] = IR_latest_reading[sensor_ID];
        
        // Get the median value of the 10 latest readings: (filter sensor readings)
        int size = sizeof(IR_reading[sensor_ID])/sizeof(int); // We get the number of elements (ints) in the array by dividing the size of the array (in no of bytes) with the size of one int (in no of bytes)
        int array_copy[] = {IR_reading[sensor_ID][0], IR_reading[sensor_ID][1], // Need to make a copy here, "get_median" apparently does something weird with its passed array 
                            IR_reading[sensor_ID][2], IR_reading[sensor_ID][3], 
                            IR_reading[sensor_ID][4], IR_reading[sensor_ID][5], 
                            IR_reading[sensor_ID][6], IR_reading[sensor_ID][7], 
                            IR_reading[sensor_ID][8], IR_reading[sensor_ID][9]}; 
        IR_median[sensor_ID] = get_median(array_copy, size);
    }
}

float lookup_distance(ADC_distance_pair ADC_dist_table[], float ADC_value, int table_size)
{
    // Return minimum value if "ADC_value" is smaller than the smallest table value:
	if(ADC_value <= ADC_dist_table[0].ADC_value)	
    {
        return ADC_dist_table[0].distance;
    }

    // Return maximum value if "ADC_value" is greater than the biggest table value:
	if(ADC_value >= ADC_dist_table[table_size-1].ADC_value)
	{
        return ADC_dist_table[table_size-1].distance;
    }

    // Linear interpolation:
	for(int i = 0; i < table_size-1; i++)
	{
        // Interpolate (linearly) if "ADC_value" is between two table values:
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

void convert_IR_values()
{
    IR_distance[0] = lookup_distance(IR0_table, IR_median[0], 15);
  	IR_distance[1] = lookup_distance(IR1_table, IR_median[1], 13);
  	IR_distance[2] = lookup_distance(IR2_table, IR_median[2], 15);
  	IR_distance[3] = lookup_distance(IR3_table, IR_median[3], 12);
  	IR_distance[4] = lookup_distance(IR4_table, IR_median[4], 15);
}

void move_forward_with_control(float speed, float alpha_value)
{
  // right side forward:
  digitalWrite(IN1_R, LOW);
  digitalWrite(IN2_R, HIGH);
 
  // left side forward:
  digitalWrite(IN1_L, HIGH);
  digitalWrite(IN2_L, LOW);

  float right_speed = speed - alpha_value;
  float left_speed = speed + alpha_value;
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
  
  analogWrite(EN_R, right_speed); // motor speed right
  analogWrite(EN_L, left_speed); // motor speed left 
  //Serial.println(alpha_value);
}

void calculate_Yaw()
{
	float l_delta_right = IR_distance[1] - IR_distance[2];
	float l_delta_left = IR_distance[3] - IR_distance[4];

    // Calculate the angle relative the corridor walls, based on the right side IR sensors:
    IR_Yaw_right = (atan(l_delta_right/IR_sensor_distance_right)/3.1415926)*180;	
    // Calculate the angle relative the corridor walls, based on the left side IR sensors:
    IR_Yaw_left = (atan(l_delta_left/IR_sensor_distance_left)/3.1415926)*180;
    
    // Take "Yaw" to be the mean:
    Yaw = (IR_Yaw_right + IR_Yaw_left)/2;
    // Convert to radians (to use in control algorithms, in degrees for display to user):
    Yaw_rad = (Yaw/180)*3.1415926;
}

void calculate_p_part()
{
    p_part = IR_distance[2] - IR_distance[4];
}

void calculate_alpha()
{
    if ((IR_Yaw_right - IR_Yaw_left > 15) || (IR_Yaw_right - IR_Yaw_left < -15)) // if they differ to much we're probably just on our way out of a juction and should not do any adjustments
    {
        alpha = 0;
    }
    else
    {
        alpha = Kp*p_part + Kd*Yaw_rad;
    }
}

void update_state()
{
    float IR_0 = IR_distance[0];
    float IR_1 = IR_distance[1];
    float IR_2 = IR_distance[2];
    float IR_3 = IR_distance[3];
    float IR_4 = IR_distance[4];
    
    switch (AUTO_STATE)
	{
        case DEAD_END:
        {
            if ((IR_0 > 50) && (IR_1 < CORRIDOR_SIDE_DISTANCE) && (IR_2 < CORRIDOR_SIDE_DISTANCE)
            && (IR_3 < CORRIDOR_SIDE_DISTANCE) && (IR_4 < CORRIDOR_SIDE_DISTANCE))
            {
                AUTO_STATE = CORRIDOR;
            }
            
            break;
        }
        
		case CORRIDOR:
		{
			if ((IR_0 < DEAD_END_DISTANCE) && (IR_1 < CORRIDOR_SIDE_DISTANCE)
            && (IR_2 < CORRIDOR_SIDE_DISTANCE) && (IR_3 < CORRIDOR_SIDE_DISTANCE)
            && (IR_4 < CORRIDOR_SIDE_DISTANCE))
            {
                AUTO_STATE = DETERMINE_IF_TARGET;
                cycle_count = 0;
            }
            
            if ((IR_1 > CORRIDOR_SIDE_DISTANCE) || (IR_2 > CORRIDOR_SIDE_DISTANCE) 
            || (IR_3 > CORRIDOR_SIDE_DISTANCE - 10) || (IR_4 > CORRIDOR_SIDE_DISTANCE))
            {
                AUTO_STATE = OUT_OF_CORRIDOR;
                cycle_count = 0;
            }
            
			break;
		}
        
		case DETERMINE_IF_TARGET:
		{
			if (cycle_count > 20)
            {
                AUTO_STATE = DEAD_END;
                cycle_count = 0;
            }
            
			break;
		}
        
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
			
        case INTO_JUNCTION:
        {
            if (cycle_count > 5)
            {
                AUTO_STATE = DETERMINE_JUNCTION;
                cycle_count = 0;
            }
            break;
        }
        
        case DETERMINE_JUNCTION:
        {
            if ((cycle_count > 20) && (IR_0 < CORRIDOR_SIDE_DISTANCE)
            && (IR_1 > CORRIDOR_SIDE_DISTANCE) && (IR_2 > CORRIDOR_SIDE_DISTANCE) 
            && (IR_3 < CORRIDOR_SIDE_DISTANCE) && (IR_4 < CORRIDOR_SIDE_DISTANCE))
            {
                AUTO_STATE = JUNCTION_A_R;
                cycle_count = 0;
            }
            
            else if ((cycle_count > 20) && (IR_0 < CORRIDOR_SIDE_DISTANCE)
            && (IR_1 < CORRIDOR_SIDE_DISTANCE) && (IR_2 < CORRIDOR_SIDE_DISTANCE) 
            && (IR_3 > CORRIDOR_SIDE_DISTANCE-10) && (IR_4 > CORRIDOR_SIDE_DISTANCE))
            {
                AUTO_STATE = JUNCTION_A_L;
                cycle_count = 0;
            }
            
            else if ((cycle_count > 20) && (IR_0 > CORRIDOR_SIDE_DISTANCE)
            && (IR_1 < CORRIDOR_SIDE_DISTANCE) && (IR_2 < CORRIDOR_SIDE_DISTANCE) 
            && (IR_3 > CORRIDOR_SIDE_DISTANCE-10) && (IR_4 > CORRIDOR_SIDE_DISTANCE))
            {
                AUTO_STATE = JUNCTION_B_L;
                cycle_count = 0;
            }

            else if ((cycle_count > 20) && (IR_0 > CORRIDOR_SIDE_DISTANCE)
            && (IR_1 > CORRIDOR_SIDE_DISTANCE) && (IR_2 > CORRIDOR_SIDE_DISTANCE) 
            && (IR_3 < CORRIDOR_SIDE_DISTANCE) && (IR_4 < CORRIDOR_SIDE_DISTANCE))
            {
                AUTO_STATE = JUNCTION_B_R;
                cycle_count = 0;
            }
            
            else if ((cycle_count > 20) && (IR_0 < CORRIDOR_SIDE_DISTANCE)
            && (IR_1 > CORRIDOR_SIDE_DISTANCE) && (IR_2 > CORRIDOR_SIDE_DISTANCE) 
            && (IR_3 > CORRIDOR_SIDE_DISTANCE-10) && (IR_4 > CORRIDOR_SIDE_DISTANCE))
            {
                AUTO_STATE = JUNCTION_C;
                cycle_count = 0;
            }
            
            break;
        }
        
        case JUNCTION_A_R:
        {
            if ((IR_0 > 35) && ((IR_1> 30) || (IR_2 > 30)))
            {
                AUTO_STATE = OUT_OF_JUNCTION;
                cycle_count = 0;
            }
            
            break;
        }
        
        case JUNCTION_A_L:
        {
            if ((IR_0 > 35) && ((IR_3 > 30) || (IR_4 > 30)))
            {
                AUTO_STATE = OUT_OF_JUNCTION;
                cycle_count = 0;
            }
            
            break;
        }
        
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
        
        case JUNCTION_B_L:
        {
            if ((IR_0 > 35) && ((IR_1 > 30) || (IR_2 > 30)) && ((IR_3 > 30) || (IR_4 > 30)))
            {
                AUTO_STATE = OUT_OF_JUNCTION;
                cycle_count = 0;
            }
            
            break;
        }
        
        case JUNCTION_C:
        {
            if ((IR_0 > 35) && (IR_1 < CORRIDOR_SIDE_DISTANCE) && (IR_2 < CORRIDOR_SIDE_DISTANCE))
            {
                AUTO_STATE = OUT_OF_JUNCTION;
                cycle_count = 0;
            }

            break;
        }
        
        case JUNCTION_D:
        {
            
            break;
        }
        
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
		
		default:
        {
            break;
        }
	}   
}

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
        
        case DETERMINE_IF_TARGET:
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
        
        case JUNCTION_C:
        {
            move_left();
            ++cycle_count;
            break;
        }
        
        case JUNCTION_D:
        {
            move_left();
            ++cycle_count;
            break;
        }
        
        case OUT_OF_JUNCTION:
        {
            move_forward_with_control(100, 0);
            ++cycle_count;
            break;
        }
		
		default:
        {
            break;
        }
	}
}

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
        Kd_low_byte + Kd_high_byte + AUTO_STATE + manual_state + mode;
    
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
    
    if (startup_counter < 40) // wait approx 2 secs
    {
        ++startup_counter;
    }
    
    else if (mode == manual_int)
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
