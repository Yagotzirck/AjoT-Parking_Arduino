#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "shadow_utils.h"
#include "connection_settings.h"
#include "connection_utils.h"
#include "ParkingLot.h"
#include "ParkingStall.h"

extern ParkingLot parkingLot;


/* List of topics to subscribe to in order to interact with
** the AWS Device Shadow.
** For more info, see:
** https://docs.aws.amazon.com/iot/latest/developerguide/device-shadow-comms-device.html#device-shadow-comms-device-first-connect
*/
static const char *shadowSubscribeTopics[] = {
  "delete/accepted",
  "delete/rejected",

  "get/accepted",
  "get/rejected",

  //"update/documents",
  "update/accepted",
  "update/rejected",
  "update/delta"
};

enum class EnumShadowSubscribeTopics {
  Delete_accepted,
  Delete_rejected,

  Get_accepted,
  Get_rejected,

  //Update_documents,
  Update_accepted,
  Update_rejected,
  Update_delta,
	
  Unknown_topic
};


/********************************************************************
***************** Local functions' declarations *********************
********************************************************************/
static EnumShadowSubscribeTopics getIncomingMsgTopic(const char *topic);
static void getDeviceStateFromShadowState(byte* payload, unsigned int length, bool getIfTrue_updateIfFalse);
static int showRejectedInfo(byte* payload, unsigned int length);

/********************************************************************
***************** Global functions' definitions *********************
********************************************************************/


/* Retrieves the shadow's state by publishing on shadow's topic "get/" with
** an empty payload.
** AWS will send the shadow's state in the topic "get/accepted";
** the received message/payload will be processed by the callback function messageHandler().
*/
void getShadowState(){
  char topicBuf[128];

  strcpy(topicBuf, aws_iot_shadow_root_topic);
  strcat(topicBuf, "get");

  Serial.print("\nRetrieving AWS device shadow's state by publishing an empty payload in topic:\n\t");
  Serial.print(topicBuf);
  Serial.println("\n\t(The payload in the response will be handled by PubSubClient's callback function messageHandler() )\n");

  publishToTopic(topicBuf, "");
}

void messageHandler(char* topic, byte* payload, unsigned int length)
{
  Serial.printf("messageHandler(): incoming message in topic:\n\t%s\n", topic);

  switch(getIncomingMsgTopic(topic)){
    case EnumShadowSubscribeTopics::Delete_accepted:
    //case EnumShadowSubscribeTopics::Update_documents:
    case EnumShadowSubscribeTopics::Unknown_topic:
      Serial.println("\tTopic is unknown and/or not handled");
      break;
    
    case EnumShadowSubscribeTopics::Delete_rejected:
    case EnumShadowSubscribeTopics::Get_rejected: 
    case EnumShadowSubscribeTopics::Update_rejected:
      {
        int errorCode = showRejectedInfo(payload, length);

          if(errorCode == 404){
          Serial.println("Shadow doesn't exist; sending the device state to create it");
          sendDeviceStateToShadow(SEND_ALL);
          }
      }
      break;

    case EnumShadowSubscribeTopics::Get_accepted:
      getDeviceStateFromShadowState(payload, length, true);
      break;

    case EnumShadowSubscribeTopics::Update_delta:
      getDeviceStateFromShadowState(payload, length, false);
      break;
    

    case EnumShadowSubscribeTopics::Update_accepted:
      Serial.println("\tThe device state update sent to the shadow has been accepted");
      break;
  }

}

void subscribeToShadowTopics(PubSubClient &client){
  char topicBuf[128];

  strcpy(topicBuf, aws_iot_shadow_root_topic);

  // Create a pointer that points at the end of the shadow root topic string
  char *shadowRootTopicEnd{topicBuf + strlen(topicBuf)};

  for(const char *currTopic: shadowSubscribeTopics){
    strcpy(shadowRootTopicEnd, currTopic);
    Serial.printf("Subscribing to topic %s ... ", topicBuf);
    bool subscribeOk = client.subscribe(topicBuf);

    Serial.printf("%s\n", subscribeOk ? "done" : "Couldn't subscribe to this topic");
  }
}


void sendDeviceStateToShadow(unsigned fieldsToSend){
  if(fieldsToSend == SEND_NOTHING)
    return;

  char topicBuf[128];
  strcpy(topicBuf, aws_iot_shadow_root_topic);
  strcat(topicBuf, "update");

  

  // The "* 3" multiplication for JSON_ARRAY_SIZE is due to the fact we have 3 arrays inside "reported"
  DynamicJsonDocument jsonBuffer(JSON_OBJECT_SIZE(16) + JSON_ARRAY_SIZE(ParkingLot::getMaxStalls()) * 3 + 100);
  JsonObject root = jsonBuffer.to<JsonObject>();
  JsonObject state = root.createNestedObject("state");
  JsonObject stateReported = state.createNestedObject("reported");

  JsonArray stalls_ids;
  JsonArray stalls_pinIds;
  JsonArray stalls_free;

  if(fieldsToSend & SEND_STALLS_IDS)
    stalls_ids = stateReported.createNestedArray("stalls_ids");
  if(fieldsToSend & SEND_STALLS_PIN_IDS)
    stalls_pinIds = stateReported.createNestedArray("stalls_pinIds");
  if(fieldsToSend & SEND_STALLS_FREE)
    stalls_free = stateReported.createNestedArray("stalls_free");
  
  if(fieldsToSend & SEND_MAXSTALLS)
    stateReported["maxStalls"] =                        ParkingLot::getMaxStalls();
  if(fieldsToSend & SEND_NUMSTALLS)
    stateReported["numStalls"] =                        parkingLot.getNumStalls();
  if(fieldsToSend & SEND_PARKINGLOT_OPEN)
      stateReported["parkingLot_open"] =                parkingLot.isOpen();
  if(fieldsToSend & SEND_BRIGHTNESS_THRESHOLD)
    stateReported["stall_free_brightness_threshold"] =  ParkingStall::getBrightnessThreshold();

  for(const ParkingStall& currStall: parkingLot){
    if(fieldsToSend & SEND_STALLS_IDS)
      stalls_ids.add(currStall.getStallId());
    if(fieldsToSend & SEND_STALLS_PIN_IDS)
      stalls_pinIds.add(currStall.getPinId());
    if(fieldsToSend & SEND_STALLS_FREE)
      stalls_free.add(currStall.isStallFree());
  }


  String jsonStringBuf;
  jsonStringBuf.reserve(measureJson(root) + 1);


  //char jsonStringBuf[measureJson(jsonBuffer) + 1];
  
  // Convert JSON object to char[] string
  if(serializeJson(jsonBuffer, jsonStringBuf) == DeserializationError::NoMemory){
    Serial.println("Error in sendDeviceStateToShadow(): serializeJson() returned NoMemory");
    return;
  }

  publishToTopic(topicBuf, jsonStringBuf.c_str());
  //publishToTopic(topicBuf, jsonStringBuf);
}



/********************************************************************
****************** Local functions' definitions *********************
********************************************************************/
static EnumShadowSubscribeTopics getIncomingMsgTopic(const char *topic){
  static const int numTopics{sizeof(shadowSubscribeTopics) / sizeof(shadowSubscribeTopics[0])};
  /* Create a pointer that discards the shadow root topic part of the string
  ** and points only to the subtopic string.
  **
  ** NOTE: this assumes that we only subscribed to shadow-related topics
  ** whose strings' first part consists of aws_iot_shadow_root_topic;
  ** if that's not the case and strlen(topic) < strlen(aws_iot_shadow_root_topic),
  ** weird things might happen (e.g. crash / segmentation fault).
  ** In order to avoid that, we first check that topic's length is greater than
  ** the shadow root topic's length before proceeding with the
  ** shadow root topic's comparison. 
  */
  if( strlen(topic) <= strlen(aws_iot_shadow_root_topic) )
    return EnumShadowSubscribeTopics::Unknown_topic;

  const char *shadowSubTopic{topic + strlen(aws_iot_shadow_root_topic)};

  int i{};
  for(; i < numTopics; ++i)
    if(!strcmp(shadowSubTopic, shadowSubscribeTopics[i]))
      break;
  
  return static_cast<EnumShadowSubscribeTopics>(i);
}


static void getDeviceStateFromShadowState(byte* payload, unsigned int length, bool getIfTrue_updateIfFalse){

  // The "* 3" multiplication for JSON_ARRAY_SIZE is due to the fact we might have up to 3 arrays associated to parking stalls
  DynamicJsonDocument jsonRoot(JSON_OBJECT_SIZE(16) + JSON_ARRAY_SIZE(ParkingLot::getMaxStalls()) * 3 + 100);

  deserializeJson(jsonRoot, (char*)payload, length);

  JsonObject jsonState;

  /* If the payload comes from the topic get/accepted, the state variables are nested in
      {
        "state":  {
          "desired": {
              // variables
          }
        }
      }

    If the payload comes from the topic update/delta, the state variables are nested in
      {
        "state": {
          // variables
        }
      }
  */
  jsonState = getIfTrue_updateIfFalse ? jsonRoot["state"]["desired"].as<JsonObject>() : jsonRoot["state"].as<JsonObject>();

  int numStalls{-1};
  unsigned fieldsToSend{SEND_NOTHING};

  Serial.println(
    "\n======================================================\n\n"
    "Retrieving device state from shadow"
    );
  
  if(jsonState.containsKey("numStalls")){
    numStalls = jsonState["numStalls"].as<int>();
    Serial.printf("numStalls: %d\n", numStalls);
    fieldsToSend |= (SEND_NUMSTALLS | SEND_STALLS_IDS | SEND_STALLS_PIN_IDS);
  }

  /* If we're responding to the topic "get/accepted", we must get the stalls' status
  ** (free/occupied) as well, in order to update it if it changed
  */
  if(getIfTrue_updateIfFalse)
    fieldsToSend |= SEND_STALLS_FREE;

      
  if(jsonState.containsKey("parkingLot_open")){
    parkingLot.setOpenStatus(jsonState["parkingLot_open"].as<bool>());
    Serial.printf("Parking lot status: %s\n", parkingLot.isOpen() ? "open" : "closed");
    fieldsToSend |= SEND_PARKINGLOT_OPEN;
  }
  
	 if(jsonState.containsKey("stall_free_brightness_threshold")){
	  ParkingStall::setBrightnessThreshold(jsonState["stall_free_brightness_threshold"].as<uint16_t>());
    Serial.printf("Brightness threshold: %u\n", ParkingStall::getBrightnessThreshold());
    fieldsToSend |= SEND_BRIGHTNESS_THRESHOLD;
   }
  
  /* If numStalls key was present in the JSON state file, we can assume that the json arrays
  ** stalls_ids and stalls_pinIds are present as well
  */
  if(numStalls >= 0){

    JsonArray jsonStallsIds{jsonState["stalls_ids"].as<JsonArray>()};
    JsonArray jsonStallsPinIds{jsonState["stalls_pinIds"].as<JsonArray>()};

    /* We're using range-based for loops here since json arrays are stored as linked lists,
    ** and cycling through them with classic indexing is extremely inefficient (the list is
    ** traversed from the beginning each time the array is referenced with the operator []).
    */
    pinStallAssoc_t pinStallAssocs[numStalls];

    stallId_t i{};

    for(pinId_t currPinId: jsonStallsPinIds)
      pinStallAssocs[i++].pinId = currPinId;
    
    i = 0;

    for(stallId_t currStallId: jsonStallsIds)
      pinStallAssocs[i++].stallId = currStallId;

    parkingLot.resetStalls();
    
    for(stallId_t i{}; i < numStalls; ++i)
      parkingLot.addStall(pinStallAssocs[i]);
    

    Serial.println("Parking stalls:");
    Serial.println("\tstallId\tpinId");

    for(const ParkingStall& currStall: parkingLot)
      Serial.printf("\t%u\t%u\n", currStall.getStallId(), currStall.getPinId());

  }
  
    Serial.println("\n======================================================\n");

  /* Now that the device state matches with the shadow state in the "desired" field,
  ** update the "reported" field in the shadow state with the updated device state
  */
  sendDeviceStateToShadow(fieldsToSend);

}

/* Prints information about the error reported in the "rejected" subtopics,
** and returns the int value associated to the field "code" if present,
** otherwise it simply returns 0.
*/
static int showRejectedInfo(byte* payload, unsigned int length){

  StaticJsonDocument<768> doc;


  if(!deserializeJson(doc, (char*) payload) == DeserializationError::Ok){
    Serial.println("showRejectedInfo(): error in deserializeJson()");
    return 0;
  }

  Serial.printf("Error response document's contents:\n");

  int errorCode{};
  if(doc.containsKey("code")){
    errorCode = doc["code"].as<int>();
    Serial.printf("\tHTTP error code: %d\n", errorCode);
  }
    
  if(doc.containsKey("message"))
    Serial.printf("\tError message: \"%s\"\n", doc["message"].as<const char*>());

  if(doc.containsKey("timestamp"))
    Serial.printf("\tTimestamp: %lu\n", doc["timestamp"].as<unsigned long>());
  
  if(doc.containsKey("clientToken"))
    Serial.printf("\tClient token: %lu\n\n", doc["clientToken"].as<unsigned long>());
  
  return errorCode;
}