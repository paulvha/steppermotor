/* example to provide your own input for turning the stepper motor
 * 
 * paulvha October 2018 / version 1.0
 */

#define INPUT1  8         // blue connected to pin
#define INPUT2  9         // pink connected to pin
#define INPUT3  10        // yellow connected to pin
#define INPUT4  11        // orange connected to pin

#define High   2000       // speed in us delay
#define Medium 2 * High   // speed in us delay
#define Low    4 * High   // speed in us delay

//define # of steps per cycle
int full_step = false;

////////////////////////////////////////////////////////
/// no changes needed after this point
///////////////////////////////////////////////////////

#define CounterClockwise false // direction
#define Clockwise true         // direction

void setup(){
 
  pinMode(INPUT4, OUTPUT);
  pinMode(INPUT3, OUTPUT);
  pinMode(INPUT2, OUTPUT);
  pinMode(INPUT1, OUTPUT);
  delay(10); 

  // Serial Monitor is used to control the demo
  Serial.begin(9600);
  get_input(F("Press any key to begin."));
}

void loop()
{
  int dir, sp_move, degr;

  // reset values
  dir = sp_move = degr = 0;
  
  // get valid degrees
  while (degr == 0){
    degr = get_input("How many degrees turn ?");
    if (degr < 1 || degr > 360) degr = 0;
  }
 
  // get valid direction
  while (dir == 0){
    dir = get_input("Direction to move: 1 = clock-wise, 2 = counter clock-wise ");
    if (dir < 1 || dir > 2) dir = 0;
  }

  // get valid speed of moving
  while (sp_move == 0){
    sp_move = get_input("How fast to move: 1 = high, 2 = medium, 3 = slow");
    if (sp_move < 1 || sp_move >  3 ) sp_move = 0;
  }

  // display action to perform
  Serial.print ("Perform turning ");
  Serial.print (degr);
  
  if (dir == 1) Serial.print (" degrees, clock-wise with ");
  else Serial.print (" degrees, counter clock-wise with ");

  if(sp_move == 1) Serial.print("high");
  else if(sp_move == 2) Serial.print("medium");
  else Serial.print("low");
   
  if (full_step) Serial.print(" speed using full-step, movement error: ");
  else Serial.print ("speed using half-step, movement error: ");

  Serial.print(margin_error(degr));
  Serial.println ("%");
  
  // now move it.
  if (dir == 1)  turnStepper(Clockwise,sp_move,degr);
  else turnStepper(CounterClockwise,sp_move,degr);

  Serial.println();
}

/* calculate number of steps needed */
float calc_steps(int degr)
{
  float tmp;
  
  tmp = 360 / (float) degr;

  if (full_step) return(round(2048 / tmp));
  else           return(round(4096 / tmp));
}


/*  calculate the movement precision / error depending on
 *  full-step 4 steps
 *  half-step 8 steps  */
float margin_error(int degr)
{
  float result;

  // in case full-step
  if (full_step) result = calc_steps(degr) * (float) 360 / 2048;
  else           result = calc_steps(degr) * (float) 360 / 4096;

  return((result / (float) degr -1) * 100);
}

/*  Display message and expect a number input
 *  which is returned as a int
 */
int get_input(String message)
{
  String inp;

  Serial.println(message);
  
  // wait for input
  while (!Serial.available());
  
  inp = Serial.readString();
    
  return(atoi(inp.c_str()));
}

/*  The specification of the 28BYJ-48 are
 *  
 *  Stride angle  5.625 /64
 *  gear 64
 *  
 *  Half-step mode: 8 step control signal sequence (recommended) 5.625 degrees per step / 64 steps per one revolution of the internal motor shaft
 *  The gear means: 64 internal motor turns are needed for one turn of top-shaft. So 360 degrees turn top-shaft needs 64 * 64 = 4096 pulses
 *  This also means that the top-shaft can be positioned with 5.625 / 64 = 0.08789 degrees precision.
 *
 *  Full Step mode: 4 step control signal sequence 11.25 degrees per step / 32 steps per one revolution of the internal motor shaft
 *  The gear means: 64 motor turn are needed for one turn of top shaft. So 360 degrees turn top-shaft needs 64 * 32 = 2048 pulses  
 *  This also means that the top-shaft can be positioned with 5.625 / 32 = 0,1757 degrees precision.
 *  
 *  MotorInput (blue, pink, yellow, orange)
 *  
 *             ------------CW ---------->>
 *           1   2   3   4   5   6   7   8
 *  orange   Y   X                       X
 *  yellow       X   Y   X
 *  pink                 X   Y   X 
 *  blue                         X   Y   X
 *  
 *  Setting full-step = true will perform the X-steps
 *  Setting full-step = false will perform the X and Y steps
 */
void turnStepper(boolean Direction,int spd,int Angle)
{
   int i, j, RPM;

   RPM = Low;
   if (spd == 1)  RPM = High;
   else if (spd == 2)  RPM = Medium;

   // display how long this will take
   Serial.print("This will take ");
   if (full_step) Serial.print((calc_steps(Angle) * RPM) / 1000);
   else  Serial.print((calc_steps(Angle) * RPM /2) / 1000);  
   Serial.println("ms");
   
   if (full_step)     // 4 step only the X-steps
   {
        
    if(Direction){      // clock wise
    
      for(i=0,j=0;i<map(Angle,0,360,0,2048);i++)
      {
        if      (j == 0) motorInput(1,1,0,0);
        else if (j == 1) motorInput(0,1,1,0);
        else if (j == 2) motorInput(0,0,1,1);
        else             motorInput(1,0,0,1);
        delayMicroseconds(RPM);
        if (++j == 4) j = 0;
      }
    }
     else {           // counter clock
      for(i=0, j=0;i<map(Angle,0,360,0,2048);i++)   
      {
        if      (j == 0) motorInput(1,0,0,1);
        else if (j == 1) motorInput(0,0,1,1);
        else if (j == 2) motorInput(0,1,1,0);
        else             motorInput(1,1,0,0);
        delayMicroseconds(RPM);
        if(++j == 4) j = 0;
      }    
    }
   }
   else // eight steps   // X + Y steps
   {
     if (Direction) {      // clock wise
    
      for(i=0, j=0 ;i<map(Angle,0,360,0,4096);i++)
      {
        if      (j == 0) motorInput(1,0,0,0);
        else if (j == 1) motorInput(1,1,0,0);
        else if (j == 2) motorInput(0,1,0,0);
        else if (j == 3) motorInput(0,1,1,0);
        else if (j == 4) motorInput(0,0,1,0);
        else if (j == 5) motorInput(0,0,1,1);
        else if (j == 6) motorInput(0,0,0,1);
        else             motorInput(1,0,0,1);
        delayMicroseconds(RPM/2 );
        if(++j == 8) j = 0;
      }
     }
   
     else {           // counter clock
      for(i=0, j=0; i<map(Angle,0,360,0,4096);i++)   
      {
        if      (j == 0) motorInput(1,0,0,1);
        else if (j == 1) motorInput(0,0,0,1);
        else if (j == 2) motorInput(0,0,1,1);
        else if (j == 3) motorInput(0,0,1,0);
        else if (j == 4) motorInput(0,1,1,0);
        else if (j == 5) motorInput(0,1,0,0);
        else if (j == 6) motorInput(1,1,0,0);
        else             motorInput(1,0,0,0);
        
        delayMicroseconds(RPM/2);
        if(++j == 8) j = 0;
      }    
    }
   }
      
   // turn off motor
   motorInput(0,0,0,0);
   
   delay(10);
}

/* set signals on the motor */
void motorInput(boolean IN1,boolean IN2,boolean IN3,boolean IN4)
{
    digitalWrite(INPUT1, IN1);    // blue
    digitalWrite(INPUT2, IN2);    // pink
    digitalWrite(INPUT3, IN3);    // yellow
    digitalWrite(INPUT4, IN4);    // orange
}

