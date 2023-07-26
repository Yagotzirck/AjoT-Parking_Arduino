#ifndef LCDCONTROLLER_H
#define LCDCONTROLLER_H

#include <LiquidCrystal_I2C.h>
#include <string.h>

#include "shared_types.h"

class LcdController{
private:

  static const int numColumns   {16};
  static const int numRows      {2};
  static const int I2C_address  {0x27};

  static const char numFreeStallsMsg[numRows][numColumns];
  static const char enterOpenMsg[numRows][numColumns];
  static const char enterFullMsg[numRows][numColumns];
  static const char enterClosedMsg[numRows][numColumns];
  static const char exitMsg[numRows][numColumns];


  LiquidCrystal_I2C lcd{I2C_address, numColumns, numRows};

  static const unsigned long msgDisplayDurationMs{2000};

  unsigned long lastUpdateTime;


public:

  // Constructor
  LcdController(){}

  /* Call init() from setup(); init()'s contents were originally in the constructor which
  ** was called in a global variable initialization, but apparently this causes
  ** problems:
  ** https://arduino.stackexchange.com/questions/86544/arduino-liquidcrystal-library-code-prevents-esp32-from-running-for-16x2-charact
  */
  void init(){
    lcd.init();
    lcd.backlight();
  }

  void showStatus(int numFreeStalls,
                  int assignedFreeStallId,
                  EntranceRequest entranceRequest,
                  ParkingLotStatus parkingLotStatus)
  {
    unsigned long currTime = millis();

    /* We keep messages for msgDisplayDurationMs milliseconds only if
    ** the new message displays the number of free stalls.
    ** Any other message that responds to user interaction is
    ** displayed immediately and overwrites the previous one, regardless
    ** of the amount of time the previous message has been displayed
    */
    if(entranceRequest == EntranceRequest::NoRequest &&
       currTime < lastUpdateTime + msgDisplayDurationMs)
      return;
    
    lastUpdateTime = currTime;

    switch(entranceRequest){
      case EntranceRequest::NoRequest:
      {
        char msgBuf[numRows][numColumns];
        strcpy(msgBuf[0], numFreeStallsMsg[0]);
        sprintf(msgBuf[1], numFreeStallsMsg[1], numFreeStalls);

        lcd_printMsg(msgBuf);
      }
        break;
      
      case EntranceRequest::Enter:
        switch(parkingLotStatus){
          case ParkingLotStatus::Open:
          {
            char msgBuf[numRows][numColumns];
            strcpy(msgBuf[0], enterOpenMsg[0]);
            sprintf(msgBuf[1], enterOpenMsg[1], assignedFreeStallId);

            lcd_printMsg(msgBuf);
          }
            return;

          case ParkingLotStatus::Full:
            lcd_printMsg(enterFullMsg);
            return;
          
          case ParkingLotStatus::Closed:
            lcd_printMsg(enterClosedMsg);
            return;
        }
      
      case EntranceRequest::Exit:
        lcd_printMsg(exitMsg);
        break;
    }
  }

private:

  void lcd_printMsg(const char msg[numRows][numColumns]){
    lcd.clear();
    for(int currRow{}; currRow < numRows; ++currRow){
      lcd.setCursor(0, currRow);
      lcd.print(msg[currRow]);
    }
  }

};

#endif