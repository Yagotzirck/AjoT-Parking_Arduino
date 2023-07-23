#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <string.h>
#include <string>

#include "connection_utils.h"
#include "shadow_utils.h"
#include "connection_settings.h"
#include "ParkingStall.h"


static WiFiClientSecure net;
static PubSubClient client{net};


void init_MACaddress_AWSshadow(){
  Serial.print("Initializing fields MAC_address and AWS shadow root topic...");
  strcpy(MAC_address, WiFi.macAddress().c_str());

  strcpy(aws_iot_shadow_root_topic, AWS_IOT_SHADOW_ROOT_TOPIC_PREFIX);
  strcat(aws_iot_shadow_root_topic, AWS_IOT_THING_NAME);
  strcat(aws_iot_shadow_root_topic, AWS_IOT_SHADOW_ROOT_TOPIC_SUFFIX);

  Serial.printf(
    "done\n"
      "\tMAC_address: %s\n"
      "\taws_iot_shadow_root_topic: %s\n",
    MAC_address, aws_iot_shadow_root_topic);
}


/* Publishes on the MQTT topic specified by aws_iot_sensorValues_topic (defined in connection_settings.cpp)
** the parking stall's ID and its associated average brightness value, calculated over
** the last 10 photoresistor readings.
*/
void publishStallSensorValue(const ParkingStall& stall, unsigned brightnessAvg){
  static const size_t jsonSize_bytes{768};

  StaticJsonDocument<jsonSize_bytes> doc;

  /* Get time with precision down to the microsecond, so that we can use it as a primary key
  ** (unlike time_t, which has precision down to the second and therefore it's very likely that
  ** different stall readings will share the same "time" key value).
  */
  uint64_t now_microseconds;
  {
    struct timeval tv;
	  gettimeofday(&tv, NULL);
	 now_microseconds = {tv.tv_sec * 1000000ULL + tv.tv_usec};
  }
  
  doc["time_microseconds"] =  now_microseconds;
  doc["MAC_address"] =        MAC_address;
  doc["stall_id"] =           stall.getStallId();
  doc["brightness_value"] =   brightnessAvg;


  String jsonStringBuf;
  jsonStringBuf.reserve(measureJson(doc) + 1);
  
  // Convert JSON object to char[] string
  if(serializeJson(doc, jsonStringBuf) == DeserializationError::NoMemory){
    Serial.println("Error in publishStallSensorValue(): serializeJson() returned NoMemory");
    return;
  }

  client.publish(aws_iot_sensorValues_topic, jsonStringBuf.c_str());
}

void publishToTopic(const char *topic, const char *payload){ client.publish(topic, payload); }

void client_loop(){ client.loop(); }

 
void connectWifi(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);
 
  Serial.print("Connecting to Wi-Fi...");
 
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("done");
}
 
void connectMQTT()
{
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(aws_cert_ca);
  net.setCertificate(aws_cert_client);
  net.setPrivateKey(aws_cert_private_key);

  client.setBufferSize(8192);
  Serial.printf("New buffer size: %u\n", client.getBufferSize());
 
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(aws_iot_endpoint, 8883);
 
  // Create a message handler
  client.setCallback(messageHandler);
 
  Serial.print("Connecting to AWS IOT...");
 
  
  do{
    Serial.print(".");
    delay(500);
  } while (!client.connect(AWS_IOT_THING_NAME));
 
  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }
 
  Serial.println("AWS IoT Connected!");

  // Subscribe to the AWS shadow's related topics
  subscribeToShadowTopics(client);
}

// Get time through Simple Network Time Protocol
void connectNTP(void)
{
  time_t now{};
  // nowish = November 13, 2017 (a random past date); when now becomes > nowish, we know the time has been set
  const time_t nowish = 1510592825;

  Serial.print("Setting time using SNTP...");
  configTime(0, 0, "pool.ntp.org");    // First connect to NTP server, with 0 TZ offset
  now = time(nullptr);
  while (now < nowish){
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }

  // Set the timezone to the one defined in posix_timezone (connection_settings.cpp)
  setenv("TZ", posix_timezone,1);
  tzset();
  
  Serial.println("done!");
  now = time(nullptr);
  struct tm *timeinfo = localtime(&now);
  Serial.print("Current time: ");
  Serial.print(asctime(timeinfo));
}

void check_connection(){
  if(client.connected())
    return;   // Client is connected - nothing to do here

  Serial.println("Client is disconnected - trying to re-establish a connection");
  connectWifi();
  connectMQTT();
  getShadowState();
}
