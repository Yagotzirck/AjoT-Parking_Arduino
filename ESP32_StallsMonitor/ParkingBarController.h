#ifndef PARKINGBARCONTROLLER_H
#define PARKINGBARCONTROLLER_H

#include <ESP32Servo.h>

#include "shared_types.h"

class ParkingBarController{
private:
  using servoInputPin_t = uint8_t;

  static const servoInputPin_t servoControlPin{23};

  static const int barLowered{90};
  static const int barRaised{0};

  /* Amount of time the bar is left raised (from the moment it starts raising)
  ** before it closes again, in milliseconds
  */
  static const unsigned long barRaisedTimeMs{3000};

  /* We have no way to check whether the servo motor has actually stopped moving,
  ** therefore we introduce an extra delay to make sure the motor has actually
  ** reached its lowered target position before accepting other moving commands
  */
  static const unsigned long barLoweredDelay{1000};

  bool isBarMoving;
  unsigned long lastRaisedBarTime;

  Servo servo;

public:

  // Constructor
  ParkingBarController(): isBarMoving{false}, lastRaisedBarTime{}{
    servo.attach(servoControlPin);
    servo.write(barLowered);
  }

  /* Parking bar control loop.
  ** Returns true if the bar is moving / in a raised state, false otherwise.
  */
  bool loop(EntranceRequest entranceRequest, ParkingLotStatus parkingLotStatus){
    unsigned long currTime = millis();
    
    if(isBarMoving){
      if(currTime < lastRaisedBarTime + barRaisedTimeMs)
        return true;
      else{
        isBarMoving = false;
        servo.write(barLowered);
        return true;
      }
    }
    

    if(currTime < lastRaisedBarTime + barRaisedTimeMs + barLoweredDelay)
      return true;

    switch(entranceRequest){
      case EntranceRequest::NoRequest:
        return false;
      
      case EntranceRequest::Enter:
        if(parkingLotStatus != ParkingLotStatus::Open)
          return false;
        
      case EntranceRequest::Exit:
        lastRaisedBarTime = currTime;
        isBarMoving = true;
        servo.write(barRaised);
        return true;
    }
  }

};

#endif