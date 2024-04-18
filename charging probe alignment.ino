
#include <s3servo.h>

static const int servoPin = 4;

s3servo servo;

void setup() {
    Serial.begin(115200);
    servo.attach(servoPin);
    //servo.attach(servoPin,0,0,180,500,2000); 
}

void loop() {
    for(int angle = 0; angle <= 180; angle++) {
        servo.write(angle);
        Serial.println(angle);
        delay(20);
    }

    for(int angle = 180; angle >= 0; angle--) {
        servo.write(angle);
        Serial.println(angle);
        delay(20);
    }
}
