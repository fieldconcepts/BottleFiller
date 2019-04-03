//-------------------------------------------------------------------------------------
// AutoBottleFiller
// Automaitic bottle filler with loadcell and manula pump override
// Ben Sherwell
// Tested with : Arduino Uno, LCD Display, HX711 Amp and Loadcell
// Date : 03/04/2019
//-------------------------------------------------------------------------------------

// This sketch allows user to place bottle on scales, detects the bottle and begins  
// pumping by turning on a motor and opening a solenoid valve. The loadcell is polled
// whilst pump is on until tagret mass is reached and pump/solenoid closes. A manual 
// override option is available for purging lines from air prior to auto pumping.

// Requires calibration.ino to calibatre load cell prior to use


#include <LiquidCrystal.h>
#include <HX711_ADC.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7); //initialise LCD
HX711_ADC LoadCell(12, 11);  //DT=12, SCK=11

int readKey; //for analogue buttons
const int pumpPin =  13; //control pin for LED/motor
long currentTime; //for counting
const int bottleSize =  500;

int pumpState = 0; //pump on or of state
int bottleState = 0; //bottle present state
int manualState = 0; //manual override state


void setup() {

  float calValue; 
  calValue = -702.94; //adjust calibration factor. get this number by uploading and running file "calibration.ino"
  
  Serial.begin(9600); //Start Serial comms
  delay(100);
  Serial.println("Starting...");

  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ShrubMaster 3000");
  delay(1000);
  lcd.setCursor(0, 0);
  lcd.print("Zero Scales...  ");
  lcd.setCursor(0, 1);
  lcd.print("Bottle Size:    ");
  lcd.setCursor(13, 1);
  lcd.print(bottleSize);
  
  LoadCell.begin(); //Start Loadcell comms
  long stabilisingtime = 2000; //settle time un startup to zero scales.
  LoadCell.start(stabilisingtime);
  LoadCell.setCalFactor(calValue); // set calibration value (float)
  Serial.println("Startup + tare is complete");
  
  pinMode(pumpPin, OUTPUT); //LED/pump pin allocation
  digitalWrite(pumpPin, LOW); //start pump off

  lcd.clear();

}

void loop() {
  LoadCell.update();

  //sample loadcell every 250ms
  if (millis() > currentTime + 250) {
    float raw = LoadCell.getData();
    int mass = (int) raw;

    //Serial for statuses and debugging
    Serial.print("Mass: ");
    Serial.print(mass);
    Serial.print(", PumpState: ");
    Serial.print(pumpState);
    Serial.print(", bottleState: ");
    Serial.print(bottleState);
    Serial.print(", manualState: ");
    Serial.print(manualState);
    Serial.print(", '");

    //LCD mass output re-freshed every 250ms
    lcd.setCursor(0, 1);
    lcd.print("        ");
    lcd.setCursor(0, 1);
    lcd.print(mass);
    
    currentTime = millis();


    // AUTO FILL MODE
    if (manualState == 0){

      //Place bottle if pad empty
      if (mass < 160) {
      bottleState = 0;
      Serial.print("Place Bottle");
      Serial.println("'");
      lcd.setCursor(0, 0);
      lcd.print("Place Bottle    ");
      }

      //Detect bottle on pad
      if (mass > 160 && mass < bottleSize && pumpState == 0) {
        bottleState = 1;
        Serial.print("Bottle Detected");
        Serial.println("'");
        lcd.setCursor(0, 0);
        lcd.print("Bottle Detected ");
      }

      //Start filling in bottle deteced
      if (mass > 160 && pumpState == 1) {
        bottleState = 1;
        Serial.print("Auto Fill...");
        Serial.println("'");
        lcd.setCursor(0, 0);
        lcd.print("Auto Fill...    ");
      }

      // Detectbottle fill and turn off pump
      if (mass > bottleSize){
        pumpState = 0;
        digitalWrite(pumpPin, pumpState);
        Serial.print("Bottle Full");
        Serial.println("'");
        lcd.setCursor(0, 0);
        lcd.print("Bottle Full     ");
      }
    }
    
    // MANUAL FILL MODE - turn off pump ignoring mass
    if (manualState == 1){
      Serial.print("Manual Fill...");
      Serial.println("'");
      lcd.setCursor(0, 0);
      lcd.print("Manual Fill...  ");
    }
   
  }

  //sample analogue pin for buttons
  readKey = analogRead(0);
  if (readKey < 790) {
    delay(100);
    readKey = analogRead(0);
  }
  int result = 0;
  if (readKey < 50) {
    result = 1; // right
  } else if (readKey < 148) {
    result = 2; // up
  } else if (readKey < 305) {
    result = 3; // down
  } else if (readKey < 459) {
    result = 4; // left
  } else if (readKey < 689) {
    result = 5; // select
  }

  
  switch (result) {
     
    case 1: //RIGHT
      Serial.println("RIGHT");
      break;
      
    case 2: //UP - Turns ON auto pump
      if (pumpState == 0 && bottleState == 1){
        pumpState = 1;    
        Serial.println("UP - auto ON"); 
        digitalWrite(pumpPin, pumpState);
      }
      break;
    
    case 3: //DOWN - Turns ON manual pump
      if (pumpState == 0) {
        pumpState = 1;
        manualState = 1;
        Serial.println("DOWN - manual ON");
        digitalWrite(pumpPin, pumpState);
        
      }
      break;
    
    case 4: //LEFT 
      Serial.println("LEFT");
      break;
    
    case 5: //SELECT - Turns OFF both auto and manual pump
      if (pumpState == 1){
        pumpState = 0;
        manualState = 0;    
        Serial.println("SELECT - pump OFF");
        digitalWrite(pumpPin, pumpState);
        lcd.setCursor(0, 0);
        lcd.print("Pump Off        ");
        delay(1000);
      }
      break;
  }
}
