#include <PZEM004Tv30.h>
#include <WiFi.h>
#include <WebServer.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HTTPClient.h>

// Set the LCD address to 0x27 or 0x3F (Check your module's I2C address)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Telegram Bot Token & Chat ID
#define BOT_TOKEN "Telegram bot token"
#define CHAT_ID "your telegram chat id"

// WiFi Credentials
#define WIFI_SSID "wifi id"
#define WIFI_PASS "password"

// Adafruit IO Credentials
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "adafruit id"
#define AIO_KEY         "adafruit key"

const int ledpin = 2;
float bill=0;

// WiFi Client
WiFiClient client;

// Web server on port 80
WebServer server(80);

// Adafruit MQTT Client
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// Adafruit IO Feed
Adafruit_MQTT_Subscribe amountFeed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/amount");
float payment;
float payment2=0;
int flag2=0;

#define PZEM_RX_PIN 16
#define PZEM_TX_PIN 17
#define PZEM_SERIAL Serial2
PZEM004Tv30 pzem(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN);
float totalEnergy = 0.0;

void setup() {
    pinMode(ledpin,OUTPUT);
    Wire.begin(21, 22); // SDA = GPIO 21, SCL = GPIO 22 (ESP32 default I2C)
    lcd.init();         // Initialize LCD
    lcd.backlight();    // Turn on the backlight
    lcd.clear();

    // Debugging Serial port
    Serial.begin(115200);
        delay(10);

    // Connect to WiFi
    Serial.print("Connecting to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected!");
    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());

    // Define webpage route
    server.on("/", handleRoot);
    server.begin();
    Serial.println("Web server started");

    // Connect to MQTT (Adafruit IO)
    mqtt.subscribe(&amountFeed);

}
void loop() {
  server.handleClient();  // Handle web requests
  //displaySerial();        // Display sensor values in Serial Monitor

  float bill0=adafruitconnect();
  bill=bill+bill0;
  Serial.println(bill);
  Serial.println(bill0);

    lcd.setCursor(0, 1);
    lcd.print("               ");
    lcd.setCursor(0, 1);
    lcd.print("balance:");
    lcd.print(bill/100);



  float voltage = pzem.voltage();
  float current = pzem.current();
  float energy = pzem.energy();  // Energy in Wh for the current session
  if(isnan(energy)){
    energy=0;
  }
  float energy0=voltage*current*3.5;
  if(isnan(voltage)){
    energy0=0;
  }
  if(bill!=0){
    float bill_dummy=bill_calcules(bill,energy0);
    bill=bill_dummy;
  }


  // Calculate power consumption for current session in kWh
  float currentEnergy = energy / 1000.0;  // Convert Wh to kWh

  // Accumulate total energy consumption in kWh (only while ESP32 is powered on)
  if (!isnan(currentEnergy) && currentEnergy >= 0) {
    totalEnergy += currentEnergy;  // Add current energy to temporary total
  }
  

  if (isnan(voltage)) {
    Serial.println("Error reading PZEM sensor!");
    lcd.setCursor(0, 0);
    lcd.print("               ");
    lcd.setCursor(0, 0);
    lcd.print("Error!");
  } else {
    Serial.print("Voltage: "); Serial.print(voltage); Serial.println(" V");
    Serial.print("Current: "); Serial.print(current); Serial.println(" A");
    Serial.print("Energy (Session): "); Serial.print(energy); Serial.println(" Wh");
    Serial.print("Total Power Consumption (Temp): "); Serial.print(totalEnergy); Serial.println(" kWh");
    Serial.print("bill:");Serial.println(bill);
    Serial.println("-----------------------------");
    lcd.setCursor(0, 0);
    lcd.print("               ");
    lcd.setCursor(0, 0);
    lcd.print("volt:");
    lcd.print(voltage);
  }

  delay(500);  // Refresh every 2 seconds
  if(WiFi.status() != WL_CONNECTED){
   wififixing();
  }
}


float adafruitconnect(){
      // Ensure MQTT connection
    MQTT_connect();
    int flag=0;
    // Read data from Adafruit IO
    Adafruit_MQTT_Subscribe *subscription;
    while ((subscription = mqtt.readSubscription(3000))) {
        if (subscription == &amountFeed) {
            payment = atoi((char *)amountFeed.lastread);
            payment=payment+5;
            payment2=payment;
            flag=flag+1;
            // Print received value
            Serial.print("Received Feed Value (amount): ");
            Serial.println(payment);
            lcd.setCursor(0, 1);
            lcd.print("               ");
            lcd.setCursor(0, 1);
            lcd.print("Credicted:");
            lcd.print(payment);
            sendTelegramMessage("payment successful!");
          
            
        }
        else{
          payment=0;
        }
    }
    if(isnan(payment)){
      payment=0;
      payment2=0;
    }
    else if(flag==0){
      payment=0;
    }
  delay(10);
  return payment;
}

// Function to connect/reconnect to MQTT
void MQTT_connect() {
    int8_t ret;

    if (mqtt.connected()) {
        return;
    }

    Serial.print("Connecting to Adafruit IO...");
    while ((ret = mqtt.connect()) != 0) {
        Serial.println(mqtt.connectErrorString(ret));
        Serial.println("Retrying in 5 seconds...");
        mqtt.disconnect();
        delay(5000);
    }
    Serial.println("Connected to Adafruit IO!");
}

float bill_calcules(float bill2,float energy2 ){
  float energy3=energy2/10;
  float bill3=bill2-energy2;
  //bill3=bill3-15;
  if(bill3<=0){
        digitalWrite(ledpin,0);
        bill3=0;
        sendTelegramMessage("⚠️ Alert:your quata is finished, Recharge your balance! ⚠️"); 
  }
  else{
    digitalWrite(ledpin,1);
  }
  if(bill3<=payment2/2){
    if(flag2==0){
      sendTelegramMessage("⚠️ Alert: 50% of amount used.save electricity ⚠️");
      flag2=flag2+1; 
    }
  }
  else {
    flag2=0;
  }
  return bill3;
}

// Function to serve stylish webpage with live sensor values
void handleRoot() {
    float voltage = pzem.voltage();
    float current = pzem.current();
    //float power = pzem.power();
    float energy = pzem.energy();
    float webbill=bill/100;

    String html = "<html><head><title>ESP32 Power Monitor</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<meta http-equiv='refresh' content='2'>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; text-align: center; background-color: #f4f4f4; margin: 0; padding: 0; }";
    html += ".container { max-width: 400px; margin: auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); margin-top: 50px; }";
    html += "h2 { color: #333; }";
    html += ".data-box { background: #007bff; color: white; padding: 10px; border-radius: 5px; margin: 10px 0; font-size: 20px; }";
    html += ".label { font-weight: bold; font-size: 18px; }";
    html += "</style>";
    html += "</head><body>";
    html += "<div class='container'>";
    html += "<h2>WATTPAY</h2>";
    html += "<div class='data-box'><span class='label'>Voltage:</span> " + String(voltage) + " V</div>";
    html += "<div class='data-box'><span class='label'>Current:</span> " + String(current) + " A</div>";
    html += "<div class='data-box'><span class='label'>Balance:</span> " + String(webbill) + " rs</div>";
    html += "<div class='data-box'><span class='label'>Energy:</span> " + String(energy) + " kWh</div>";
    html += "</div>";
    html += "</body></html>";

    server.send(200, "text/html", html);
}

void sendTelegramMessage(String message) {
message.replace(" ", "%20");  // Replace spaces with %20 for URL encoding

    String url = "https://api.telegram.org/bot" + String(BOT_TOKEN) + "/sendMessage?chat_id=" + CHAT_ID + "&text=" + message;

    Serial.println("Sending message...");
    Serial.println("URL: " + url);  // Debugging: Print the URL

    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();
    String payload = http.getString();  // Get response from server

    Serial.println("HTTP Code: " + String(httpCode));  // Print HTTP code
    Serial.println("Server Response: " + payload);  // Print full response

    if (httpCode == 200) {
        Serial.println("✅ Message sent successfully!");
    } else {
        Serial.println("❌ Failed to send message!");
    }

    http.end();
}

void wififixing(){
      // Connect to WiFi
      int count=0;
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        count=count+1;
        if(count==6){
          break;
        }
    }
    Serial.println("\nConnected to WiFi");
    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());

    // Define webpage route
    server.on("/", handleRoot);
    server.begin();
    Serial.println("Web server started");
}
