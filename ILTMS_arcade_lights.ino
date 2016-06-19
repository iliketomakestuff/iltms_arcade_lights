
#include <FastLED.h>

/////////////////////////////
//VARS
//the time we give the sensor to calibrate (10-60 secs according to the datasheet)
int calibrationTime = 10;        

//the time when the sensor outputs a low impulse
long unsigned int lowIn;         

//the amount of milliseconds the sensor has to be low 
//before we assume all motion has stopped
long unsigned int pause = 120000;  

boolean lockLow = true;
boolean takeLowTime;  

int pirPin = 2;    //the digital pin connected to the PIR sensor's output
int relayPin = 4;
int buttonPin = 8;

#define MARQUEE_PIN     11
#define BACKLIGHT_PIN     12

#define COLOR_ORDER GRB
#define CHIPSET     WS2812B
#define NUM_LEDS    150

#define initBRIGHTNESS  40
#define BRIGHTNESS  100
#define FRAMES_PER_SECOND 500

int MARQcolor = CRGB::White;
int BACKcolor = CRGB::Blue;
int lightState = 0; //0 off, 1, fading up, 2 on, 3 fading down
int fadeStep = 0;
int fadeStepSize = 2;
bool isAnimating = 0;
      
CRGB MARQ_leds[NUM_LEDS];
CRGB BL_leds[NUM_LEDS];
  
/////////////////////////////
//SETUP
void setup(){
  Serial.begin(9600);
  pinMode(buttonPin, INPUT);
  pinMode(pirPin, INPUT);
  pinMode(relayPin, OUTPUT); 
  pinMode(MARQUEE_PIN, OUTPUT);
  pinMode(BACKLIGHT_PIN, OUTPUT);
  digitalWrite(pirPin, LOW);
  FastLED.addLeds<CHIPSET, BACKLIGHT_PIN, COLOR_ORDER>(BL_leds, NUM_LEDS).setCorrection( TypicalLEDStrip );

  FastLED.addLeds<CHIPSET, MARQUEE_PIN, COLOR_ORDER>(MARQ_leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( initBRIGHTNESS );

  if(digitalRead(buttonPin) == HIGH){
    
    digitalWrite(relayPin,LOW);
for( int j = 0; j < 20; j++) {
      MARQ_leds[j] = CRGB::Orange;
      BL_leds[j] = CRGB::Orange;
    }
    
    setAllBrightNess(10);
    FastLED.show();

  //give the sensor some time to calibrate
  Serial.print("calibrating sensor ");
    for(int i = 0; i < calibrationTime; i++){
      Serial.print(".");
      delay(1000);
      }
    Serial.println(" done");
    Serial.println("SENSOR ACTIVE");
    setAllBrightNess(0);
    
    delay(50);
  } else {
     digitalWrite(relayPin,HIGH);
  }
  }

////////////////////////////
//LOOP
void loop(){
if(digitalRead(buttonPin) == HIGH){
  // turn on sensors
     if(digitalRead(pirPin) == HIGH){
       if(lockLow){  
         //makes sure we wait for a transition to LOW before any further output is made:
         lockLow = false;            
         Serial.println("---");
         Serial.print("motion detected at ");
         Serial.print(millis()/1000);
         Serial.println(" sec"); 
         if(lightState  != 2){
          lightState = 1;
         }
         digitalWrite(relayPin,HIGH);
         delay(50);
         }         
         takeLowTime = true;
       }

     if(digitalRead(pirPin) == LOW){ 
       if(takeLowTime){
        lowIn = millis();          //save the time of the transition from high to LOW
        takeLowTime = false;       //make sure this is only done at the start of a LOW phase
        }
       //if the sensor is low for more than the given pause, 
       //we assume that no more motion is going to happen
       if(!lockLow && millis() - lowIn > pause){  
           //makes sure this block of code is only executed again after 
           //a new motion sequence has been detected
           lockLow = true;                        
           Serial.print("motion ended at ");      //output
           Serial.print((millis() - pause)/1000);
           Serial.println(" sec");
           if(lightState != 0){
            lightState= 3;
           }
           digitalWrite(relayPin,LOW);
           FastLED.show();
           delay(50);
           }
       }

    switch(lightState){
    case 0:
    //light is off
      
      break;
    case 1:
      //light is animating up
      lightIsAnimatingUp();
      break;
    case 2:
      //light is on
      lightIsOn();
      break;
    case 3:
      //light is animating down
      lightIsAnimatingDown();
      break;
    }
  FastLED.show(); // display this frame
  
  } else {
    //sensors are off.. just light up. 
  digitalWrite(relayPin,HIGH);

    for( int j = 0; j < NUM_LEDS; j++) {
      MARQ_leds[j] = MARQcolor;
      BL_leds[j] = BACKcolor;
    }
    setAllBrightNess(BRIGHTNESS);
    FastLED.show();
  }
}

void setColors(){
for( int j = 0; j < NUM_LEDS; j++) {
      MARQ_leds[j] = MARQcolor;
      BL_leds[j] = BACKcolor;
    }  
}
void lightIsOn()
{
  setColors();
  delay(100);
}
void lightIsAnimatingUp(){
    fadeStep+=fadeStepSize;
    setColors();
    setAllBrightNess(fadeStep);
    Serial.println(fadeStep);
    delay(50);
    if (fadeStep == BRIGHTNESS){
      isAnimating=0;
      lightState = 2;  
    }
    
    }
    
void lightIsAnimatingDown(){
    fadeStep-=fadeStepSize;
    setColors();
    setAllBrightNess(fadeStep);
    delay(50);
  if (fadeStep == 0){
      isAnimating=0;
      lightState = 0;  
    }
}

int setAllBrightNess(int val){
    FastLED.setBrightness( val ); 
}
