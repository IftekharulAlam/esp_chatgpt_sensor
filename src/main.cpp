#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char *ssid = "dhong";                                                                                                                                                                  // Replace with your Wi-Fi SSID
const char *password = "01521439838";                                                                                                                                                        // Replace with your Wi-Fi password
const char *apiUrl = "https://api.openai.com/v1/chat/completions";                                                                                                                           // OpenAI API endpoint
const char *apiKey = "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"; // Replace with your OpenAI API key

AsyncWebServer server(80);

String conversation = "";

bool inputValuesSet = false;
String customWarning = "";

float displaySpeed;
void notFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}
String processResponse(String response)
{
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, response);

    if (error)
    {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
    }
    // Extract the chat response from the JSON
    const char *chatResponse = doc["choices"][0]["message"]["content"];
    Serial.println("ChatGPT: " + String(chatResponse));

    return String(chatResponse);

    // String my = String(chatResponse);
    // display.setCursor(0, 0);
    // display.print(my);
    // display.display();

    // Display the ChatGPT response on the OLED
    // displayText(String("ChatGPT: " + String(chatResponse)));
}
String sendToChatGPT(String message)
{
    String responseOFChatgptProcessed = "";
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;
        http.begin(apiUrl);
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Authorization", "Bearer " + String(apiKey));

        // Create the JSON payload for the OpenAI API
        String payload = "{\"model\": \"gpt-3.5-turbo\", \"messages\": [{\"role\": \"user\", \"content\": \"" + message + "\"}]}";

        // Send the POST request
        int httpResponseCode = http.POST(payload);

        if (httpResponseCode > 0)
        {
            String response = http.getString();

            Serial.println("ChatGPT Response:");
            responseOFChatgptProcessed = processResponse(response);
            return responseOFChatgptProcessed;
        }
        else
        {
            Serial.println("Error on HTTP request: " + String(httpResponseCode));
            responseOFChatgptProcessed = httpResponseCode;
            return responseOFChatgptProcessed;
        }

        http.end();
    }
    else
    {
        Serial.println("WiFi not connected!");
        responseOFChatgptProcessed = "WiFi not connected!";
        return responseOFChatgptProcessed;
    }
}
String processor(const String &var)
{
    if (var == "CONVERSATION")
    {
        return conversation;
    }
    return String();
}

void setup()
{
    Serial.begin(115200);

    // Configure the PulseOximeter
    Serial.println("Connecting to ");
    Serial.println(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
        Serial.printf("WiFi Failed!\n");
        return;
    }

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Initialize SPIFFS
    if (!SPIFFS.begin(true))
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }
    Serial.println("SPIFFS mounted successfully");

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        Serial.println("Requesting index page...");
        request->send(SPIFFS, "/index.html", "text/html",false,processor ); });

    server.on("/css/custom.css", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/css/custom.css", "text/css"); });

    server.on("/js/custom.js", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/js/custom.js", "text/javascript"); });

    // Endpoint to get the current motor speed in RPM
    server.on("/get_rpm", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String rpmString = String(displaySpeed);
        request->send(200, "text/plain", rpmString); });
    server.on("/send", HTTP_POST, [](AsyncWebServerRequest *request)
              {
                Serial.println("Got it");
    if (request->hasParam("message", true)) {

    
      String userMessage = request->getParam("message", true)->value();
      conversation += "You: " + userMessage + "<br>";
      String botResponse = sendToChatGPT(userMessage);
      conversation += "ChatGPT: " + botResponse + "<br>";
      Serial.println(conversation);
    }
    request->send(200, "text/plain", "OK"); });

    server.on("/get_conversation", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "text/plain", conversation); });

    server.onNotFound(notFound);

    server.begin();
    Serial.println("Server started");
}

void loop()
{
}