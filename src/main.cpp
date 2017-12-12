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


OneWire oneWire(5);
DallasTemperature sensors(&oneWire);


// LCD /////////////////////////////////////////
Adafruit_LiquidCrystal lcd(0);

// Encoder /////////////////////////////////////
#define encA 2
#define encB 3
//this encoder has a button here
#define encBtn 4

encoderIn<encA,encB> encoder;//simple quad encoder driver
#define ENC_SENSIVITY 4
encoderInStream<encA,encB> encStream(encoder,ENC_SENSIVITY);// simple quad encoder fake Stream

//a keyboard with only one key as the encoder button
keyMap encBtn_map[]={{-encBtn,options->getCmdChar(enterCmd)}};//negative pin numbers use internal pull-up, this is on when low
keyIn<1> encButton(encBtn_map);//1 is the number of keys

//input from the encoder + encoder button + serial
menuIn* inputsList[]={&encStream,&encButton};
chainStream<2> in(inputsList);//3 is the number of inputs

int tempSet = 30;

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

float tempRead()
{
    sensors.requestTemperatures();
    return sensors.getTempCByIndex(0);
}

void displayHome() {
    lcd.setCursor(0, 0);
    lcd.print("T: ");
    lcd.print(tempRead());
    lcd.print(" of ");
    lcd.print(tempSet);
}

void setup() {
  pinMode(encBtn,INPUT_PULLUP);
  encoder.begin();
  lcd.begin(16,2);
  nav.showTitle=false;
  nav.idleOn();
}

void loop() {
    nav.poll();

    if (nav.sleepTask) {
        displayHome();
    }
}
