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

int manual_state = stop_int;

// Top speed: analogWrite(*some pin*, 255)
// No speed: analogWrite(*some pin*, 0)

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
 
  analogWrite(EN_R, 180); // motor speed right
  analogWrite(EN_L, 180); // motor speed left 
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
    if (serial_input == stop_int || serial_input == forward_int
        || serial_input == backward_int || serial_input == right_int
        || serial_input == left_int)
    {
      manual_state = serial_input;
    }
    
    else
    {
      // stop the robot if there's something wrong with the serial 
      // transmission (this shouldn't normally happen)
      manual_state = stop_int;
    }
    
  }
  
}

///////////////////////////////////////////////////////////////////////
// main loop
///////////////////////////////////////////////////////////////////////
void loop()
{  
 read_serial();
 run_manual_state();
 
 delay(50);  // delay for loop frequency of roughly 20 Hz
}





