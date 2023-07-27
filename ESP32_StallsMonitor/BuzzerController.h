#ifndef BUZZERCONTROLLER_H
#define BUZZERCONTROLLER_H

// NOTE: This is meant to be used with active buzzers only.
class BuzzerController{
private:
  using buzzerOutputPin_t = uint8_t;
  static const buzzerOutputPin_t buzzerOutputPin{26};

  /* Time interval for the buzzer's on/off switching sequence,
  ** in milliseconds
  */
  static const unsigned long onOffSwitchingMs{500};

  bool onOffSwitch;
  unsigned long currIntervalStart;

public:

  // Constructor
  BuzzerController(): onOffSwitch{false}, currIntervalStart{}{
    pinMode(buzzerOutputPin, OUTPUT);
    digitalWrite(buzzerOutputPin, LOW);
  }


  void loop(bool active){
    unsigned long currTime = millis();
    
    if(!active){
      onOffSwitch = false;
      digitalWrite(buzzerOutputPin, LOW);
      return;
    }
      
    if(currTime < currIntervalStart + onOffSwitchingMs)
      return;

    onOffSwitch = !onOffSwitch;
    currIntervalStart = currTime;
    digitalWrite(buzzerOutputPin, onOffSwitch);
  }

};

#endif