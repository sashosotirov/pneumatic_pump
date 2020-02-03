#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 9
Adafruit_SSD1306 display(OLED_RESET);

float targetPressure = 0;
float pressure = 0;

const int pumpPin =  A1;  
const int solPin =  A2;
const int sol2Pin = A3;


int start = 7;
int up = 8;
int down = 6;
int kill = 5;
int start_ext = 9;
int stop_ext = 10;

int valFF = 0;
int valF = 0;
int valB = 0;
int valFB = 0;
int valStart = 0;
int valStop = 0;
 
int started = 0;
int stopped = 0;
float fmap(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    
}
float psi(int raw)
{
  float voltage = 4.9 * raw/1023;         //voltage present
  float percent = 100.0 * voltage/5;      //percetange of total voltage

  // handle out of scope range
  if (percent < 10) return -1;
  if (percent > 90) return -2;

  float maxPressure = 100.0;
  float minPressure = 0.0;
  float pressure = fmap(percent, 10, 90, minPressure, maxPressure); // we map 5% to minPressure and 95% to maxPressure

  return pressure; 
}

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  pinMode(pumpPin, OUTPUT);
  pinMode(solPin, OUTPUT);
  pinMode(sol2Pin, OUTPUT);  

  pinMode(start, INPUT);
  pinMode(up, INPUT);
  pinMode(down, INPUT);
  pinMode(kill, INPUT);
  pinMode(start_ext,INPUT_PULLUP);
  pinMode(stop_ext, INPUT_PULLUP);
}

void loop() {
  valFF = digitalRead(start);
  valF = digitalRead(up);
  valB = digitalRead(down);
  valFB = digitalRead(kill);
  valStart = digitalRead(start_ext);
  valStop = digitalRead(stop_ext);

  int raw = 0;
  for (int i=0; i<20; i++)   //takes 20 samples
       { 
         raw += analogRead(A0);
       }
       
  raw /=20;                 //averages 20 samples

  float pressureBAR1 = (psi(raw) + 1) / 10; 

  delay(1);
  
  if (valF == 1)
  {
    targetPressure += 0.1;
    if (targetPressure > 5.0) {
      targetPressure = 5.0;
    }
    delay(100);
  }

  else if (valB == 1)  {
    targetPressure -= 0.1;
    if (targetPressure < 0.0) {
      targetPressure = 0.0;
    }
    delay(100);
  }

  else if (valFF == 1 || valStart == 0) //start
  {
      stopped = 0;
      started = 1;
  }

  else if (valFB == 1 || valStop == 0) //stop
  {
      stopped = 1;
      started = 0;
  }

    if (started == 1){
        if (pressureBAR1 < targetPressure) //start
        {
          analogWrite(pumpPin, 255); //start pump
          delay(10);
          analogWrite(solPin, 255); //open valve cylinder                    
        }      
        else if (pressureBAR1 > targetPressure - 0.1)
        {
          analogWrite(pumpPin, 0); //stop pump
          analogWrite(solPin, 0); //release pump pressure 
        }
 
      }
    
    if (stopped == 1){
        analogWrite(pumpPin, 0); //stop pump
        analogWrite(solPin, 0); //release pump pressure
        if (pressureBAR1 > 0){
          analogWrite(sol2Pin, 255);
          delay(3000);
          analogWrite(sol2Pin, 0);
        }
                
      }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setTextColor(INVERSE);
  display.setCursor(0, 0);
  display.print(F("now: "));
  display.setCursor(68, 0);
  display.print(pressureBAR1, 1);
  display.setCursor(0, 17);
  display.print(F("set: "));
  display.setCursor(68, 17);
  display.print(targetPressure, 1);
  display.display();

}


