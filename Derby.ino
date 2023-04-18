#include <Wire.h> // Enable this line if using Arduino Uno, Mega, etc.
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

Adafruit_7segment matrix1 = Adafruit_7segment(); //Wire C to A5 or SCL Pin 28
Adafruit_7segment matrix2 = Adafruit_7segment(); //Wire D to A4 or SDA Pin 27

#include <FastLED.h>

#define LED_PIN1     7 //pins for LED strips
#define LED_PIN2     8

#define NUM_LEDS    20 //Number of LEDs in each strip

CRGB leds1[NUM_LEDS]; //Create arrays of LEDS to match the LED strips 
CRGB leds2[NUM_LEDS];

byte regData;     //Pins D0 - D7 'D' register input data
int i; //loop variables
int j;
int k;                 
int ir2 = 2; //Lane 2 IR diodes + leg connected to ground through 10k resistor 
unsigned long lane2Time; // stores the internal clock time when lane 2 finishes the race
int ir3 = 3; // Lane 1
unsigned long lane1Time; // stores the internal clock time when lane 1 finishes the race
int limSwitch = A0;
unsigned long startTime; //Stores the starting time of the race
int carCount;  //count how many cars have finished
int changeHouse = A1; //blue 5
int changeLane = A2; // yellow 4
int lane = 0; //variable for which lane is currently selected
//         s1   s2  Bd1  Bd2  frb  frb  BR   BR    Sas  Sas
int R[] = {10,  10, 15,  80,  0,   0,   60,  60,   100, 10}; // These arrays store the RGB values for the house colors
int G[]=  {10,  5,  5,   20,  0,   0,   30,  0,    0,   0};
int B[]=  {10,  10, 0,   0,   20,  120, 0,   0,    0,   0};
int r1 = 0; //These variables determine which colors from the arrays are displayed
int g1 = 0; // Each house has two colors (R1, G1, B1) and (R2, G2, B2)
int b1 =0;
int r2 = 0;
int g2 = 0;
int b2 =0;
int cycle = 0; // This shifts the colors along the LED strip to create a scrolling effect

void setup() {
  //Serial.begin(9600);
  pinMode(ir2, INPUT); //IR sensor on pin 2
  pinMode(ir3, INPUT); //IR sensor on pin 3
  pinMode(limSwitch, INPUT_PULLUP);  // limSwitch is 1 when not pressed and 0 when pressed
  pinMode(changeHouse, INPUT_PULLUP);  //For the botton that changes the hous colors LEDs
  pinMode(changeLane, INPUT_PULLUP); // Changes which lane you are changing the colors for
  matrix1.begin(0x70); //Begin writting to the seven segment display
  matrix2.begin(0x71); //Begin writting to the other seven segment display
  FastLED.addLeds<WS2812, LED_PIN1, GRB>(leds1, NUM_LEDS); //Begin writting to the LED strip
  FastLED.addLeds<WS2812, LED_PIN2, GRB>(leds2, NUM_LEDS); //Begin writing to the other LED strip
}

void loop() {
  // Set the LED displays to 0000
  matrix1.print(0000);
  matrix1.writeDisplay();  

  matrix2.print(0000);
  matrix2.writeDisplay();  
  
  while(digitalRead(limSwitch) == 1) {  // While the limit switch is not pressed 
    if(digitalRead(changeLane) == 0) {  // If the lane change button is pressed change the selected lane
      lane++;
    delay(200);  // Delay to prevent accidentially changing lanes twice        
    }
    if(digitalRead(changeHouse) == 0) {      // If the change house button is pressed    
      if(lane % 2 == 0){                     // Change the colors of the selected lane
          r1 = (r1 + 2)%10;   
          g1 = (g1 + 2)%10;
          b1 = (b1 + 2)%10;                          
      }
      else{
          r2 = (r2 + 2) % 10;   
          g2 = (g2 + 2) % 10;
          b2 = (b2 + 2) % 10;                          
      }
      delay(200);
    }
    if(digitalRead(limSwitch) == 0) break; //If the limit switch gets pressed leave this loop
    for (i = 0; i <= NUM_LEDS; i + 0) { //write RGB data to LED strip 1
      for (j = 0; j <= 4; j++) {
        leds1[(i + cycle) % 20] = CRGB ( R[r1], G[g1], B[b1]);
        leds2[(i + cycle) % 20] = CRGB ( R[r2], G[g2], B[b2]);
        FastLED.show();
        i++;
        if(digitalRead(limSwitch) == 0) break; //If the limit switch gets pressed leave this loop
      }
      if(digitalRead(limSwitch) == 0) break; //If the limit switch gets pressed leave this loop
      for (j = 0; j <= 4; j++) { // Write RGB data to LED strip 2
        leds1[(i + cycle) % 20] = CRGB ( R[r1 + 1], G[g1 + 1], B[b1 + 1]);
        leds2[(i + cycle) % 20] = CRGB ( R[r2 + 1], G[g2 + 1], B[b2 + 1]);
        FastLED.show();
        i++;
        if(digitalRead(limSwitch) == 0) break; //If the limit switch gets pressed leave this loop
      }
      if(digitalRead(limSwitch) == 0) break; //If the limit switch gets pressed leave this loop
      cycle++;
      delay(10);
    }
  }
    
  //wait until the limit switch is pressed
  //Serial.println("Click!");  
  startTime = millis(); //record the time in the internal clock for timing
  //Serial.println(startTime);
  carCount = 0;  //How many cars have crossed the finish line
  lane1Time = 0; //When did lane one finish
  lane2Time = 0; //When did lane 2 finish

  while(carCount < 2) { //While less than 2 cars have crossed the finish line
    regData = PIND; // Read pins D0 - D7
    if (lane1Time == 0){ // If car 1 has not finished
      if(!(regData & 0x08)){   //Check if the ir sensor in lane one has triggered
        lane1Time = millis(); //If it has record the time
        //Serial.println(lane2Time);         
        carCount++; //Count that one car has finished
      }
    }
    if (lane2Time == 0){ //If car 2 has not finished
      if(!(regData & 0x04)){  //Check if the ir sensor in lane two has triggered
        lane2Time = millis(); //If it has record the time
        //  Serial.println(lane3Time);
        carCount++; //count that a car has finished
      }
    }
    if((millis() - startTime) > 9999) { //if 10 seconds have elapsed since the race start finish the race
      carCount++; 
    }
  }

  if(lane2Time != 0){ //if lane 2 finished calculate the race time
    lane2Time = lane2Time - startTime;
  }
  if (lane1Time != 0){ // If lane one finshed calculate the race time
    lane1Time = lane1Time - startTime;
  }

  if(lane1Time == 0){ // if car didn't finish write DNF to the display
    matrix1.print("DNF");
    matrix1.writeDisplay();
    lane1Time = 10000;
  } else {            // If car did finish write the time to the seven segemnt display
    matrix1.writeDigitNum(0, (lane1Time / 1000), true);
    matrix1.writeDigitNum(1, (lane1Time / 100) % 10, false);
    matrix1.writeDigitNum(3, (lane1Time / 10) % 10, false);
    matrix1.writeDigitNum(4, lane1Time % 10, false); 
    matrix1.writeDisplay();
  }
  if(lane2Time == 0){  // if car didn't finish write DNF to the display
    matrix2.print("DNF");
    matrix2.writeDisplay();
    lane2Time = 10000;
  } else {            // If car did finish write the time to the seven segemnt display
    matrix2.writeDigitNum(0, (lane2Time / 1000), true);
    matrix2.writeDigitNum(1, (lane2Time / 100) % 10, false);
    matrix2.writeDigitNum(3, (lane2Time / 10) % 10, false);
    matrix2.writeDigitNum(4, lane2Time % 10, false); 
    matrix2.writeDisplay();
  }
  
  if(lane1Time < lane2Time) {  //Determine who won.  make the winners LEDs bright and the losers dim
    for (i = 0; i <= NUM_LEDS; i++) {
      leds2[i] = CRGB ( R[r2]/10, G[g2]/10, B[b2]/10);
      FastLED.show();
    }
    for(k = 0; k < 10; k++) {
      for (i = 0; i <= NUM_LEDS; i++) {
        leds1[i] = CRGB ( 0, 0, 0);
        FastLED.show();    
      }
      delay(100);              
      for (i = 0; i <= NUM_LEDS; i + 0) {
        for (j = 0; j <= 4; j++) {
          leds1[(i + cycle) % 20] = CRGB ( R[r1], G[g1], B[b1]);
          FastLED.show();
          i++;
        }
        for (j = 0; j <= 4; j++) {
          leds1[(i + cycle) % 20] = CRGB ( R[r1 + 1], G[g1 + 1], B[b1 + 1]);
          FastLED.show();
          i++;
        }
      }
      delay(100); 
    } 
  } else if(lane2Time < lane1Time) {
      for (i = 0; i <= NUM_LEDS; i++) {
        leds1[i] = CRGB ( R[r1]/10, G[g1]/10, B[b1]/10);
        FastLED.show(); 
      }   
      for(k = 0; k < 10; k++) {
        for (i = 0; i <= NUM_LEDS; i++) {
          leds2[i] = CRGB ( 0, 0, 0);
          FastLED.show();    
        }
       delay(100);              
        for (i = 0; i <= NUM_LEDS; i + 0) {
         for (j = 0; j <= 4; j++) {
          leds2[(i + cycle) % 20] = CRGB ( R[r2], G[g2], B[b2]);
          FastLED.show();
          i++;
        }
        for (j = 0; j <= 4; j++) {
          leds2[(i + cycle) % 20] = CRGB ( R[r2 + 1], G[g2 + 1], B[b2 + 1]);
          FastLED.show();
          i++;
        }
      }
      delay(100);
    } 
    } else {
      for (i = 0; i <= NUM_LEDS; i++) {
        leds1[i] = CRGB ( R[r1]/10, G[g1]/10, B[b1]/10);
        leds2[i] = CRGB ( R[r2]/10, G[g2]/10, B[b2]/10);
        FastLED.show(); 
      }
    }
  
  
  while(digitalRead(changeHouse) == 1) {}  //wait until the changeHouse switch is pressed to stop showing the times
  delay(200);
}