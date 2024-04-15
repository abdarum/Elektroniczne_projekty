/*
  Script working as a Watchdog of the router device

  If google.com can not be reached a few attempts in a row, then router device will be rebooted
*/
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_task_wdt.h> // Include the watchdog timer library

#include "secrets.h" // Copy secrets_example.h to secrets.h and fill up

#define REBOOT_NOTIFICATION_LED 33 // Red led - reverse logic
#define ROUTER_POWER_DIS_PIN 12    // P-chanel Mosfet - reverse logic, so HIGH - power for router is disabled ; LOW - Power for router is enabled -

#define TEST_CONNECTION_LOOP_TIMEOUT 30   // seconds - check status every X seconds
#define REBOOT_AFTER_X_CONNECTION_FAILS 4 // After X failed test loop connections the router will be rebooted
#define REBOOT_POWER_DOWN_TIMEOUT 90      // seconds - how much time router will be turned while restarting

#define SETUP_ROUTER_POWER_PINOUTS       \
  pinMode(ROUTER_POWER_DIS_PIN, OUTPUT); \
  pinMode(REBOOT_NOTIFICATION_LED, OUTPUT);
#define ENABLE_ROUTER_POWER                \
  digitalWrite(ROUTER_POWER_DIS_PIN, LOW); \
  digitalWrite(REBOOT_NOTIFICATION_LED, HIGH); // enable router and disable notification LED
#define DISABLE_ROUTER_POWER                \
  digitalWrite(ROUTER_POWER_DIS_PIN, HIGH); \
  digitalWrite(REBOOT_NOTIFICATION_LED, LOW); // disable router and enable notification LED

// Values 0 or above are allowed for counters, so when you would like to disable counter set it with this value
// Supported only by some counters
#define COUNTER_DISABLED -1
#define WDT_TIMEOUT REBOOT_POWER_DOWN_TIMEOUT + 3 * TEST_CONNECTION_LOOP_TIMEOUT // Set the watchdog timeout to X seconds max possible is 2 loops and restart

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

int ping_failed_count = 0;
int wifi_failed_count = 0;
int cycles_after_reboot_count = COUNTER_DISABLED; // if COUNTER_DISABLED then reboot havent been done or notification have been already sent

void setup()
{
  Serial.begin(115200);

  SETUP_ROUTER_POWER_PINOUTS;
  ENABLE_ROUTER_POWER;

  esp_task_wdt_init(WDT_TIMEOUT, true); // Initialize the watchdog timer with a timeout and enable panic so ESP32 restarts
  esp_task_wdt_add(NULL);               // Add the current thread to the watchdog timer watch

  // Połącz z siecią WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi connecting ...");
    delay(2000); // Wait 2 seconds
  }
  Serial.println("WiFi connected!");
}

void perform_router_reboot()
{
  ping_failed_count = 0;
  wifi_failed_count = 0;
  cycles_after_reboot_count = 0; // enable counter for notifications

  DISABLE_ROUTER_POWER;
  Serial.println("Router power OFF");
  delay(REBOOT_POWER_DOWN_TIMEOUT * 1000); // router turned off for X seconds
  Serial.println("Router power ON");
  ENABLE_ROUTER_POWER;
}

void notify_power_off_on_cycle()
{
  String messageContent = "Router rebooted ";
  if (0 == cycles_after_reboot_count)
  {
    messageContent += "right now";
  }
  else
  {
    messageContent += String(cycles_after_reboot_count * TEST_CONNECTION_LOOP_TIMEOUT) + " seconds before";
  }

#ifdef DISCORD_WEBHOOK
  // Send a message to discord
  HTTPClient http;
  http.begin(DISCORD_WEBHOOK); // Specify your Discord webhook URL
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST("{\"content\":\"" + messageContent + "\"}"); // Send the actual POST request
  if (httpResponseCode > 0)
  {
    if (
        (httpResponseCode >= 100 && httpResponseCode <= 103) || // 100, 101, 102, 103,
        (httpResponseCode >= 204 && httpResponseCode <= 205)    // 204, 205,
    )
    {                                                                                // No content codeS
      Serial.println("Message sent to discord. Message content: " + messageContent); // Print return code
    }
    else
    {
      String response = http.getString(); // Get the response to the request
      Serial.println(httpResponseCode);   // Print return code
      Serial.println(response);           // Print request answer
    }
  }
  else
  {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  }
#endif

  cycles_after_reboot_count = COUNTER_DISABLED; // disable counter, it will be enabled
}

void loop()
{
  esp_task_wdt_reset();

  // Check if connection with google.com is available
  if (WiFi.status() == WL_CONNECTED)
  {
    if (wifi_failed_count > 0)
    {
      Serial.println("WiFi connected after " + String(wifi_failed_count) + " unsuccessful connections");
      wifi_failed_count = 0;
      ping_failed_count = 0; // reset also ping counter, so it could be counted from the begging
    }

    WiFiClient client;
    if (client.connect("google.com", 80))
    {
      ping_failed_count = 0;
      Serial.print("+");
      if (COUNTER_DISABLED != cycles_after_reboot_count)
      {
        // notify only when internet access is possible
        notify_power_off_on_cycle();
      }
    }
    else
    {
      if (0 == ping_failed_count)
      {
        Serial.println("");
      }
      Serial.println("Couldn't connect to google.com");
      ping_failed_count++;
    }
    client.stop();
  }
  else
  {
    wifi_failed_count++;
    Serial.println("Couldn't connect to WiFi");
  }

  if (COUNTER_DISABLED != cycles_after_reboot_count)
  {
    cycles_after_reboot_count++;
  }

  if (ping_failed_count > REBOOT_AFTER_X_CONNECTION_FAILS)
  {
    perform_router_reboot();
  }

  delay(TEST_CONNECTION_LOOP_TIMEOUT * 1000); // Check every X seconds
}
