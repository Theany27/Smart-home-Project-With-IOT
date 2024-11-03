#define BLYNK_TEMPLATE_ID "TMPL6hNBooLWl"
#define BLYNK_TEMPLATE_NAME "Group4"
#define BLYNK_PRINT Serial

#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <SoftwareSerial.h>
#include <ESP8266_Lib.h>
#include <BlynkSimpleShieldEsp8266.h>

// Pin definitions
#define white_LED_PIN A1
#define WHITE_PIN 13
#define BUZZER_PIN A2
#define SERVO_PIN A0
#define IR_SENSOR_PIN 12

// Constants
#define PASS_LENGTH 4
#define SERVO_OPEN 0
#define SERVO_CLOSE 90
#define DOOR_MOVE 12
#define DOOR_OPEN_DURATION 2000
#define PASSWORD "5555"
#define DISABLE_ALARM_PASSWORD "4444"

// Blynk setup
char auth[] = "UWyEWxhO00U0NsNayQjELYdnVqhE9oA8";
char ssid[] = "Hello";
char pass[] = "113333555555";

SoftwareSerial EspSerial(10, 11); // RX, TX
#define ESP8266_BAUD 38400
ESP8266 wifi(&EspSerial);

const byte rows = 4;
const byte columns = 4;
char keyboard_keys[rows][columns] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};
byte row_pins[rows] = {9, 8, 7, 6};
byte column_pins[columns] = {5, 4, 3, 2};

char entered_password[PASS_LENGTH + 1];
byte current_sign_position = 0;
int failed_attempts = 0;
bool alarm_triggered = false;

unsigned long prev_time = 0;
const long duration = 200;

Keypad keypad = Keypad(makeKeymap(keyboard_keys), row_pins, column_pins, rows, columns);
Servo servo;
LiquidCrystal_I2C lcd(0x27, 16, 2);

BLYNK_WRITE(V0)
{
    digitalWrite(white_LED_PIN, param.asInt() ? HIGH : LOW);
}

void setup() {
    Serial.begin(9600);
    EspSerial.begin(ESP8266_BAUD);
    delay(10);
    Blynk.begin(auth, wifi, ssid, pass);

    servo.attach(SERVO_PIN);
    servo.write(SERVO_CLOSE);
    pinMode(white_LED_PIN, OUTPUT);
    pinMode(WHITE_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(IR_SENSOR_PIN, INPUT);
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Welcome to");
    lcd.setCursor(1, 1);
    lcd.print("GROUP 4");
    delay(2000);
    enterPassword();
    keypad.setDebounceTime(20);
}

void loop() {
    Blynk.run();
    checkStatus();
    checkIrSensor();
    if (alarm_triggered) {
        triggerAlarm();
    }
}

void enterPassword() {
    current_sign_position = 0;
    lcd.init();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter password");
}

void checkStatus() {
    char key = keypad.getKey();
    if (key) {
        if (key == '*') {
            entered_password[current_sign_position] = '\0';
            if (alarm_triggered) {
                if (strcmp(entered_password, DISABLE_ALARM_PASSWORD) == 0) {
                    lcd.clear();
                    disableAlarm();
                    enterPassword();
                } else {
                    failed_attempts++;
                    lcd.clear();
                    lcd.print("Wrong password");
                    delay(1000);
                }
            } else {
                if (strcmp(entered_password, PASSWORD) == 0) {
                    lcd.clear();
                    lcd.print("Valid password");
                    failed_attempts = 0;
                    manageDoor();
                    enterPassword();
                } else {
                    failed_attempts++;
                    tone(BUZZER_PIN, 2000, 200);
                    if (failed_attempts >= 3) {
                        alarm_triggered = true;
                    }
                }
            }
            enterPassword();
        } else {
            if (current_sign_position < PASS_LENGTH) {
                entered_password[current_sign_position++] = key;
            }
        }
    }
}

void openDoor() {
    lcd.clear();
    lcd.print("Opening the door");
    for (int pos = SERVO_CLOSE; pos >= SERVO_OPEN; pos--) {
        //tone(BUZZER_PIN, 2000, 200);
        servo.write(pos);
        delay(DOOR_MOVE);
         tone(BUZZER_PIN, 1000);
    }
     noTone(BUZZER_PIN);
}

void closeDoor() {
    lcd.clear();
    lcd.print("Closing the door");
    for (int pos = SERVO_OPEN; pos <= SERVO_CLOSE; pos++) {
        servo.write(pos);
        delay(DOOR_MOVE);
        tone(BUZZER_PIN, 1000);
    }
    noTone(BUZZER_PIN);
    delay(1000);
}

void manageDoor() {
    openDoor();
    delay(DOOR_OPEN_DURATION);
    closeDoor();
}

void triggerAlarm() {
    unsigned long current_time = millis();
    if (current_time - prev_time >= duration) {
        prev_time = current_time;
        tone(BUZZER_PIN, 1000, 200);
    }
}

void disableAlarm() {
    alarm_triggered = false;
    failed_attempts = 0;
    delay(1000);
    noTone(BUZZER_PIN);
}

void checkIrSensor() {
    if (digitalRead(IR_SENSOR_PIN) == LOW) { // Assuming LOW indicates detection
        digitalWrite(WHITE_PIN, HIGH);
    }else{
      digitalWrite(WHITE_PIN, LOW);
    }
}
