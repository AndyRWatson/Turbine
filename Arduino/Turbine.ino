#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiClient.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <NTPClient.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


const IPAddress apIP(192, 168, 4, 1);
const char* apSSID = "Turbine";
const char* apPWD =  "LetMe1n2";
const char* appName = "Turbine";
boolean settingMode;
boolean ledOn=false;
boolean timeSet=false;
String ssidList;
String ipAddress;
const long utcOffsetInSeconds = 3600;  // One Hour
const long  gmtOffset_sec = 3600;      // One Hour
static int hour;
static int minute;
static int second;
static int turbine_speed=0;
int time_period = 60000;  // 10 minutes
unsigned long time_now=0;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


// Define NTP Client to get time
WiFiUDP ntpUDP;
//NTPClient tc(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
NTPClient tc(ntpUDP, "pool.ntp.org", 0);

DNSServer dnsServer;
ESP8266WebServer webServer(80);

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(D5,OUTPUT);
  pinMode(D6,OUTPUT);
  
  // Initialising the UI will init the display too.
   if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
   }
  setOLED();
  time_now = millis();
  if (restoreConfig()) {
    if (checkConnection()) {
      settingMode = false;
      startWebServer();
      return;
    }
  }
  settingMode = true;
  setupMode();
}

void loop() {
    
  if (settingMode) {
    // Config
    if(ledOn==false)
    {
      Serial.println("LED_BUILTIN,LOW");
      digitalWrite(LED_BUILTIN, LOW);
      ledOn=true;
    }
    dnsServer.processNextRequest(); 
  }
  webServer.handleClient();
}

void setOLED()
{
    // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.clearDisplay();
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.println("https://codewind.dev");
  display.setCursor(0, 12);     // Start at top-left corner
  if (settingMode)
  {
    display.println(WiFi.softAPIP());
  }
  else
  {
    display.println(WiFi.localIP().toString());
  }
  display.setCursor(0, 24);     
  display.print("setTurbine(");
  display.print(turbine_speed);
  display.println(")");
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.display();
}

void getTime()
{
  tc.update();
  hour=tc.getHours();
  minute=tc.getMinutes();
  second=tc.getSeconds();
  Serial.print(daysOfTheWeek[tc.getDay()]);
  Serial.print(", ");
  Serial.println(tc.getFormattedTime());
  timeSet=true;
  time_now=millis();
  Serial.print("millis() =");
  Serial.println(time_now);
}


boolean restoreConfig() {
  Serial.println("Reading EEPROM...");
  String ssid = "";
  String pass = "";
  if (EEPROM.read(0) != 0) {
    for (int i = 0; i < 32; ++i) {
      ssid += char(EEPROM.read(i));
    }
    Serial.print("SSID: ");
    Serial.println(ssid);
    for (int i = 32; i < 96; ++i) {
      pass += char(EEPROM.read(i));
    }
    Serial.print("Password: ");
    Serial.println(pass);
    WiFi.begin(ssid.c_str(), pass.c_str());
    return true;
  }
  else {
    Serial.println("Config not found.");
    return false;
  }
}

boolean checkConnection() {
  int count = 0;
  Serial.print("Waiting for Wi-Fi connection");
  while ( count < 30 ) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.print(".");
    if (WiFi.status() == WL_CONNECTED) {
      ipAddress=WiFi.localIP().toString();
      Serial.println();
      Serial.print("Connected! ");
      Serial.println(ipAddress);
      ledOn=false;
      getTime();
      setOLED();
      return (true);
    }
    count++;
  }
  Serial.println("Timed out.");
  return false;
}

void startWebServer() {
  if (settingMode) {
    Serial.print("Starting Web Server at ");
    Serial.println(WiFi.softAPIP());
    webServer.on("/settings", []() {
      String s = "<h1>Wi-Fi Settings</h1><p>Please enter your password by selecting the SSID.</p>";
      s += "<form method=\"get\" action=\"setap\"><label>SSID: </label><select name=\"ssid\">";
      s += ssidList;
      s += "</select><br>Password: <input name=\"pass\" length=64 type=\"password\"><input type=\"submit\"></form>";
      webServer.send(200, "text/html", makePage("Wi-Fi Settings", s));
    });
    webServer.on("/setap", []() {
      for (int i = 0; i < 96; ++i) {
        EEPROM.write(i, 0);
      }
      String ssid = urlDecode(webServer.arg("ssid"));
      Serial.print("SSID: ");
      Serial.println(ssid);
      String pass = urlDecode(webServer.arg("pass"));
      Serial.print("Password: ");
      Serial.println(pass);
      Serial.println("Writing SSID to EEPROM...");
      for (int i = 0; i < ssid.length(); ++i) {
        EEPROM.write(i, ssid[i]);
      }
      Serial.println("Writing Password to EEPROM...");
      for (int i = 0; i < pass.length(); ++i) {
        EEPROM.write(32 + i, pass[i]);
      }
      EEPROM.commit();
      Serial.println("Write EEPROM done!");
      String s = "<h1>Setup complete.</h1><p>device will be connected to \"";
      s += ssid;
      s += "\" after the restart.";
      webServer.send(200, "text/html", makePage("Wi-Fi Settings", s));
      ESP.restart();
    });
    webServer.onNotFound([]() {
      String s = "<h1>";
      s+=appName;
      s+=" (AP mode )</h1><p><a href=\"/settings\">Wi-Fi Settings</a></p>";
      webServer.send(200, "text/html", makePage("AP mode", s));
    });
  }
  else {
    Serial.print("Starting Web Server at ");
    Serial.println(WiFi.localIP());
    webServer.on("/", []() {
      String s = "<h1>";
      s+=appName;
      s+="   ";
      s+=WiFi.localIP().toString();
      s+=" </h1><p><a href=\"/reset\">Reset Wi-Fi Settings</a></p><p><a href=\"/speed\"> Speed</a></p><p><a href=\"/status\"> Status</a></p>";
      webServer.send(200, "text/html", makePage("STA mode", s));
    });

   webServer.on("/status", []() {
      String s = "<h1>";
      s+= appName;
      s+="   ";
      s+=WiFi.localIP().toString();      
      s+=" Status</h1><p>OK</p>";
      s += "</p>";
      webServer.send(200, "text/html", makePage("STA mode", s));
    });

    webServer.on("/speed", []() {
      String s = "<h1>Turbine Speed</h1>";
      s +="<body> <p>Here's the list of speeds. Select any one:</p>";
      s +="<form method=\"get\" action=\"/setTurbine\">";
      s +="<select name = \"speed\">";
      for (int i = 0; i <=100; i+=10) {
        s += "<option value=\"";
        s += i;
        s += "\">";
        s += i;
        s += "</option>";
      }
      s += "<input type=\"submit\" value=\"Submit\">";
      s += "</select></form></body>";
      webServer.send(200, "text/html", makePage("STA mode", s));
    });
    
    webServer.on("/health", []() {
      String s = "<p>OK</p>";
      webServer.send(200, "text/html", makePage("STA mode", s));
    });   
    webServer.on("/setTurbine", []() {
      // speed=0 to 100;
      String speed = urlDecode(webServer.arg("speed"));
      String weather = urlDecode(webServer.arg("weather"));

      String s = "<h1> setTurbineSpeed</h1><p>speed =";
      s += speed;
      s += " </p>";
      webServer.send(200, "text/html", makePage("STA mode", s));
      Serial.print("speed=");
      Serial.println(speed);
      Serial.print("weather=");
      Serial.println(weather);
      setTurbineSpeed(speed.toInt());
    });   

    webServer.on("/reset", []() {
      for (int i = 0; i < 96; ++i) {
        EEPROM.write(i, 0);
      }
      EEPROM.commit();
      String s = "<h1>Wi-Fi settings was reset.</h1><p>Please reset device.</p>";
      webServer.send(200, "text/html", makePage("Reset Wi-Fi Settings", s));
    });
  }
  webServer.begin();
}


void setupMode() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  delay(100);
  Serial.println("");
  for (int i = 0; i < n; ++i) {
    ssidList += "<option value=\"";
    ssidList += WiFi.SSID(i);
    ssidList += "\">";
    ssidList += WiFi.SSID(i);
    ssidList += "</option>";
  }
  delay(100);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apSSID,apPWD);
  dnsServer.start(53, "*", apIP);
  startWebServer();
  Serial.print("Starting Access Point at \"");
  Serial.print(apSSID);
  Serial.println("\"");
}

void setTurbineSpeed(int speed)
{
  if(speed >100)
  {
    speed=100;
  }  
  turbine_speed=speed;
  
  speed=speed*10;
  analogWrite(D5,700);  // Motor may need extra push to start
  delay(500);
  analogWrite(D5,speed);
  digitalWrite(D6,LOW);
  Serial.print("Set Turbine Speed :");
  Serial.println(speed);
  setOLED();
}

String makePage(String title, String contents) {
  String s = "<!DOCTYPE html><html><head>";
  s += "<meta name=\"viewport\" content=\"width=device-width,user-scalable=0\">";
  s += "<title>";
  s += title;
  s += "</title></head><body>";
  s += contents;
  s += "</body></html>";
  return s;
}


String urlDecode(String input) {
  String s = input;
  s.replace("%20", " ");
  s.replace("+", " ");
  s.replace("%21", "!");
  s.replace("%22", "\"");
  s.replace("%23", "#");
  s.replace("%24", "$");
  s.replace("%25", "%");
  s.replace("%26", "&");
  s.replace("%27", "\'");
  s.replace("%28", "(");
  s.replace("%29", ")");
  s.replace("%30", "*");
  s.replace("%31", "+");
  s.replace("%2C", ",");
  s.replace("%2E", ".");
  s.replace("%2F", "/");
  s.replace("%2C", ",");
  s.replace("%3A", ":");
  s.replace("%3A", ";");
  s.replace("%3C", "<");
  s.replace("%3D", "=");
  s.replace("%3E", ">");
  s.replace("%3F", "?");
  s.replace("%40", "@");
  s.replace("%5B", "[");
  s.replace("%5C", "\\");
  s.replace("%5D", "]");
  s.replace("%5E", "^");
  s.replace("%5F", "-");
  s.replace("%60", "`");
  return s;
}
