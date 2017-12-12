#include <Arduino.h>
#include <Wire.h>
#include <menu.h>
#include <liquidCrystalOut.h>
#include <menuIO/serialOut.h>
#include <menuIO/serialIn.h>
#include <menuIO/encoderIn.h>
#include <menuIO/keyIn.h>
#include <menuIO/chainStream.h>
#include <DallasTemperature.h>
#include <Servo.h>

#define TEMPERATURE_PIN 5
#define LCD_ADDRESS 0
#define SERVO_VALVE_PIN 10

// Encoder /////////////////////////////////////
#define encA 2
#define encB 3
#define encBtn 4
#define ENC_SENSIVITY 4

OneWire oneWire(TEMPERATURE_PIN);
DallasTemperature sensors(&oneWire);
Servo valve;

// LCD /////////////////////////////////////////
Adafruit_LiquidCrystal lcd(LCD_ADDRESS);

// Encoder /////////////////////////////////////
encoderIn<encA,encB> encoder;//simple quad encoder driver
encoderInStream<encA,encB> encStream(encoder,ENC_SENSIVITY);// simple quad encoder fake Stream

//a keyboard with only one key as the encoder button
keyMap encBtn_map[]={{-encBtn,options->getCmdChar(enterCmd)}};//negative pin numbers use internal pull-up, this is on when low
keyIn<1> encButton(encBtn_map);//1 is the number of keys

//input from the encoder + encoder button
menuIn* inputsList[]={&encStream,&encButton};
chainStream<2> in(inputsList);//3 is the number of inputs

// Global variables

int tempSet = 30;
float currentTemp;
bool valveOpened = true;

// MENU ///////////////////////////////////////

MENU(mainMenu,"Main menu",doNothing,noEvent,wrapStyle
    ,FIELD(tempSet,"Set Temp","C",30,60,5,1,doNothing,noEvent,wrapStyle)
    ,EXIT("Exit")
);

#define MAX_DEPTH 1

MENU_OUTPUTS(out, MAX_DEPTH
    ,LIQUIDCRYSTAL_OUT(lcd,{0,0,16,2}),
    NONE
);

NAVROOT(nav,mainMenu,1,in,out);//the navigation root object

// END MENU /////////////////////////////////////

void tempRead() {
    sensors.requestTemperatures();
    currentTemp = sensors.getTempCByIndex(0);
}

void openValve() {
    if (valveOpened == false) {
        valve.attach(SERVO_VALVE_PIN);
        delay(100);

        Serial.println("Open");
        valve.write(90);
        valveOpened = true;

        delay(1500);
        valve.detach();
    }
}

void closeValve() {
    if (valveOpened == true) {
        valve.attach(SERVO_VALVE_PIN);
        delay(100);

        Serial.println("Close");
        valve.write(0);
        valveOpened = false;

        delay(1500);
        valve.detach();
    }
}

void displayHome() {
    lcd.setCursor(0, 0);
    lcd.print("T: ");
    lcd.print(currentTemp);
    lcd.print(" of ");
    lcd.print(tempSet);
}

void handleTempControl() {
    if (tempSet - currentTemp < -1) {
        openValve();
    }
    else if (tempSet - currentTemp > 1) {
        closeValve();
    }
}

void setup() {
    Serial.begin(9600);
    pinMode(encBtn,INPUT_PULLUP);
    encoder.begin();
    lcd.begin(16,2);
    nav.showTitle=false;
    nav.idleOn();
    closeValve();
}

void loop() {
    nav.poll();
    tempRead();
    handleTempControl();

    if (nav.sleepTask) {
        displayHome();
    }
}
