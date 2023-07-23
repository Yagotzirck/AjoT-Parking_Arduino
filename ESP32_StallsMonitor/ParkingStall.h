#ifndef PARKINGSTALL_H
#define PARKINGSTALL_H

#include "connection_utils.h"

using pinId_t =     uint8_t;
using stallId_t =   uint8_t;

/* Structure used to create an association between the parking stall number/ID
** and the board's pin number/ID where the photoresistance controlling that
** stall is attached.
*/
struct pinStallAssoc_t{
  pinId_t   pinId;
  stallId_t stallId;
};


class ParkingStall {
  private:

  /**************************************************************************
  *********************** Class Variables (static) **************************
  **************************************************************************/

  /* Brightness threshold shared by all objects, used to define whether a stall is occupied
  ** (when the brightness value reported by the photoresistance is <= the threshold)
  ** or free (when the brightness value is > the threshold)
  */
  static uint16_t brightnessThreshold;

  // The number of readings to perform before computing the average value
  static const uint8_t avgNumReads{10};



  /****************************************************************************
  ****************************** Member Variables *****************************
  ****************************************************************************/

  pinId_t     pinId;
  stallId_t   stallId;

  bool        freeStall;   // true if the parking stall is free, false otherwise

  uint8_t     numReads;

  uint32_t    sumBrightnessValues;
  

  /****************************************************************************
  ****************************** Member functions *****************************
  ****************************************************************************/

  public:

  // Constructor
  ParkingStall(pinStallAssoc_t pinStallAssoc): 
  pinId{pinStallAssoc.pinId},
  stallId{pinStallAssoc.stallId},
  freeStall{true},
  numReads{},
  sumBrightnessValues{}
  {
    pinMode(pinId, INPUT);
  }


  /* checkStall()
  ** Checks the parking stall status by computing the average of numReads photoresistance readings,
  ** marking the stall as
  **    - free, if the average value is <= brightnessThreshold;
  **    - occupied, if the average value is > brightnessThreshold.
  **
  ** Returns:
  **    - true, if the parking stall's state has changed (from free to occupied or vice-versa);
  **    - false otherwise.
  */
  bool checkStall(){
    sumBrightnessValues += analogRead(pinId);
    ++numReads;

    if(numReads == avgNumReads){
      numReads = 0;
      bool prevStallStatus = freeStall;

      unsigned brightnessAvg{sumBrightnessValues / avgNumReads};
      sumBrightnessValues = 0;

      freeStall = brightnessAvg > brightnessThreshold;
      
      
      Serial.printf("Stall: %u, brightnessAvg: %u\n", stallId, brightnessAvg);

      publishStallSensorValue(*this, brightnessAvg);

      return freeStall != prevStallStatus;
    }

    return false;
  }


  // Getters
  static uint16_t getBrightnessThreshold() { return brightnessThreshold; }

  pinId_t   getPinId() const { return pinId; }
  stallId_t getStallId() const { return stallId; }
  bool      isStallFree() const { return freeStall; }

  // Setters
  static void setBrightnessThreshold(uint16_t newBrightnessThreshold) { brightnessThreshold = newBrightnessThreshold; }
  
};

#endif