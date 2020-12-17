/*
# 使い方
育児中の家庭を想定
リモートワークで働いていると、子供のお世話が結局ワンオペになるので気軽に呼べる機能をM5StickCを使って作成
トラブル発生とご飯の時ぐらいしか呼ばれないので、とりあえず呼ばれれば応答できる仕組みを整える。



事前に
- M5StickCにビルドできる環境
- LINE通知を有効にする設定
- LINE Notifyのトークン
が必要

# 
*/

#include <ssl_client.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include "M5StickC.h"
#include "M5Display.h"

const char *ssid = "Your SSID(2.4G)";
const char *password = "Your SSID pass";
const char *token = "Your LINE Notify Token";  // https://notify-bot.line.me/my/

// [M5]ボタンの表示設定
const char *btna_line_message = "LINEに送るメッセージ。日本語可。サンプルでは「ちょっと来て」";
const char *btna_display_message = "送信状況をM5stickで表示。英語のみ。「help」";

// サイドボタンの表示設定
const char *btnb_line_message = "LINEに送るメッセージ。日本語可。サンプルでは「ご飯できたよ」。要らなさそう";
const char *btnb_display_message = "送信状況をM5stickで表示。英語のみ。「gohan」";

void view(String message)
{
    Serial.println(message);
    M5.Lcd.println(message);
}

void Wifi_connect()
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

void send(String message)
{
    const char *host = "notify-api.line.me";
    WiFiClientSecure client;
    Serial.println("Try");
    //LineのAPIサーバに接続
    if (!client.connect(host, 443))
    {
        Serial.println("Connection failed");
        return;
    }
    Serial.println("Connected");
    //リクエストを送信
    String query = String("message=") + message;
    String request = String("") +
                     "POST /api/notify HTTP/1.1\r\n" +
                     "Host: " + host + "\r\n" +
                     "Authorization: Bearer " + token + "\r\n" +
                     "Content-Length: " + String(query.length()) + "\r\n" +
                     "Content-Type: application/x-www-form-urlencoded\r\n\r\n" +
                     query + "\r\n";
    client.print(request);

    //受信終了まで待つ
    while (client.connected())
    {
        String line = client.readStringUntil('\n');
        Serial.println(line);
        if (line == "\r")
        {
            break;
        }
    }

    String line = client.readStringUntil('\n');
    Serial.println(line);
}

void clean()
{
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("push [M5] button: call\nright side button: gohan(eatting)");
}

void reset(String message, String display)
{
    clean();
    M5.Lcd.println("sending " + display + " message...");
    send(message);
    M5.Lcd.println("OK");
    delay(5000);
    clean();
}

void setup()
{
    M5.begin();
    M5.Axp.ScreenBreath(9);
    setCpuFrequencyMhz(80);
    M5.Lcd.setRotation(3);
    M5.Lcd.fillScreen(BLACK);
    Wifi_connect();
    M5.Lcd.setTextSize(2);
    clean();
}

void loop()
{
    M5.update();
    M5.Lcd.setCursor(0, 40);
    if (M5.BtnA.wasPressed())
    {
        reset(btna_line_message, btna_display_message);
    }
    else if (M5.BtnB.wasPressed())
    {
        reset(btnb_line_message, btnb_display_message);
    }
}
