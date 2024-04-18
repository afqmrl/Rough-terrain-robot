// #include "String.h"
// #include "EEPROM.h"
// #include "Freenove_WS2812_Lib_for_ESP32.h" 
// #include <charging_dock_inferencing.h>
// #include "edge-impulse-sdk/dsp/image/image.hpp"
// #include <WiFi.h>
// #include <WebServer.h>
// #include "img_converters.h"
// #include "Arduino.h"
// #include <s3servo.h>
// #include "index_html.h"
// #include "Config.h"
// #include "CameraConfig.h"
// #include "Camera_detection.h"
// #include "camera_html.h"






// void setup() {
//      if (!EEPROM.begin(EEPROM_SIZE)) {
//         Serial.println("Failed to initialise EEPROM");
//         return;
//     }
//   pinMode(LED, OUTPUT);
//   Serial.begin(115200);
//   pinMode(pinI1,OUTPUT);//define this port as output
//   pinMode(pinI2,OUTPUT);
//   pinMode(speedpin1,OUTPUT);
//   pinMode(pinI3,OUTPUT);//define this port as output
//   pinMode(pinI4,OUTPUT);
//   pinMode(speedpin2,OUTPUT);
//   strip.begin();
//   setupUltrasonic();
//   myServo.attach(SERVO_PIN);


  
//   // Setup WiFi
//   // 
  

//   WiFi.begin(ssid, password);  // Connect to the existing network

//     while (WiFi.status() != WL_CONNECTED) {
//         delay(500);
//         Serial.print(".");
//     }

//     Serial.println("");
//     Serial.print("Connected to WiFi network with IP Address: ");
//     Serial.println(WiFi.localIP());


  
//    server.on("/", HTTP_GET, []() {
//     server.send(200, "text/html", index_html);
//   });
//   server.on("/camera", HTTP_GET, []() {
//     server.send(200, "text/html", camera_html); // Serve the camera stream page
//   });
//   server.on("/stream", HTTP_GET, handleStream);
//   server.on("/command", HTTP_GET, handleCommand); // Consolidated command handler
  




//      server.on("/camera-output", HTTP_GET, []() { // Camera output handler
//         if (cameraDetectionResults.isEmpty()) {
//             server.send(204); // No Content
//         } else {
//             server.send(200, "text/plain", cameraDetectionResults);
            
//         }
//     });

 

//   server.begin();

//   while (!Serial);
//     Serial.println("Edge Impulse Inferencing Demo");
//     if (ei_camera_init() == false) {
//         ei_printf("Failed to initialize Camera!\r\n");
//     }
//     else {
//         ei_printf("Camera initialized\r\n");
//     }

//     ei_printf("\nStarting continious inference in 2 seconds...\n");
//     ei_sleep(2000);

   

// }

// void loop() {


//   server.handleClient();
 
//   // cameradetection();

 
// }



// #if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_CAMERA
// #error "Invalid model for current sensor"
// #endif

// void handleStream() {
//     WiFiClient client = server.client();
//     camera_fb_t* fb = nullptr;
//     size_t _jpg_buf_len = 0;
//     uint8_t* _jpg_buf = nullptr;
    
//     client.println("HTTP/1.1 200 OK");
//     client.println("Content-Type: multipart/x-mixed-replace; boundary=" PART_BOUNDARY);
//     client.println();

//     while (client.connected()) {

//         fb = esp_camera_fb_get();
//         if (!fb) {
//             Serial.println("Camera capture failed");
//             continue;
//         }

//         if (fb->format != PIXFORMAT_JPEG) {
//             bool converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
//             esp_camera_fb_return(fb);
//             if (!converted) {
//                 Serial.println("JPEG conversion failed");
//                 continue;
//             }
//         } else {
//             _jpg_buf = fb->buf;
//             _jpg_buf_len = fb->len;
//         }

//         client.print(_STREAM_BOUNDARY);
//         client.printf(_STREAM_PART, _jpg_buf_len);
//         client.write(_jpg_buf, _jpg_buf_len);
        
//         if (fb->format != PIXFORMAT_JPEG) {
//             free(_jpg_buf);
//         }
        
//         esp_camera_fb_return(fb);
        
//     }
// }

// void handleCommand() {
//   if (server.hasArg("act")) {
//     String command = server.arg("act");
//     controlMotors(command);

//     command=" ";
//     server.send(200, "text/plain", "Command received: " + command);
//   } else {
//     server.send(400, "text/plain", "Bad Request: Missing 'act' parameter");
//   }
// }

#include "String.h"
#include "EEPROM.h"
#include "Freenove_WS2812_Lib_for_ESP32.h"
#include <charging_dock_inferencing.h>
#include "edge-impulse-sdk/dsp/image/image.hpp"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "img_converters.h"
#include "Arduino.h"
#include <s3servo.h>
#include "index_html.h"
#include "Config.h"
#include "CameraConfig.h"
#include "Camera_detection.h"
#include "camera_html.h"

AsyncWebServer server(80);

void setup() {
    if (!EEPROM.begin(EEPROM_SIZE)) {
        Serial.println("Failed to initialise EEPROM");
        return;
    }
    pinMode(LED, OUTPUT);
    Serial.begin(115200);
    pinMode(pinI1, OUTPUT);
    pinMode(pinI2, OUTPUT);
    pinMode(speedpin1, OUTPUT);
    pinMode(pinI3, OUTPUT);
    pinMode(pinI4, OUTPUT);
    pinMode(speedpin2, OUTPUT);
    strip.begin();
    setupUltrasonic();
    myServo.attach(SERVO_PIN);

    // Setup WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());

    // Define Web Server Handlers
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", index_html);
    });
    // server.on("/camera", HTTP_GET, [](AsyncWebServerRequest *request) {
    //     request->send(200, "text/html", camera_html);
    // });
    server.on("/stream", HTTP_GET, handleStream);
    server.on("/single", HTTP_GET, handleSingleFrame);


    server.on("/command", HTTP_GET, [](AsyncWebServerRequest *request) {
        handleCommand(request);
    });
    server.on("/camera-output", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (cameraDetectionResults.isEmpty()) {
            request->send(204);
        } else {
            request->send(200, "text/plain", cameraDetectionResults);
        }
    });

    server.begin();
    Serial.println("Edge Impulse Inferencing Demo");
    if (!ei_camera_init()) {
        ei_printf("Failed to initialize Camera!\r\n");
    } else {
        ei_printf("Camera initialized\r\n");
    }
    ei_printf("\nStarting continuous inference in 2 seconds...\n");
    ei_sleep(2000);
}

void loop() {
    // Asynchronous server does not need to call handleClient()
}

void handleStream(AsyncWebServerRequest *request) {
    request->sendChunked("multipart/x-mixed-replace;boundary=" PART_BOUNDARY, [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
        static camera_fb_t *fb = nullptr;
        static size_t _jpg_buf_len = 0;
        static uint8_t *_jpg_buf = nullptr;

        Serial.printf("Handling stream, index: %d\n", index); // Debug output

        if (!index) { // If index is 0, start a new frame
            if (fb) {
                Serial.println("Returning previous frame buffer.");
                if (fb->format != PIXFORMAT_JPEG) {
                    Serial.println("Freeing non-JPEG buffer.");
                    free(_jpg_buf);
                }
                esp_camera_fb_return(fb);
            }
            fb = esp_camera_fb_get();
            if (!fb) {
                Serial.println("Camera capture failed");
                return 0;
            }

            if (fb->format != PIXFORMAT_JPEG) {
                bool converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
                if (!converted) {
                    Serial.println("JPEG conversion failed");
                    esp_camera_fb_return(fb);
                    fb = nullptr;
                    return 0;
                }
                Serial.println("Frame converted to JPEG.");
            } else {
                _jpg_buf = fb->buf;
                _jpg_buf_len = fb->len;
                Serial.println("Frame is JPEG.");
            }
            Serial.printf("Frame buffer length: %d bytes\n", _jpg_buf_len);
        }

        size_t willSend = (maxLen > _jpg_buf_len - index) ? _jpg_buf_len - index : maxLen;
        memcpy(buffer, _jpg_buf + index, willSend);
        Serial.printf("Sending %d bytes.\n", willSend);

        if (index + willSend == _jpg_buf_len) {
            if (fb->format != PIXFORMAT_JPEG) {
                Serial.println("Freeing converted JPEG buffer.");
                free(_jpg_buf);
            }
            Serial.println("Returning frame buffer to driver.");
            esp_camera_fb_return(fb);
            fb = nullptr;
            Serial.println("Frame transmission complete.");
            return 0; // End of frame
        }
        return willSend;
    });
}







void handleSingleFrame(AsyncWebServerRequest *request) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        request->send(500, "text/plain", "Failed to capture image");
    } else {
        // Correctly cast the buffer pointer when sending the image
        request->send_P(200, "image/jpeg", (const uint8_t *)fb->buf, fb->len);
        esp_camera_fb_return(fb);
    }
}




void handleCommand(AsyncWebServerRequest *request) {
    if (request->hasParam("act")) {  // Check without specifying 'true' to look in both POST and GET requests
        AsyncWebParameter* p = request->getParam("act");
        String command = p->value();
        controlMotors(command);
        Serial.println("Command received: " + command);
        request->send(200, "text/plain", "Command received: " + command);
    } else {
        Serial.println("Bad Request: No 'act' parameter found.");
        // Enhanced logging to see what's being passed
        int params = request->params();
        for (int i = 0; i < params; i++) {
            AsyncWebParameter* p = request->getParam(i);
            Serial.println("Param " + String(i) + ": " + p->name() + " = " + p->value());
        }
        request->send(400, "text/plain", "Bad Request: Missing 'act' parameter");
    }
}





void setupUltrasonic() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

long getDistance() {
  digitalWrite(TRIG_PIN, LOW); 
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  long distance = duration * 0.034 / 2;
 
  return distance;
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
      // server.handleClient();

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
   else if (command == "docking") {
  docking =1;

  
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

  // cameradetection();
  Serial.println(logic);

  while (logic != 1){
  // server.handleClient();
  // cameradetection();

  }
 

  docking=0;
  
  myServo.write(130);

  while(docking!=1){
  // server.handleClient();
  if(docking==1){

    myServo.write(55);
    
    returnPath();
    docking=0 ;
    logic=0;

  }

  }
  }
  
void goPath()

  {

  for(addr=addr_Start; addr < addr_End;)

  {

  int value = EEPROM.read(addr++);
  unsigned int timeHigh = EEPROM.read(addr++);
  unsigned int timeLow = EEPROM.read(addr++);
  unsigned int timeDiff = (timeHigh << 8) | timeLow;
  long distance =getDistance();

    // if(distance <= 5){

    //   myServo.write(180);
    //   delay(20);
    //   long right_distance=getDistance();
    //   myServo.write(0);
    //   delay(20);
    //   long left_distance=getDistance();
    //   delay(20);

      
    //   if (right_distance > left_distance){

    //     right_correction();
    //   }
    //   else {
    //     left_correction();
    //   }
    //   }

      // else{


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
  // }
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

  long distance =getDistance();

    // if(distance <= 5){

    //   myServo.write(180);
    //   delay(20);
    //   long right_distance=getDistance();
    //   myServo.write(0);
    //   delay(20);
    //   long left_distance=getDistance();
    //   delay(20);

    //   if (right_distance > left_distance){

    //     right_correction();
    //   }
    //   else {
    //     left_correction();
    //   }
    //   }

    // else {
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
  // }


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
      delay(1500);
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
      delay(1500);
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


  void right_correction(){

        right();
        delay(10);
        forward();
        delay(10);
        left();
        delay(10);
        forward();
        delay(10);
        left();
        delay(10);
        forward();
        delay(10);
        right();
        delay(10);


  }

  void left_correction(){

        left();
        delay(10);
        forward();
        delay(10);
        right();
        delay(10);
        forward();
        delay(10);
        right();
        delay(10);
        forward();
        delay(10);
        left();
        delay(10);



  }

