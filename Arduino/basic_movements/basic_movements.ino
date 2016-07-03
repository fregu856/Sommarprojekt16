int EN_H = 10;
int EN_V = 5;
int IN1_H = 7;
int IN2_H = 8;
int IN1_V = 4;
int IN2_V = 2;
void setup()
{
 pinMode(EN_H,OUTPUT);  
 pinMode(EN_V,OUTPUT);
 pinMode(IN1_H,OUTPUT);  
 pinMode(IN2_H,OUTPUT);
 pinMode(IN1_V,OUTPUT);
 pinMode(IN2_V,OUTPUT);   
}
void loop()
{  

 // Drive BACKWARD (RIGHT): 
 //digitalWrite(IN1_H,HIGH);
 //digitalWrite(IN2_H,LOW);
 //analogWrite(EN_H, 20); // motor speed    
 //delay(5000);

 // Drive FORWARD (RIGHT):
 //digitalWrite(IN1_H,LOW);
 //digitalWrite(IN2_H,HIGH);
 //analogWrite(EN_H, 200); // motor speed   
 //delay(5000);

 // Drive FORWARD (LEFT): 
 //digitalWrite(IN1_V,HIGH);
 //digitalWrite(IN2_V,LOW);
 //analogWrite(EN_V, 200); // motor speed  
 //delay(5000);

 // Drive BACKWARD (LEFT): 
 //digitalWrite(IN1_V,LOW);
 //digitalWrite(IN2_V,HIGH);
 //analogWrite(EN_V, 200); // motor speed   
 //delay(5000);


 // Drive FORWARD:
 digitalWrite(IN1_H,LOW);
 digitalWrite(IN2_H,HIGH);
 
 digitalWrite(IN1_V,HIGH);
 digitalWrite(IN2_V,LOW);
 
 analogWrite(EN_H, 250); // motor speed right
 analogWrite(EN_V, 250); // motor speed left  
 delay(5000);

 // Stop
 //digitalWrite(IN1_H,LOW);
 //digitalWrite(IN2_H,LOW);
 //digitalWrite(IN1_V,LOW);
 //digitalWrite(IN2_V,LOW);
 //delay(5000);
 
 // Drive BACKWARD:
 //digitalWrite(IN1_H,HIGH);
 //digitalWrite(IN2_H,LOW);
 
 //digitalWrite(IN1_V,LOW);
 //digitalWrite(IN2_V,HIGH);
 
 //analogWrite(EN_H, 250); // motor speed right
 //analogWrite(EN_V, 250); // motor speed left  
 //delay(5000);

  // Stop
 //digitalWrite(IN1_H,LOW);
 //digitalWrite(IN2_H,LOW);
 //digitalWrite(IN1_V,LOW);
 //digitalWrite(IN2_V,LOW);
 //delay(5000);

 
}
