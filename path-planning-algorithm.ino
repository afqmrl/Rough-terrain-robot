#include "String.h"
#include "EEPROM.h"
#include "Freenove_WS2812_Lib_for_ESP32.h" 
#include <autonomous_robot_inferencing.h>
#include "edge-impulse-sdk/dsp/image/image.hpp"
#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "test robot";
const char* password = "123456789";

WebServer server(80);


#define EEPROM_SIZE 512
#define LED 2
#define LEDS_COUNT  8
#define LEDS_PIN	48
#define CHANNEL		0
#define CAMERA_MODEL_ESP32S3_EYE
#define EI_CAMERA_RAW_FRAME_BUFFER_COLS           320
#define EI_CAMERA_RAW_FRAME_BUFFER_ROWS           240
#define EI_CAMERA_FRAME_BYTE_SIZE                 3


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
bool ei_camera_init(void);
void ei_camera_deinit(void);
bool ei_camera_capture(uint32_t img_width, uint32_t img_height, uint8_t *out_buf) ;


#if defined(CAMERA_MODEL_ESP32S3_EYE)
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 15
#define SIOD_GPIO_NUM 4
#define SIOC_GPIO_NUM 5

#define Y2_GPIO_NUM 11
#define Y3_GPIO_NUM 9
#define Y4_GPIO_NUM 8
#define Y5_GPIO_NUM 10
#define Y6_GPIO_NUM 12
#define Y7_GPIO_NUM 18
#define Y8_GPIO_NUM 17
#define Y9_GPIO_NUM 16

#define VSYNC_GPIO_NUM 6
#define HREF_GPIO_NUM 7
#define PCLK_GPIO_NUM 13

#else
#error "Camera model not selected"
#endif

static camera_config_t camera_config = {
    .pin_pwdn = PWDN_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,
    .pin_xclk = XCLK_GPIO_NUM,
    .pin_sscb_sda = SIOD_GPIO_NUM,
    .pin_sscb_scl = SIOC_GPIO_NUM,

    .pin_d7 = Y9_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d0 = Y2_GPIO_NUM,
    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,

    //XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG, //YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size = FRAMESIZE_QVGA,    //QQVGA-UXGA Do not use sizes above QVGA when not JPEG

    .jpeg_quality = 12, //0-63 lower number means higher quality
    .fb_count = 1,       //if more than one, i2s runs in continuous mode. Use only with JPEG
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};


void setup() {
     if (!EEPROM.begin(EEPROM_SIZE)) {
        Serial.println("Failed to initialise EEPROM");
        return;
    }
  
  pinMode(LED, OUTPUT);
  Serial.begin(115200);
  pinMode(pinI1,OUTPUT);//define this port as output
  pinMode(pinI2,OUTPUT);
  pinMode(speedpin1,OUTPUT);
  pinMode(pinI3,OUTPUT);//define this port as output
  pinMode(pinI4,OUTPUT);
  pinMode(speedpin2,OUTPUT);
  strip.begin();
 
  
  // Setup WiFi
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  server.on("/", HTTP_GET, handleRoot);
  server.on("/command", HTTP_GET, handleCommand); // Consolidated command handler
  
  server.begin();

  while (!Serial);
    Serial.println("Edge Impulse Inferencing Demo");
    if (ei_camera_init() == false) {
        ei_printf("Failed to initialize Camera!\r\n");
    }
    else {
        ei_printf("Camera initialized\r\n");
    }

    ei_printf("\nStarting continious inference in 2 seconds...\n");
    ei_sleep(2000);

   

}

void loop() {


  server.handleClient();

 if (ei_sleep(5) != EI_IMPULSE_OK) {
        return;
    }

    snapshot_buf = (uint8_t*)malloc(EI_CAMERA_RAW_FRAME_BUFFER_COLS * EI_CAMERA_RAW_FRAME_BUFFER_ROWS * EI_CAMERA_FRAME_BYTE_SIZE);

    // check if allocation was successful
    if(snapshot_buf == nullptr) {
        ei_printf("ERR: Failed to allocate snapshot buffer!\n");
        return;
    }

    ei::signal_t signal;
    signal.total_length = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
    signal.get_data = &ei_camera_get_data;

    if (ei_camera_capture((size_t)EI_CLASSIFIER_INPUT_WIDTH, (size_t)EI_CLASSIFIER_INPUT_HEIGHT, snapshot_buf) == false) {
        ei_printf("Failed to capture image\r\n");
        free(snapshot_buf);
        return;
    }

    // Run the classifier
    ei_impulse_result_t result = { 0 };

    EI_IMPULSE_ERROR err = run_classifier(&signal, &result, debug_nn);
    if (err != EI_IMPULSE_OK) {
        ei_printf("ERR: Failed to run classifier (%d)\n", err);
        return;
    }

    // print the predictions
    ei_printf("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.): \n",
                result.timing.dsp, result.timing.classification, result.timing.anomaly);

#if EI_CLASSIFIER_OBJECT_DETECTION == 1
    bool bb_found = result.bounding_boxes[0].value > 0;
    for (size_t ix = 0; ix < result.bounding_boxes_count; ix++) {
        auto bb = result.bounding_boxes[ix];
        if (bb.value == 0) {
            continue;
        }
        ei_printf("    %s (%f) [ x: %u, y: %u, width: %u, height: %u ]\n", bb.label, bb.value, bb.x, bb.y, bb.width, bb.height);
        if (bb.value >= 0.8 && bb.x>=40 && bb.x<=60 && bb.y>=40 && bb.y<=60){
          digitalWrite(LED, HIGH);
        }
        else if (bb.value < 0.8 && (bb.x>60 or bb.x<40) && (bb.y>60 or bb.y<40)){

          Serial.println("not in range");
        digitalWrite(LED,LOW);
        }
    }
    if (!bb_found) {
        ei_printf("    No objects found\n");
    }
#else
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        ei_printf("    %s: %.5f\n", result.classification[ix].label,
                                    result.classification[ix].value);
    }
#endif

#if EI_CLASSIFIER_HAS_ANOMALY == 1
        ei_printf("    anomaly score: %.3f\n", result.anomaly);
#endif


    free(snapshot_buf);
}

bool ei_camera_init(void) {

    if (is_initialised) return true;

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

    //initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
      Serial.printf("Camera init failed with error 0x%x\n", err);
      return false;
    }

    sensor_t * s = esp_camera_sensor_get();
    // initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID) {
      s->set_vflip(s, 1); // flip it back
      s->set_brightness(s, 1); // up the brightness just a bit
      s->set_saturation(s, 0); // lower the saturation
    }

#if defined(CAMERA_MODEL_M5STACK_WIDE)
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);
#elif defined(CAMERA_MODEL_ESP_EYE)
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);
    s->set_awb_gain(s, 1);
#endif

    is_initialised = true;
    return true;
}

void ei_camera_deinit(void) {

    //deinitialize the camera
    esp_err_t err = esp_camera_deinit();

    if (err != ESP_OK)
    {
        ei_printf("Camera deinit failed\n");
        return;
    }

    is_initialised = false;
    return;
}

bool ei_camera_capture(uint32_t img_width, uint32_t img_height, uint8_t *out_buf) {
    bool do_resize = false;

    if (!is_initialised) {
        ei_printf("ERR: Camera is not initialized\r\n");
        return false;
    }

    camera_fb_t *fb = esp_camera_fb_get();

    if (!fb) {
        ei_printf("Camera capture failed\n");
        return false;
    }

       bool converted = fmt2rgb888(fb->buf, fb->len, PIXFORMAT_JPEG, snapshot_buf);

   esp_camera_fb_return(fb);

   if(!converted){
       ei_printf("Conversion failed\n");
       return false;
   }

    if ((img_width != EI_CAMERA_RAW_FRAME_BUFFER_COLS)
        || (img_height != EI_CAMERA_RAW_FRAME_BUFFER_ROWS)) {
        do_resize = true;
    }

    if (do_resize) {
        ei::image::processing::crop_and_interpolate_rgb888(
        out_buf,
        EI_CAMERA_RAW_FRAME_BUFFER_COLS,
        EI_CAMERA_RAW_FRAME_BUFFER_ROWS,
        out_buf,
        img_width,
        img_height);
    }


    return true;
}

static int ei_camera_get_data(size_t offset, size_t length, float *out_ptr)
{
    // we already have a RGB888 buffer, so recalculate offset into pixel index
    size_t pixel_ix = offset * 3;
    size_t pixels_left = length;
    size_t out_ptr_ix = 0;

    while (pixels_left != 0) {
        out_ptr[out_ptr_ix] = (snapshot_buf[pixel_ix] << 16) + (snapshot_buf[pixel_ix + 1] << 8) + snapshot_buf[pixel_ix + 2];

        // go to the next pixel
        out_ptr_ix++;
        pixel_ix+=3;
        pixels_left--;
    }
    // and done!
    return 0;
}

#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_CAMERA
#error "Invalid model for current sensor"
#endif

void handleRoot() {
  String html = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
  <title>ESP32 Car Control</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
      flex-direction: column;
    }
    .control-panel {
      display: grid;
      grid-template-columns: repeat(3, 100px);
      grid-gap: 10px;
      justify-content: center;
    }
    button {
      padding: 10px;
      border: none;
      border-radius: 5px;
      background-color: #4CAF50;
      color: white;
      font-size: 16px;
      cursor: pointer;
      transition: background-color 0.3s ease;
    }
    button:hover {
      background-color: #45a049;
    }
    .special-buttons {
      grid-column: span 3;
      justify-content: center;
      display: flex;
      gap: 10px;
    }
  </style>
  </head>
  <body>
  <h1>ESP32 Car Control</h1>
  <p>Use the buttons below or keyboard arrows to control the car.</p>
  <div class="control-panel">
    <button onclick="sendCommand('1')">IN Trace Path 1</button>
    <button onclick="sendCommand('2')">IN Trace Path 2</button>
    <button onclick="sendCommand('3')">IN Trace Pathe 3</button>
    <button onclick="sendCommand('4')">OUT Trace Path 1</button>
    <button onclick="sendCommand('5')">OUT Trace Path 2</button>
    <button onclick="sendCommand('6')">OUT Trace Path 3</button>
    <button onclick="sendCommand('0')">END OPERATION</button>
    <button onclick="sendCommand('forward')">FORWARD</button>
    <button onclick="sendCommand('backward')">BACKWARD</button>
    <button onclick="sendCommand('right')">RIGHT</button>
    <button onclick="sendCommand('left')">LEFT</button>
    <button onclick="sendCommand('stop')" class="special-buttons">Stop</button>
    <button onclick="sendCommand('reset')" class="special-buttons">RESET</button>

   
  </div>
  <script>
    document.body.addEventListener('keydown', function(e) {
      switch(e.key) {
        case 'w': sendCommand('forward'); break;
        case 's': sendCommand('backward'); break;
        case 'a': sendCommand('left'); break;
        case 'd': sendCommand('right'); break;
        case ' ': sendCommand('stop'); break; // Spacebar to stop
        case '1': sendCommand('1'); break;
        case '2': sendCommand('2'); break;
        case '3': sendCommand('3'); break;
        case '4': sendCommand('4'); break;
        case '5': sendCommand('5'); break;
        case '6': sendCommand('6'); break;
        case '0': sendCommand('0'); break;
        case '#': sendCommand('reset'); break;
      }
    });
    function sendCommand(command) {
      fetch('/command?act=' + command)
        .then(response => response.text())
        .then(data => console.log(data))
        .catch(error => console.error('Error:', error));
    }
  </script>
  </body>
  </html>
  )rawliteral";
  server.send(200, "text/html", html);
}


void handleCommand() {
  if (server.hasArg("act")) {
    String command = server.arg("act");
    controlMotors(command);
    command=" ";
    server.send(200, "text/plain", "Command received: " + command);
  } else {
    server.send(400, "text/plain", "Bad Request: Missing 'act' parameter");
  }
}

void controlMotors( String command) {
  if (command == "forward") {
    
    forward();
    if(updateMemory)
    {
      cmd=1;
      
      memory_IN();
      
      
    }

  }
   else if (command == "backward") {
    
    backward();
     if(updateMemory)
    {
      cmd=2;
      memory_IN();
       
      
    }
  } 
  else if (command == "left") {
   
    left();
     if(updateMemory)
    {
      cmd=3;
      memory_IN();
       
      
    }
  } 
  else if (command == "right") {
    
    right();
     if(updateMemory)
    {
      cmd=4;
      memory_IN();
       
      
    }
  } 
  else if (command == "stop") {
    
    stop();
     if(updateMemory)
    {
       cmd=5;
      memory_IN();
      
      
    }
  }
  else if (command == "1") {
      updateMemory = true;

       addr = 1;

       addr_Start = 0; 
       address_End =0;


      //  i = 180;
  }
  else if (command == "2") {
    updateMemory = true;

       addr = 51;

       addr_Start = 50; 
       address_End =50;

      //  i = 180;

  }
  else if (command == "3") {
    updateMemory = true;

       addr = 101;

       addr_Start = 100; 
       address_End =100;

  //      i = 180;
   }
  else if (command == "4") {
      updateMemory = false;

    //  i = 175;

      addr_End=EEPROM.read(0);

     addr_Start = 1;
    
     memory_OUT();
    
  }
    else if (command == "5") {
    updateMemory = false;

    //  i = 175;

     addr_End = EEPROM.read(50);

     addr_Start = 51;

     
     memory_OUT();
    
  }

  
  
    else if (command == "6") {
   updateMemory = false;

    //  i = 175;

     addr_End = EEPROM.read(100);

     addr_Start = 101;

    
     memory_OUT();
    
  }

  
    else if (command == "0") {
   updateMemory = false; 

  
  }
      else if (command == "reset") {
    for (int i = 0; i < EEPROM_SIZE; i++) {
        EEPROM.write(i, 0); // Write 0 to each address
    }
    EEPROM.commit(); // Commit changes to make sure they are saved to EEPROM
    Serial.println("EEPROM has been cleared.");

  
  }
   else {
    Serial.println("Unknown command");
  }

}



void memory_IN()

  {

    
    unsigned long currentTime = millis();
    unsigned long timeDiff;
    if(addr==1 || addr==51 || addr==101){
    timeDiff =0;
    }
    else{
    timeDiff = currentTime - lastCommandTime;
    
    }
    lastCommandTime = currentTime;
    EEPROM.write(addr, cmd);
    EEPROM.commit();
    Serial.print("Written cmd at addr: "); Serial.print(addr); Serial.print(" value: "); Serial.println(EEPROM.read(addr));
    addr++;
    // Assuming timeDiff fits in 2 bytes. Adjust accordingly if your timing needs more precision.
    EEPROM.write(addr, (timeDiff >> 8) & 0xFF); // Write the high byte of timeDiff
    EEPROM.commit();
    addr++;
    EEPROM.write(addr, timeDiff & 0xFF); // Write the low byte of timeDiff
    EEPROM.commit();
    addr++;
    // EEPROM.commit();

    // Serial.print("Written cmd at addr: "); Serial.print(addr); Serial.print(" value: "); Serial.println(EEPROM.read(addr));
    // Serial.print("Time diff: "); Serial.println(timeDiff);
    // Serial.print(" addr_start="); Serial.print(addr_Start); Serial.print(" addr_end="); Serial.println(addr_End);

    addr_End = addr; // Update addr_End to the new end of the sequence
    EEPROM.write(address_End,addr_End);
    EEPROM.commit();

  }

void memory_OUT()

  {

  goPath();

  delay(10000);

  returnPath();

  }

void goPath()

  {

  for(addr=addr_Start; addr < addr_End;)

  {

  int value = EEPROM.read(addr++);
  unsigned int timeHigh = EEPROM.read(addr++);
  unsigned int timeLow = EEPROM.read(addr++);
  unsigned int timeDiff = (timeHigh << 8) | timeLow;

  delay(timeDiff);


   if( value == 1)

   {
    Serial.println("confirm 1");
    forward();
    

    

    }

   else if(value == 2)

   {
    Serial.println("confirm 2");
    backward();
    

    
    }

   else if(value == 3)

   {
   
    Serial.println("confirm 3");
    left();
    
    }

   else if(value == 4)

   {

    Serial.println("confirm 4");
    right();
    

    }

    else if(value == 5)

   {

    Serial.println("confirm 5");
    stop();
    

    }


    // delay(timeDiff);
  }
}

void returnPath()

  {

 
    right();  
    stop();

  for(addr=addr_End; addr > addr_Start;)

  {
    // Serial.print("Written cmd at addr: "); Serial.print(addr); Serial.print(" value: ");Serial.println(cmd);

    // Serial.print(" addr_start=");Serial.print(addr_Start);Serial.print("addr_end=");Serial.println(addr_End);

        unsigned int timeLow = EEPROM.read(--addr); // Decrease first, then read the low byte of the time
        unsigned int timeHigh = EEPROM.read(--addr); // Read the high byte of the time
        int value = EEPROM.read(--addr); // Read the command
        unsigned int timeDiff = (timeHigh << 8) | timeLow; 

  // delay(timeDiff);
  int value2 = EEPROM.read(addr);

   if(value2 == 1)

   {
    forward();
    Serial.println("confirm out 1");
    
    }

   else if(value2 == 2)

   {
    backward();
    Serial.println("confirm out 2");
    
    }

   else if(value2 == 3)

   {
    
    left();
    Serial.println("confirm out 3 ");
    }
   else if(value2 == 4)

   {
    Serial.println("confirm out 4");
    right();
    }

     else if(value2 == 5)

   {
    Serial.println("confirm out 5");
    stop();
    }
    delay(timeDiff);
  }
  }


 void rgb(){
    for (int j = 0; j < 255; j += 2) {
    for (int i = 0; i < LEDS_COUNT; i++) {
      strip.setLedColorData(i, strip.Wheel((i * 256 / LEDS_COUNT + j) & 255));
    }
    strip.show();
    delay(2);
  }  
  }


void forward()
  {
     analogWrite(speedpin1,250);//input a value to set the speed
      analogWrite(speedpin2,250);//input a value to set the speed
      digitalWrite(pinI1,LOW);// DC motor rotates clockwise
      digitalWrite(pinI2,HIGH);
      digitalWrite(pinI3,LOW);// DC motor rotates clockwise
      digitalWrite(pinI4,HIGH);
      digitalWrite(LED, HIGH);
      delay(500);
      stop();
  }
  void backward()
  {
      analogWrite(speedpin1,250);//input a value to set the speed
      analogWrite(speedpin2,250);//input a value to set the speed
      digitalWrite(pinI1,HIGH);// DC motor rotates clockwise
      digitalWrite(pinI2,LOW);
      digitalWrite(pinI3,HIGH);// DC motor rotates clockwise
      digitalWrite(pinI4,LOW);
      digitalWrite(LED,LOW);
      delay(500);
      stop();
  }
  void left()
  {
      analogWrite(speedpin1,250);//input a value to set the speed
      analogWrite(speedpin2,250);//input a value to set the speed
      digitalWrite(pinI1,HIGH);// DC motor rotates clockwise
      digitalWrite(pinI2,LOW);
      digitalWrite(pinI3,LOW);// DC motor rotates clockwise
      digitalWrite(pinI4,HIGH);
      digitalWrite(LED,LOW);
      delay(500);
      stop();
      
  }
  void right()
  {
      analogWrite(speedpin1,250);//input a value to set the speed
      analogWrite(speedpin2,250);//input a value to set the speed
      digitalWrite(pinI1,LOW);// DC motor rotates clockwise
      digitalWrite(pinI2,HIGH);
      digitalWrite(pinI3,HIGH);// DC motor rotates clockwise
      digitalWrite(pinI4,LOW);
      digitalWrite(LED,LOW);
      delay(500);
      stop();
      
  }
  void stop()
  {
      analogWrite(speedpin1,250);//input a value to set the speed
      analogWrite(speedpin2,220);//input a value to set the speed
      digitalWrite(pinI1,HIGH);// DC motor rotates clockwise
      digitalWrite(pinI2,HIGH);
      digitalWrite(pinI3,HIGH);// DC motor rotates clockwise
      digitalWrite(pinI4,HIGH);
      digitalWrite(LED,LOW);
  }





