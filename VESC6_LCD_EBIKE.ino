/*
This code is used to gather data from a VESC6 and display on an LCD screen.
Written for Arduino Nano and LCD I2C 1602.

Original VESCUART.h code written by SolidGeek.
 */

#include <VescUart.h>
#include <SimpleKalmanFilter.h>

//Library for the Display
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16, 2); 

/** Initiate VescUart class */
VescUart UART;

int rpm;
float voltage;
float current;
int power;
float amphour;
float tach;
float distance;
float velocity;
float watthour;
float batpercentage;

SimpleKalmanFilter Filter1(2, 2, 0.01);

////////// Custom Characters //////////
byte ph[] = {
  B11100,
  B10100,
  B11100,
  B10000,
  B10101,
  B00111,
  B00101,
  B00101
};

byte mi[] = {
  B10001,
  B11011,
  B10101,
  B10001,
  B00100,
  B00000,
  B00100,
  B00110
};

byte km[] = {
  B01000,
  B01010,
  B01100,
  B01010,
  B10001,
  B11011,
  B10101,
  B10001
};


byte ah[] = {
  B00100,
  B01010,
  B01110,
  B01010,
  B00000,
  B01010,
  B01110,
  B01010
};
byte percent[] = {
  B11001,
  B11001,
  B00010,
  B00100,
  B01000,
  B10011,
  B10011,
  B00000
};

byte m[] = {
  B10001,
  B11011,
  B10101,
  B10001,
  B10001,
  B00000,
  B00000,
  B00000
};

byte k[] = {
  B01000,
  B01010,
  B01100,
  B01010,
  B01010,
  B00000,
  B00000,
  B00000
};

byte bat[] = {
  B01110,
  B11011,
  B10001,
  B10001,
  B10001,
  B11111,
  B11111,
  B11111
};

byte sprk[] = {
  B00000,
  B01000,
  B00100,
  B00010,
  B00100,
  B01000,
  B00100,
  B00010
};



void setup() {

  /** Setup Serial port to display data */
  Serial.begin(115200);

  /** Setup UART port (Serial1 on Atmega32u4) */
// Serial1.begin(19200);
  
  while (!Serial) {;}

  /** Define which ports to use as UART */
 UART.setSerialPort(&Serial);

   lcd.init();                      // initialize the lcd 
   lcd.backlight();

  lcd.setCursor(3,0);
  lcd.print("VESC EBIKE");          // Startup screen (can't be more than 10 characters)
  lcd.createChar(5, bat);
  lcd.setCursor(1,1);
  lcd.write(5);
  lcd.createChar(6, bat);
  lcd.setCursor(14,1);
  lcd.write(6);
  lcd.createChar(7, sprk);
  lcd.setCursor(1,0);
  lcd.write(7);
  lcd.createChar(8, sprk);
  lcd.setCursor(14,0);
  lcd.write(8);

  
  delay(5000);
  lcd.clear();

}
void loop() {
  
////////// Read values //////////  
 if ( UART.getVescValues() ) {

  rpm = (UART.data.rpm)/7;          // The '7' is the number of pole pairs in the motor. Most motors have 14 poles, therefore 7 pole pairs
  voltage = (UART.data.inpVoltage);
  current = (UART.data.avgInputCurrent);
  power = voltage*current;
  amphour = (UART.data.ampHours);
  watthour = amphour*voltage;
  tach = (UART.data.tachometerAbs)/42;          // The '42' is the number of motor poles multiplied by 3
  distance = tach*3.142*(1/1609)*0.72*(16/185);          // Motor RPM x Pi x (1 / meters in a mile or km) x Wheel diameter x (motor pulley / wheelpulley)
  velocity = rpm*3.142*(60/1609)*0.72*(16/185);          // Motor RPM x Pi x (seconds in a minute / meters in a mile) x Wheel diameter x (motor pulley / wheelpulley)
  batpercentage = ((voltage-38.4)/12)*100;          // ((Battery voltage - minimum voltage) / number of cells) x 100

////////// Filter //////////
  // calculate the estimated value with Kalman Filter
  float powerfiltered = Filter1.updateEstimate(power);
  

////////// LCD //////////
  
// First line  
  if(velocity < 10){
  lcd.setCursor(1,0);
  lcd.print(" ");
  lcd.print(velocity, 0);
  }
  else{
  lcd.setCursor(1,0);
  lcd.print(velocity, 0);
  }
  lcd.createChar(4, m);          // Change 'm' to 'k' for kilometers
  lcd.setCursor(3,0);
  lcd.write(4);
  lcd.createChar(0, ph);
  lcd.setCursor(4,0);
  lcd.write(0);

  lcd.setCursor(7,0);
  lcd.print(voltage,0);
  lcd.setCursor(9,0);
  lcd.print("V");
  
  lcd.setCursor(11,0);
  lcd.print(distance, 2);
  lcd.createChar(1, mi);          // Change 'mi' to 'km' for kilometers
  lcd.setCursor(15,0);
  lcd.write(1);
  
// Second line
if(powerfiltered < 10){
  lcd.setCursor(0,1);
  lcd.print("   ");
  lcd.print(powerfiltered, 0);
}
if(powerfiltered > 10 && powerfiltered < 100){
  lcd.setCursor(0,1);
  lcd.print("  ");
  lcd.print(powerfiltered, 0);
}
if(powerfiltered > 100 && powerfiltered < 1000){
  lcd.setCursor(0,1);
  lcd.print(" ");
  lcd.print(powerfiltered, 0);
}
if(powerfiltered > 1000){
  lcd.setCursor(0,1);
  lcd.print(powerfiltered, 0);
}
  lcd.setCursor(4,1);
  lcd.print("W");
 
if(batpercentage < 100){
  lcd.setCursor(7,1);
  lcd.print(batpercentage,0);
 }
 else{
  lcd.setCursor(6,1);
  lcd.print(batpercentage,0);
 }
  lcd.createChar(3, percent);
  lcd.setCursor(9,1);
  lcd.write(3);
 
  lcd.setCursor(11,1);
  lcd.print(amphour, 2);
  lcd.createChar(2, ah);
  lcd.setCursor(15,1);
  lcd.write(2);
  
  lcd.println();
  
    }
  else
  {    
    lcd.println("Failed to get data!");
  }
 
    
  delay(50);
  
}
