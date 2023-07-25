#ifndef STATUSLEDSCONTROLLER_H
#define STATUSLEDSCONTROLLER_H

#include "circuit_settings.h"   // For ParkingLotStatus definition

using statusLedPin_t = uint8_t; 

class StatusLedsController{
private:
  const statusLedPin_t redLedPin, greenLedPin;

  void turnGreenLedOn(){
    digitalWrite(redLedPin, LOW);
    digitalWrite(greenLedPin, HIGH);
  }

  void turnRedLedOn(){
    digitalWrite(redLedPin, HIGH);
    digitalWrite(greenLedPin, LOW);
  }


public:
  // Constructor
  StatusLedsController(statusLedPin_t redPin, statusLedPin_t greenPin):
  redLedPin{redPin},
  greenLedPin{greenPin}
  {
    pinMode(redLedPin, OUTPUT);
    pinMode(greenLedPin, OUTPUT);

    turnRedLedOn();
  }


  void showParkingLotStatus(ParkingLotStatus parkingLotStatus){
    switch(parkingLotStatus){
      case ParkingLotStatus::Open:
        turnGreenLedOn();
        break;
      
      case ParkingLotStatus::Full:
      case ParkingLotStatus::Closed:
        turnRedLedOn();
        break;
    }
  }
};


#endif