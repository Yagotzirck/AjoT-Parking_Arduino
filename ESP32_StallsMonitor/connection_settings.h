#ifndef CONNECTION_SETTINGS_H
#define CONNECTION_SETTINGS_H

#include <pgmspace.h>

#define AWS_IOT_SHADOW_ROOT_TOPIC_PREFIX "$aws/things/"
#define AWS_IOT_THING_NAME MAC_address
#define AWS_IOT_SHADOW_ROOT_TOPIC_SUFFIX "/shadow/"


extern const char wifi_ssid[] PROGMEM;
extern const char wifi_password[] PROGMEM;

extern const char aws_iot_endpoint[] PROGMEM;

extern const char aws_iot_sensorValues_topic[] PROGMEM;

extern char MAC_address[];
extern char aws_iot_shadow_root_topic[];

extern const char posix_timezone[] PROGMEM;

extern const char aws_cert_ca[] PROGMEM;
extern const char aws_cert_client[] PROGMEM;
extern const char aws_cert_private_key[] PROGMEM;

#endif