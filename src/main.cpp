#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <DHT.h>

// Configuration
#define WIFI_SSID "Galaxy A13 00F4"
#define WIFI_PASSWORD "elfv3871"
#define API_KEY "AIzaSyCKZolSWzhB-cDVnJX6G1gGAx40ltYbh74"
#define DATABASE_URL "https://esp32-test-fc532-default-rtdb.firebaseio.com/"
#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;

void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ Connected to WiFi!");
  } else {
    Serial.println("\n❌ WiFi Connection Failed!");
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  connectWiFi();

  // Configure Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  
  // Sign in to Firebase (Anonymous)
  Serial.println("Signing in to Firebase...");
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("✅ Firebase authentication successful!");
    signupOK = true;
  } else {
    Serial.printf("❌ Firebase authentication failed: %s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (!isnan(temp) && !isnan(humidity)) {
    Serial.printf("📊 Temp: %.2f°C | Humidity: %.2f%%\n", temp, humidity);

    if (Firebase.ready() && signupOK) {
      // Send Temperature
      if (Firebase.RTDB.setFloat(&fbdo, "/sensor/temperature", temp)) {
        Serial.println("✅ Temperature updated in Firebase!");
      } else {
        Serial.printf("❌ Failed to send temperature: %s\n", fbdo.errorReason().c_str());
      }

      // Send Humidity
      if (Firebase.RTDB.setFloat(&fbdo, "/sensor/humidity", humidity)) {
        Serial.println("✅ Humidity updated in Firebase!");
      } else {
        Serial.printf("❌ Failed to send humidity: %s\n", fbdo.errorReason().c_str());
      }
    } else {
      Serial.println("❌ Firebase is not ready or authentication failed!");
    }
  } else {
    Serial.println("❌ Sensor reading failed!");
  }

  delay(5000);
}
