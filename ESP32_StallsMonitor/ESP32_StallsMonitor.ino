#include "ParkingLot.h"
#include "StatusLedsController.h"

#include "connection_utils.h"
#include "shadow_utils.h"
#include "circuit_settings.h"

ParkingLot parkingLot{};
StatusLedsController statusLedsController{PIN_LED_RED, PIN_LED_GREEN};
 
void setup()
{
  Serial.begin(115200);

  init_MACaddress_AWSshadow();

  connectWifi();
  connectNTP();
  connectMQTT();
  getShadowState();
}
 
void loop()
{
  check_connection();

  parkingLot.updateStalls();
  statusLedsController.showParkingLotStatus(parkingLot.getParkingLotStatus());

  client_loop();
}