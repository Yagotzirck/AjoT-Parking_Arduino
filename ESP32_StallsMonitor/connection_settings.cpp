#include <pgmspace.h>

#include "connection_settings.h"

// Put your WiFi credentials here
const char wifi_ssid[] PROGMEM = "Your WiFi SSID here";
const char wifi_password[] PROGMEM = "Your WiFi password here";


// Put your AWS' MQTT test client endpoint here
const char aws_iot_endpoint[] PROGMEM = "xxxxxxxxxxxxxx-ats.iot.eu-west-3.amazonaws.com";

// Leave this field as it is
const char aws_iot_sensorValues_topic[] PROGMEM =   "AjoT_StallsMonitor/sensorValues";

/* MAC_address:
** MAC string length(17) + '\0' terminating character.
** It will be initialized by the board at startup, so it must be left blank.
*/
char MAC_address[17+1];

/* aws_iot_shadow_root_topic:
** The root topic from which all MQTT operations will be performed on the shadow,
** by appending the desired subtopic to the root (e.g. /update, /get, etc.)
** Since we're using the convention of using the board's MAC address as the thing name
** (and MAC_address is retrieved/initialized at the board's startup), this field will
** be initialized at startup as well and must be left blank.
*/
char aws_iot_shadow_root_topic[sizeof(AWS_IOT_SHADOW_ROOT_TOPIC_PREFIX)-1 + sizeof(AWS_IOT_THING_NAME)-1 + sizeof(AWS_IOT_SHADOW_ROOT_TOPIC_SUFFIX)];


// Europe/Rome
const char posix_timezone[] PROGMEM = "CET-1CEST,M3.5.0,M10.5.0/3";


// Copy contents from AWS CA certificate here
// (CA1 certificate)
const char aws_cert_ca[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)EOF";

// Copy contents from XXXXXXXX-certificate.pem.crt here
const char aws_cert_client[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)KEY";

// Copy contents from XXXXXXXX-private.pem.key here ▼
const char aws_cert_private_key[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
-----END RSA PRIVATE KEY-----
)KEY";