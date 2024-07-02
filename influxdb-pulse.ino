#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <ESP8266WiFi.h>
//#include <ThingSpeak.h>, commenting out all thingspeak code
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

#define DEVICE "ESP8266"

#define INFLUXDB_URL "https://us-east-1-1.aws.cloud2.influxdata.com"
#define INFLUXDB_TOKEN "8ioPMvCmBJE-WxxHU6JAEo33FTNCrNtyG-RMh9pqtnjdpmBf7zvCmJjYj-_wgypktejW2b47BURGgNWfSuSROg=="
#define INFLUXDB_ORG "1fb2665615ea142b"
#define INFLUXDB_BUCKET "HeartRateMonitor"
  
// Time zone info
#define TZ_INFO "IST-5:30"
  
// Declare InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
  
// Declare Data point
Point sensor("heart_rate");

Adafruit_SSD1306 display(128,64,&Wire);
//WiFiClient client;
const int sensorPin = A0;                               // A0 is the input pin for the heart rate sensor
int sensorValue;                                        // Variable to store the value coming from the sensor
int count = 0;
unsigned long starttime = 0;
int heartrate = 0;
boolean counted = false;
char ssid[] = "Oneplus";
char pass[] = "anusha20";

void setup (void)
  { display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    pinMode (D8, OUTPUT);                              // D8 LED as Status Indicator
    Serial.begin(9600);                             // Start Serial Communication @ 9600
    display.clearDisplay();
    WiFi.begin(ssid, pass);   //takes the wifi ssid and password
      while(WiFi.status() != WL_CONNECTED)
      {delay(200);
        Serial.print("..");}
      Serial.println();
      Serial.println("NodeMCU is connected!");
      Serial.println(WiFi.localIP());

      // Add tags
      sensor.addTag("device", DEVICE);
      sensor.addTag("SSID", WiFi.SSID());

      timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");
  
      // Check server connection
      if (client.validateConnection()) {
        Serial.print("Connected to InfluxDB: ");
        Serial.println(client.getServerUrl());
      } else {
        Serial.print("InfluxDB connection failed: ");
        Serial.println(client.getLastErrorMessage());
      }
  }

void loop ()
  { starttime = millis();
    while (millis()<starttime+20000)                                            // Reading pulse sensor for 20 seconds
    { sensorValue = analogRead(sensorPin);
        //Serial.println (sensorValue);
          delay(50);
      if ((sensorValue >= 590 && sensorValue <=680) && counted == false)       // Threshold value is 590 (~ 2.7V)
      {  count++;
        digitalWrite (D8,HIGH);
          delay (250);       //was 10 originally
        digitalWrite (D8, LOW);
        counted = true;
        }
      else if (sensorValue < 590)
       {  counted = false;
          digitalWrite (D8, LOW);
          }
      }
    Serial.print ("Pulse ");
    Serial.println (count);
    heartrate = (count)*3;                              // Multiply the count by 3 to get beats per minute
    Serial.println ();
    Serial.print ("BPM = ");
    Serial.println (heartrate);                         // Display BPM in the Serial Monitor
    Serial.println ();
    count = 0;

    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.setTextSize(2);
    display.println("Heart Rate");
    display.setCursor(0,28);
    display.print("BPM: ");
    display.print(heartrate);
    display.display();

    sensor.clearFields();
    sensor.addField("BPM", heartrate);
    //print what exactly we are writing
    Serial.print("Writing: ");
    Serial.println(client.pointToLineProtocol(sensor));

    // Write point
    if (!client.writePoint(sensor)) {
      Serial.print("InfluxDB write failed: ");
      Serial.println(client.getLastErrorMessage());
    }

    //Wait 10s
    Serial.println("Wait 10s");
    delay(10000);
}
