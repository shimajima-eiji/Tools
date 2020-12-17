// Refer thanks: https://lang-ship.com/blog/work/m5stickc-resetter/

#include <M5StickC.h>
#include <Preferences.h>
#include <WiFi.h>
#include "time.h"

// 設定値
const char *ssid = "your Wifi(2.4G)";
const char *password = "your Wifi pass";
const char *ntpServer = "ntp.jst.mfeed.ad.jp";
const int time_difference = 9 * 3600; // Tokyo

// 内部変数
Preferences preferences;
bool ntp_setup = false;
RTC_TimeTypeDef RTC_TimeStruct;
RTC_DateTypeDef RTC_DateStruct;

void Wifi_connect()  // Refer thanks: https://msr-r.net/m5stickc-wifi-error/
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

void setup()
{
    M5.begin();
    M5.Lcd.setRotation(3);
    M5.Lcd.fillScreen(BLACK);

    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("M5StickC Resetter");

    // NVS領域初期化
    if (preferences.clear())
    {
        Serial.println("NVS clear");
    }

    // RTC状態リセット
    Wire1.beginTransmission(0x51);
    Wire1.write(0x00);
    Wire1.write(0x00);
    Wire1.write(0x00);
    Wire1.endTransmission();
    Serial.println("RTC status reset");

    // connect to WiFi
    Serial.printf("Connecting to %s ", ssid);
    Wifi_connect();

    // Set ntp time to local
    configTime(time_difference, 0, ntpServer);
}

void loop()
{
    M5.Lcd.setCursor(0, 8 * 2);

    // RTC NTP set
    struct tm timeInfo;
    if (!ntp_setup && WiFi.status() == WL_CONNECTED && getLocalTime(&timeInfo))
    {
        ntp_setup = true;

        // Set RTC time
        RTC_TimeTypeDef TimeStruct;
        TimeStruct.Hours = timeInfo.tm_hour;
        TimeStruct.Minutes = timeInfo.tm_min;
        TimeStruct.Seconds = timeInfo.tm_sec;
        M5.Rtc.SetTime(&TimeStruct);

        RTC_DateTypeDef DateStruct;
        DateStruct.WeekDay = timeInfo.tm_wday;
        DateStruct.Month = timeInfo.tm_mon + 1;
        DateStruct.Date = timeInfo.tm_mday;
        DateStruct.Year = timeInfo.tm_year + 1900;
        M5.Rtc.SetData(&DateStruct);
    }

    // Wi-fi
    byte macAddress[6];
    WiFi.macAddress(macAddress);
    M5.Lcd.printf("SSID: %s\n", ssid);
    M5.Lcd.print("IP  : ");
    M5.Lcd.println(WiFi.localIP());
    M5.Lcd.printf("MAC : %02X:%02X:%02X:%02X:%02X:%02X\n", macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]);
    M5.Lcd.println("");

    // バッテリー
    float vbat = M5.Axp.GetBatVoltage();
    int vlevel = (vbat - 3.2) / 0.8 * 100;
    if (vlevel < 0)
    {
        vlevel = 0;
    }
    if (100 < vlevel)
    {
        vlevel = 100;
    }
    M5.Lcd.printf("BAT : %5.3fV %3d%%\n", M5.Axp.GetBatVoltage(), vlevel);
    M5.Lcd.println("");

    // RTC
    static const char *wd[7] = {"Sun", "Mon", "Tue", "Wed", "Thr", "Fri", "Sat"};
    M5.Rtc.GetTime(&RTC_TimeStruct);
    M5.Rtc.GetData(&RTC_DateStruct);
    M5.Lcd.printf("DATE: %04d-%02d-%02d(%s)\n", RTC_DateStruct.Year, RTC_DateStruct.Month, RTC_DateStruct.Date, wd[RTC_DateStruct.WeekDay]);
    M5.Lcd.printf("TIME: %02d:%02d:%02d", RTC_TimeStruct.Hours, RTC_TimeStruct.Minutes, RTC_TimeStruct.Seconds);
    if (ntp_setup)
    {
        M5.Lcd.println(" (NTP)");
    }
    else
    {
        M5.Lcd.println("");
    }

    delay(1000);
}
