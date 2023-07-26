#ifndef ENTRANCEREQUESTSCONTROLLER_H
#define ENTRANCEREQUESTSCONTROLLER_H

#include "shared_types.h"   // For EntranceRequest definition

class EntranceRequestsController {
private:
  using requestInputPin_t = uint8_t;

  static const requestInputPin_t enterButtonPin  {18};
  static const requestInputPin_t exitButtonPin    {5};

  static const int buttonPressed{LOW};  // Since we're using INPUT_PULLUP

  static const unsigned long debouncingDelayMs{200};

  unsigned long lastUpdateTime;


public:

  // Constructor
  EntranceRequestsController(): lastUpdateTime{} {
    pinMode(enterButtonPin, INPUT_PULLUP);
    pinMode(exitButtonPin, INPUT_PULLUP);
  }

  EntranceRequest checkRequests(){
    unsigned long currTime = millis();

    if(currTime < lastUpdateTime + debouncingDelayMs)
      return EntranceRequest::NoRequest;
    
    lastUpdateTime = currTime;

    if( digitalRead(enterButtonPin) == buttonPressed )
      return EntranceRequest::Enter;
    
    if( digitalRead(exitButtonPin) == buttonPressed )
      return EntranceRequest::Exit;
    
    return EntranceRequest::NoRequest;
  }
  
};

#endif