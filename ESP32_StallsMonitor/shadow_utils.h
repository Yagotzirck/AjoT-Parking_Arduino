#ifndef SHADOW_UTILS_H
#define SHADOW_UTILS_H

#include <PubSubClient.h>

// Bitwise macros used for sendDeviceStateToShadow()'s argument fieldsToSend
#define SEND_NOTHING                 0
#define SEND_MAXSTALLS               1
#define SEND_NUMSTALLS               2
#define SEND_PARKINGLOT_OPEN         4
#define SEND_BRIGHTNESS_THRESHOLD    8
#define SEND_STALLS_IDS             16
#define SEND_STALLS_PIN_IDS         32
#define SEND_STALLS_FREE            64
#define SEND_ALL                   127

void getShadowState();
void sendDeviceStateToShadow(unsigned fieldsToSend);
void messageHandler(char* topic, byte* payload, unsigned int length);
void subscribeToShadowTopics(PubSubClient &client);

#endif