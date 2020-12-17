#include <M5StickC.h>
#include <WiFi.h>

const char *ssid = "your Wifi(2.4G)";
const char *password = "your Wifi pass";

void setup()
{
    int cnt = 0;
    M5.Lcd.printf("Connecting to %s\n", ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        cnt++;
        delay(500);
        M5.Lcd.print(".");
        if (cnt % 10 == 0)
        {
            WiFi.disconnect();
            WiFi.begin(ssid, password);
            M5.Lcd.println("");
        }
        if (cnt >= 30)
        {
            ESP.restart();
        }
    }
    M5.Lcd.printf("\nWiFi connected\n");
}
