#include "ParkingLot.h"
#include "StatusLedsController.h"
#include "EntranceRequestsController.h"
#include "LcdController.h"

#include "connection_utils.h"
#include "shadow_utils.h"

#include "shared_types.h"

ParkingLot parkingLot{};

static StatusLedsController       statusLedsController{};
static EntranceRequestsController entranceRequestsController{};
static LcdController              lcdController{};

bool isParkingBarMoving{false};

 
void setup()
{
  Serial.begin(115200);

  init_MACaddress_AWSshadow();

  connectWifi();
  connectNTP();
  connectMQTT();
  getShadowState();

  lcdController.init();
}
 
void loop()
{
  check_connection();
  parkingLot.updateStalls();
  client_loop();

  ParkingLotStatus parkingLotStatus     {parkingLot.getParkingLotStatus()};
  int              numFreeStalls        {parkingLot.getNumFreeStalls()};
  int              assignedFreeStallId  {parkingLot.getFreeStallId()};
  EntranceRequest  entranceRequest      {entranceRequestsController.checkRequests()};

  statusLedsController.showParkingLotStatus(parkingLotStatus);
  lcdController.showStatus(numFreeStalls, assignedFreeStallId, entranceRequest, parkingLotStatus);
}