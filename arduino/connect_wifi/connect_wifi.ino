#include <WiFi.h>

const int INT_LED_PIN = 8;
char ssid[] = "my_wifi_a1";
char pass[] = "kikker123";

bool internetConnected = false;

void slowBlinkOnce() {
  digitalWrite(INT_LED_PIN, HIGH);
  delay(500);
  digitalWrite(INT_LED_PIN, LOW);
  delay(500);
}

void fastBlinkOnce() {
  digitalWrite(INT_LED_PIN, HIGH);
  delay(100);
  digitalWrite(INT_LED_PIN, LOW);
  delay(100);
}

bool testInternetConnection() {
  Serial.println("Testing internet connection to google.com...");
  
  WiFiClient client;
  
  if (client.connect("google.com", 80)) {
    Serial.println("✓ Successfully connected to google.com");
    Serial.println("  Internet connection verified!");
    
    client.println("HEAD / HTTP/1.1");
    client.println("Host: google.com");
    client.println("Connection: close");
    client.println();
    
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println("  Timeout waiting for response");
        client.stop();
        return false;
      }
    }
    
    if (client.available()) {
      String line = client.readStringUntil('\n');
      Serial.print("  Server response: ");
      Serial.println(line);
    }
    
    client.stop();
    return true;
    
  } else {
    Serial.println("✗ Failed to connect to google.com");
    Serial.println("  WiFi connected but no internet access");
    return false;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n=== ESP32 WiFi Connection Test ===");
  
  pinMode(INT_LED_PIN, OUTPUT);
  
  Serial.println("Starting up (slow blink)...");
  for (int i = 0; i < 3; i++) {
    slowBlinkOnce();
  }
  
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  
  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 30000) {
    fastBlinkOnce();
    Serial.print(".");
  }
  
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("✓ WiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal Strength (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    Serial.print("Gateway: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("DNS: ");
    Serial.println(WiFi.dnsIP());
    Serial.println();
    
    // Test internet connectivity
    internetConnected = testInternetConnection();
    
    if (internetConnected) {
      Serial.println("\n=== Connection Test PASSED ===");
      digitalWrite(INT_LED_PIN, HIGH);
    } else {
      Serial.println("\n=== WiFi connected but no internet access ===");
      Serial.println("Will retry every 10 seconds...");
      digitalWrite(INT_LED_PIN, LOW);
    }
    
  } else {
    digitalWrite(INT_LED_PIN, LOW);
    Serial.println("✗ WiFi Connection FAILED");
    Serial.println("Check your SSID and password");
    
    // Slow blink forever to indicate error
    while(1) { 
      slowBlinkOnce();
    }
  }
}

void loop() {
  static unsigned long lastCheck = 0;
  
  // If internet is not connected, check every 10 seconds
  // If internet is connected, check every 30 seconds
  unsigned long checkInterval = internetConnected ? 30000 : 10000;
  
  if (millis() - lastCheck > checkInterval) {
    lastCheck = millis();
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n--- Connection Check ---");
      Serial.print("Signal: ");
      Serial.print(WiFi.RSSI());
      Serial.println(" dBm");
      digitalWrite(INT_LED_PIN, HIGH);
      
      bool wasConnected = internetConnected;
      internetConnected = testInternetConnection();
      
      if (internetConnected) {
        digitalWrite(INT_LED_PIN, HIGH);
        if (!wasConnected) {
          Serial.println("✓ Internet connection restored!");
        }
      } else {
        digitalWrite(INT_LED_PIN, LOW);
        Serial.println("Still no internet. Retrying in 10 seconds...");
      }
      
    } else {
      Serial.println("WiFi disconnected! Attempting reconnect...");
      digitalWrite(INT_LED_PIN, LOW);
      internetConnected = false;
      WiFi.reconnect();
      
      // Fast blink while reconnecting
      unsigned long startAttempt = millis();
      while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 15000) {
        fastBlinkOnce();
      }
      
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("✓ WiFi reconnected!");
      }
    }
  }
}
