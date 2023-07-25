#ifndef PARKINGLOT_H
#define PARKINGLOT_H

#include <iterator> // For std::forward_iterator_tag

#include "ParkingStall.h"
#include "connection_utils.h"
#include "shadow_utils.h"
#include "circuit_settings.h"   // For ParkingLotStatus definition

class ParkingLot{
  private:

  /**************************************************************************
  *********************** Class Variables (static) **************************
  **************************************************************************/

  /* The ESP32 has 6 ADC ports, therefore the maximum amount of stalls we can
  ** control is 6
  ** (Technically the ports are 15, but we can't use analogRead() on the nine
  ** ADC2 ports when WiFi is enabled.)
  */
  static const stallId_t maxStalls = 6;

  // Parking stalls' update frequency in milliseconds
  static const unsigned long updateFrequencyMs{200};


  /****************************************************************************
  ****************************** Member Variables *****************************
  ****************************************************************************/

  /* Array of parking stalls; we're defining it this way because if we define it as
  ** ParkingStall stalls[maxStalls];
  ** we're going to call the default constructor for all of the maxStalls instances,
  ** which is something we don't want to happen since it would be a waste of
  ** computational resources.
  */
  uint8_t stallsMem[maxStalls * sizeof(ParkingStall)];
  ParkingStall *stalls = (ParkingStall*)stallsMem;

  unsigned long lastUpdateTime;

  stallId_t numStalls;
  stallId_t numFreeStalls;

  bool openStatus;


   /****************************************************************************
  ****************************** Member functions *****************************
  ****************************************************************************/

  public:

  // Constructor
  ParkingLot(): numStalls{}, numFreeStalls{}, lastUpdateTime{}, openStatus{true} {}


  void updateStalls(){
    unsigned long currTime = millis();

    if(currTime < lastUpdateTime + updateFrequencyMs)
      return;

    bool stallStatusChanged{false};
    
    lastUpdateTime = currTime;

    numFreeStalls = 0;


    for(stallId_t i{}; i < numStalls; ++i){

      if(stalls[i].checkStall()){
        
        stallStatusChanged = true;

        Serial.printf(
          "Status for stall #%u has changed (new status: %s)\n",
          stalls[i].getStallId(),
          stalls[i].isStallFree()? "free" : "occupied"
        );
      }

      numFreeStalls += static_cast<stallId_t>(stalls[i].isStallFree());
    }

    if(stallStatusChanged)
      sendDeviceStateToShadow(SEND_STALLS_FREE | SEND_STALLS_IDS);


  }

  bool addStall(pinStallAssoc_t pinStallAssoc){
    if(numStalls == maxStalls)
      return false;
    
    stalls[numStalls++] = ParkingStall(pinStallAssoc);

    return true;
  }


  /* Iterator for the stalls array, in order to allow
  ** reading the parking stalls with a range-based for loop
  */
  struct Iterator{
    using iterator_category = std::forward_iterator_tag;
    using value_type        = ParkingStall;
    using pointer           = const value_type*;
    using reference         = const value_type&;

    Iterator(pointer ptr) : m_ptr(ptr) {}

    reference operator*() const { return *m_ptr; }

    // Prefix increment
    Iterator& operator++() { m_ptr++; return *this; }  

    // Postfix increment
    Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }

    friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_ptr == b.m_ptr; };
    friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_ptr != b.m_ptr; };

private:
    pointer m_ptr;
  };

public:
  Iterator begin() { return Iterator(&stalls[0]); }
  Iterator end()   { return Iterator(&stalls[numStalls]); }


  // Getters
  static stallId_t getMaxStalls()     { return maxStalls; }
  stallId_t getNumStalls() const      { return numStalls; }
  stallId_t getNumFreeStalls() const  { return numFreeStalls; }
  bool isOpen() const                 { return openStatus; }

  ParkingLotStatus getParkingLotStatus(){
    if(openStatus){
      if(getNumFreeStalls())
        return ParkingLotStatus::Open;
      else
        return ParkingLotStatus::Full;
    }
    
    return ParkingLotStatus::Closed;
  }

  // Setters
  void setOpenStatus(bool status) { openStatus = status; }

  void resetStalls() {
    numStalls = 0;
    lastUpdateTime = 0;
  }

};

#endif