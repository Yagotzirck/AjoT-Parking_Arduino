#ifndef CONNECTION_UTILS_H
#define CONNECTION_UTILS_H

class ParkingStall;   // forward declaration

void init_MACaddress_AWSshadow();

void connectWifi();
void connectMQTT();
void connectNTP();

void publishStallSensorValue(const ParkingStall& stall, unsigned brightnessAvg);

void publishToTopic(const char *topic, const char *payload);

void check_connection();
void client_loop();

#endif