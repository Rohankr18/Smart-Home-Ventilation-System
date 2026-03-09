#include <DHT.h>
#include <ESP32Servo.h>
#define BLYNK_TEMPLATE_ID "TMPL3LWAtZ7AB"
#define BLYNK_TEMPLATE_NAME "Smart Ventilation"

#define BLYNK_AUTH_TOKEN "Z4B71kh65EpL8ozAJYoMdDLQb4e5uaps"
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
// WiFi and Blynk credentials
char auth[] = "Z4B71kh65EpL8ozAJYoMdDLQb4e5uaps";
char ssid[] = "OnePlus Nord CE4";
char pass[] = "smith2004";
#define DHTPIN 5
#define DHTTYPE DHT11
#define SERVO_PIN 14
#define TOUCH_PIN 27
DHT dht(DHTPIN, DHTTYPE);
Servo ventServo;
const int humidityThreshold = 85;
const int offset = 10;
bool overrideMode = false;
bool lastTouchState = false;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 300;
int currentServoPos = 180;
// Blynk Switch Handler (V0): 0 = Auto, 1 = Manual
BLYNK_WRITE(V0)
{
    int pinValue = param.asInt(); // 1 = Manual, 0 = Auto
    overrideMode = pinValue;
    Serial.println(overrideMode ? "Manual Override ON [Blynk]" : "Auto Mode Enabled [Blynk]");
    if (overrideMode)
    {
        if (currentServoPos > 90)
        {
            currentServoPos = 0 + offset;
        }
        else
        {
            currentServoPos = 180 + offset;
        }
        ventServo.write(currentServoPos);
        Serial.print("Manual Control (Blynk): Servo Position ");
        Serial.println(currentServoPos);
    }
}
void setup()
{
    Serial.begin(115200);
    dht.begin();
    pinMode(22, OUTPUT);
    digitalWrite(22, HIGH);
    ventServo.attach(SERVO_PIN, 500, 2400);
    ventServo.write(currentServoPos);
    Blynk.begin(auth, ssid, pass);
    Serial.println("System Ready ");
}
void loop()
{
    Blynk.run(); // Run Blynk tasks
    int touchValue = touchRead(TOUCH_PIN);
    Serial.print("Touch Value: ");
    Serial.println(touchValue);
    bool currentTouch = (touchValue < 30);
    unsigned long currentTime = millis();
    if (currentTouch && !lastTouchState && (currentTime - lastDebounceTime > debounceDelay))
    {
        overrideMode = !overrideMode;
        Blynk.virtualWrite(V0, overrideMode); // Sync switch in app
        Serial.println(overrideMode ? "Manual Override ON " : "Auto Mode Enabled ");
        if (overrideMode)
        {
            if (currentServoPos > 90)
            {
                currentServoPos = 0 + offset;
            }
            else
            {
                currentServoPos = 180 + offset;
                
            }
            ventServo.write(currentServoPos);
            Serial.print("Manual Control (Touch): Servo Position ");
            Serial.println(currentServoPos);
        }
        lastDebounceTime = currentTime;
    }
    lastTouchState = currentTouch;
    if (!overrideMode)
    {
        float humidity = dht.readHumidity();
        if (isnan(humidity))
        {
            Serial.println("Sensor Error");
        }
        else
        {
            Serial.print("Humidity: ");
            Serial.print(humidity);
            Serial.println(" %");
            Blynk.virtualWrite(V1, humidity);
            if (humidity > humidityThreshold)
            {
                currentServoPos = 0 + offset;
                ventServo.write(currentServoPos);
                Serial.println("Vent OPEN ");
            }
            else
            {
                currentServoPos = 180 + offset;
                ventServo.write(currentServoPos);
                Serial.println("Vent CLOSED");
            }
        }
    }
    delay(500);
}