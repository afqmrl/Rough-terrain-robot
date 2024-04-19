// Config.h
#ifndef Config_h
#define Config_h

// Pin definitions
#define EEPROM_SIZE 512
#define LED 2
#define TRIG_PIN 3
#define ECHO_PIN 46
#define SERVO_PIN 45
#define LEDS_COUNT  8
#define LEDS_PIN	48
#define CHANNEL		0
#define CAMERA_MODEL_ESP32S3_EYE
#define EI_CAMERA_RAW_FRAME_BUFFER_COLS           320
#define EI_CAMERA_RAW_FRAME_BUFFER_ROWS           240
#define EI_CAMERA_FRAME_BYTE_SIZE                 3
#define PART_BOUNDARY "123456789000000000000987654321"


// Add other pin definitions here

// WiFi credentials
const char* ssid = "RUMAHKU SYURGAKU 5G";
const char* password = "PC68DJDP";

const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace; boundary=" PART_BOUNDARY;
const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

// const char* _STREAM_BOUNDARY = "--boundary\r\n";
// const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

int pinI1= 1;//4;//define I1 port
int pinI2=47;//5;//define I2 port
int speedpin1=42;//6;//define EA(PWM speed regulation)port
int pinI3=41;//16;//define I1 port
int pinI4=40;//17;//define I2 port
int speedpin2=39;//15;//define EA(PWM speed regulation)port
bool goesRover = false;
bool updateMemory =false;
int addr = 0;
int address_End;
int addr_Start = 0;
int addr_End = 0;
int cmd;
int value =0;
int value2=0;
unsigned long lastCommandTime = 0; // Tracks the time when the last command was executed
String command;
Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(LEDS_COUNT, LEDS_PIN, CHANNEL);
static bool debug_nn = false; // Set this to true to see e.g. features generated from the raw signal
static bool is_initialised = false;
uint8_t *snapshot_buf; //points to the output of the capture
int docking;

s3servo myServo;
// WebServer server(80);




// Camera model
#define CAMERA_MODEL_ESP32S3_EYE
// Add other configurations and declarations here

#endif
