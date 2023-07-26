#ifndef STATUSLEDSCONTROLLER_H
#define STATUSLEDSCONTROLLER_H

#include "shared_types.h"   // For ParkingLotStatus definition

class StatusLedsController{
private:
  using statusLedPin_t = uint8_t; 

  static const statusLedPin_t redLedPin{4};
  static const statusLedPin_t greenLedPin{15};

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
  StatusLedsController(){
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