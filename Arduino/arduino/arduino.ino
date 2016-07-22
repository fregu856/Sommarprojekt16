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

int Kp = 4;
int Kd = 350;

int manual_state = stop_int;
int mode = manual_int;

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
 
  analogWrite(EN_R, 200); // motor speed right
  analogWrite(EN_L, 200); // motor speed left 
}

void move_backward()
{
  // right side backward
  digitalWrite(IN1_R, HIGH);
  digitalWrite(IN2_R, LOW);
 
  // left side backward
  digitalWrite(IN1_L, LOW);
  digitalWrite(IN2_L, HIGH);
 
  analogWrite(EN_R, 200); // motor speed right
  analogWrite(EN_L, 200); // motor speed left 
}

void move_right()
{
  // right side backward
  digitalWrite(IN1_R, HIGH);
  digitalWrite(IN2_R, LOW);
 
  // left side forward
  digitalWrite(IN1_L, HIGH);
  digitalWrite(IN2_L, LOW);
 
  analogWrite(EN_R, 180); // motor speed right
  analogWrite(EN_L, 180); // motor speed left 
}

void move_left()
{
  // right side forward
  digitalWrite(IN1_R, LOW);
  digitalWrite(IN2_R, HIGH);
 
  // left side backward
  digitalWrite(IN1_L, LOW);
  digitalWrite(IN2_L, HIGH);
 
  analogWrite(EN_R, 190); // motor speed right
  analogWrite(EN_L, 170); // motor speed left 
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
    if (Serial.available())
    {
        int serial_input = Serial.read() - '0';
        Serial.println(serial_input);
        
        // if in manual mode:
        if (mode == manual_int)
        {
            // Change manual state if requested:
            if (serial_input == stop_int || serial_input == forward_int
            || serial_input == backward_int || serial_input == right_int
            || serial_input == left_int)
            {
                manual_state = serial_input;
            }
            
            // Change to auto mode if requested:
            else if (serial_input == auto_int)
            {
                mode = auto_int; // set the mode to auto
            }
            
            // Do nothing if manual is requested again:
            else if (serial_input == manual_int)
            {
                
            }
            
            else if (serial_input == Kp_and_Kd_int)
            {
                Kp = Serial.read() - '0';
                Kd = Serial.read() - '0';
            }
            
            else if (serial_input == Kp_int)
            {
                Kp = Serial.read() - '0';
            }
            
            else if (serial_input == Kd_int)
            {
                Kd = Serial.read() - '0';
            }
            
            else
            {
                manual_state = stop_int; // if something is weird: stop (this shouldn't happen)
            }
        }

        // else if in auto mode:
        else if (mode == auto_int)
        {
            // Change to manual mode if requested:
            if (serial_input == manual_int)
            {
                mode = manual_int;
                manual_state = stop_int; // don't move in the very first loop after a mode change
            }
            
            // Do nothing if auto is requested again:
            else if (serial_input == auto_int)
            {
                
            }
            
            // Do nothing if manual state is requested in auto:
            else if (serial_input == stop_int || serial_input == forward_int
            || serial_input == backward_int || serial_input == right_int
            || serial_input == left_int)
            {
                
            }
            
            // Do nothing if new control parameters are requested in auto: (only possible in manual)
            else if (serial_input == Kp_and_Kd_int || serial_input == Kp_int
            || serial_input == Kd_int)
            {
                
            }
            
            // If something is weird, stop (this shouldn't happen):
            else
            {
                mode = manual_int;
                manual_state = stop_int; 
            }
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

void move_forward_with_control(float alpha_value)
{
  // right side forward:
  digitalWrite(IN1_R, LOW);
  digitalWrite(IN2_R, HIGH);
 
  // left side forward:
  digitalWrite(IN1_L, HIGH);
  digitalWrite(IN2_L, LOW);

  float right_speed = 150 - alpha_value;
  float left_speed = 150 + alpha_value;
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
    alpha = Kp*p_part + Kd*Yaw_rad;
}

void test()
{
    //Serial.println(IR_distance[0]);
    //Serial.println(IR_Yaw_left);
    //Serial.println(IR_Yaw_right);
    //Serial.println(alpha);
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
    
    if (mode == manual_int)
    {
        run_manual_state();
    }
    
    else if (mode == auto_int)
    {
        read_IR_sensors();
        filter_IR_values();
        convert_IR_values();
        calculate_Yaw();
        calculate_p_part();
        calculate_alpha();
        move_forward_with_control(alpha);
        test();
    }
    
    delay(50);  // delay for loop frequency of roughly 20 Hz
}
