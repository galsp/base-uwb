
#include <Arduino.h>
#include <HardwareSerial.h>

#define WIFI_USE
#include <Hendi-Multi-IoT.h>

unsigned long daq_check;
unsigned long daq_check_interval = 10 * 1000;
void switchRelay(bool _the_state);
int getTemperature();
void sendData();
void deviceRelaySwitch(DynamicJsonDocument source_doc);
void devicePing(DynamicJsonDocument source_doc);
void deviceRandomFunction(DynamicJsonDocument source_doc);
void autoRelay();
void deviceSwitchConnection(DynamicJsonDocument source_doc);

#define WIFI_USE
byte hexPrinton[17] = {0x01, 0x80, 0xFF, 0xB8, 0x02, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x23, 0xC7, 0x76};
byte hexPrintoff[11] = {0x01, 0x80, 0xFF, 0xB9, 0x02, 0xFF, 0x00, 0xFF, 0x23, 0x53, 0x5B};
byte hexrndc[11] = {0x01, 0x80, 0xFF, 0xC2, 0x02, 0xFF, 0x00, 0xFF, 0x23, 0x59, 0x10};

int statusRufer = 0;
//////////////

void uwbRead();

int Xvalue;
int Yvalue;
int Xvalue2;
int Yvalue2;
int Xvalue3;
int Yvalue3;

int hexToDec(String hexString);

long lastMilis;
long lastMilis2;

int Nilai[500];
int nilaike = 0;

String NilaiString;
String hexString;

int load;

void printNilai();
//////////////

HardwareSerial SerialUwb(2);
void setup()
{
  Serial.begin(115200);
  SerialUwb.begin(115200, SERIAL_8N1, 16, 17);
  // SerialUwb.write(hexrndc, sizeof(hexrndc));
  // SerialUwb.write(hexPrinton, sizeof(hexPrinton));
  Serial.print("booting");
  while (SerialUwb.availableForWrite() == false)
  {
    delay(200);
    Serial.print(".");
  }

  /////////////////

  if (true)
  { // read EEPROM for node code, register mark, wifi ssid and wifi password
    setEEPROM();
    readEEPROMProfile();
  }

  if (true)
  {
    setButton();
    setWifiProfile();
    enableFreeWifi(); // connect free wifi if main wifi unavailable
    // setSecureFunction(autoRelay);                  // function runned while reconnect internet
  }

  if (true)
  {
  }

  if (register_mark != true)
  {
    apnMode();
  }

  if (true)
  {                              // add topic
    addInTopic("flux/command/"); // automatically converted to "topic/nodecode"
    addOutTopic("flux/general_json/");
    addFeedBackTopic("flux/feedback/"); // automatically converted to "topic/nodecode"
    setPingFunction(sendData);
    setOTA();
  }

  if (true)
  {
    setCommandCode(0, deviceRelaySwitch); // callback with EEPROM
    setCommandCode(1, devicePing);        // cmd 2-5 already preserved
    setCommandCode(4, deviceSwitchConnection);
    setCommandCode(6, deviceUnregisterNew);
    setCommandCode(7, deviceRandomFunction);
  }

  while (net_mark == false)
  { // connect to wifi and mqtt
    reconnectAttempt();
    SerialUwb.write(hexPrinton, sizeof(hexPrinton));
  }
}
void loop()
{  
  ptr_MQTT->loop();

  connectivityEstablishment();                    // include connectivity checking for every 10 seconds and auto reconnect for all type connection and mqtt

  if (isFirst){                                     // send data every 10 seconds
    if (millis() - daq_check > daq_check_interval) {
      sendData();
      daq_check = millis();
    }
  }

  while (statusRufer >= 2)
  {
    uwbRead();
    SerialUwb.write(hexPrinton, sizeof(hexPrinton));
    delay(300);
    Serial.print(".");
  }

  if (millis() - lastMilis > 1000)
  {
    lastMilis = millis();
    uwbRead();
    printNilai();
  }
}
void uwbRead()
{
  statusRufer = 0;

  for (int o = 0; o < 3;)
  {

    o++;
    if (SerialUwb.available())
    {
      // Membaca sejumlah byte dari Serial
      byte buffer[64]; // Buffer untuk menyimpan data
      int bytesRead = SerialUwb.readBytes(buffer, sizeof(buffer));

      // Mengonversi setiap byte dalam buffer menjadi hexadecimal dan mencetaknya
      for (int i = 0; i < bytesRead; i++)
      {
        String hexString = String(buffer[i], HEX);
        if (hexString.length() < 2)
        {
          hexString = "0" + hexString; // Menambahkan 0 di depan jika perlu
        }

        if (hexString == "01" || load >= 1)
        {
          if (hexString == "83" || load >= 2)
          {
            load = 2;
            // Serial.print(hexString);
            // Serial.print(" ");
            // Serial.print(hexToDec(hexString));
            // Serial.print(" ");
            nilaike++;
            Nilai[nilaike] = hexToDec(hexString);
          }
          else
          {
            load = 1;
          }
        }
      }
    }
    else
    {
      statusRufer++;
    }

    Xvalue = Nilai[10];
    Yvalue = Nilai[12];
    Xvalue2 = Nilai[24];
    Yvalue2 = Nilai[26];
    Xvalue3 = Nilai[38];
    Yvalue3 = Nilai[40];
    // if (Nilai[10] > 0 && Nilai[12] > 0)
    // {
    // }
  }
  load = 0;
  nilaike = 0;
}
int hexToDec(String hexString)
{
  int decimalValue = 0;

  // Loop melalui setiap karakter dalam string hexadecimal
  for (int i = 0; i < hexString.length(); i++)
  {
    char hexChar = hexString.charAt(i);
    int hexValue = 0;

    // Mengkonversi karakter hexadecimal ke nilai integer
    if (hexChar >= '0' && hexChar <= '9')
    {
      hexValue = hexChar - '0';
    }
    else if (hexChar >= 'A' && hexChar <= 'F')
    {
      hexValue = 10 + (hexChar - 'A');
    }
    else if (hexChar >= 'a' && hexChar <= 'f')
    {
      hexValue = 10 + (hexChar - 'a');
    }

    // Menghitung nilai decimal
    decimalValue = (decimalValue * 16) + hexValue;
  }

  return decimalValue;
}
void printNilai()
{

  Serial.println();
  Serial.println("Forklift 1 :");
  Serial.print("X Axis ");
  Serial.println(Nilai[10]);
  Serial.print("Y Axis ");
  Serial.println(Nilai[12]);
  Serial.println();
  Serial.println();
  Serial.println("Forklift 2:");
  Serial.print("X Axis ");
  Serial.println(Nilai[24]);
  Serial.print("Y Axis ");
  Serial.println(Nilai[26]);
  Serial.println();
  Serial.println();
  Serial.println("Forklift 3 :");
  Serial.print("X Axis ");
  Serial.println(Nilai[38]);
  Serial.print("Y Axis ");
  Serial.println(Nilai[40]);
  Serial.println();
}
///////////////
void autoRelay()
{
}
void sendData()
{
  unsigned int _data_length = 300;
  DynamicJsonDocument _doc(_data_length);
  String _txt;
  _doc["nodeCode"] = node_code;
  _doc["time"] = pTime();
  _doc["0"] = getCurrentRSSI();
  _doc["1"] = Xvalue;
  _doc["2"] = Yvalue;
  _doc["3"] = Xvalue2;
  _doc["4"] = Yvalue2;
  _doc["5"] = Xvalue3;
  _doc["6"] = Yvalue3;
  serializeJsonPretty(_doc, _txt);
  ptr_MQTT->publish(out_topic, _txt.c_str());
  Serial.println(_txt);
  delay(1000);
  // Serial.println("Send Data");
}
void controlRelay(int _valOrder, int _condition)
{

  delay(1000);
}
void deviceRelaySwitch(DynamicJsonDocument source_doc)
{
  Serial.println("Income command");
  int _cmd_code = source_doc["cmdCode"];
  int _the_order = source_doc["valOrder"];
  int _the_value = source_doc["value"];
  String _uid = source_doc["uid"];
  int _relay_start_parameter_order = 1; // lowest is 0
  // controlRelay(_the_order - _relay_start_parameter_order,_the_value);
  DynamicJsonDocument _doc(200);
  String _txt;
  _doc["cmdCode"] = _cmd_code;
  _doc["status"] = "true";
  _doc["uid"] = _uid;
  serializeJsonPretty(_doc, _txt);
  ptr_MQTT->publish(feed_back_topic, _txt.c_str());
  sendData();
}
void deviceSwitchConnection(DynamicJsonDocument source_doc)
{
  int _connection = source_doc["connection"];

  if (_connection == CONNECT2LAN)
  {
    if (connectivity_mark != CONNECT2LAN)
    {
      wifi_mark = false;
      net_mark = false;
      free_mark = false;
      mqtt_mark = false;
      pConnectLAN();
    }
  }
  else if (_connection == CONNECT2WIFI)
  {
    if (connectivity_mark != CONNECT2WIFI)
    {
      wifi_mark = false;
      net_mark = false;
      free_mark = false;
      mqtt_mark = false;
      pConnectWifi(local_wifi_ssid, local_wifi_pass);
    }
  }
  else if (_connection == CONNECT2GSM)
  {
    if (connectivity_mark != CONNECT2GSM)
    {
      wifi_mark = false;
      net_mark = false;
      free_mark = false;
      mqtt_mark = false;
      pReconnectGPRS();
    }
  }
  else
  {
    if (connectivity_mark != CONNECT2WIFI)
    {
      wifi_mark = false;
      net_mark = false;
      free_mark = false;
      mqtt_mark = false;
      pFreeWifi();
    }
  }
  pSecureFunction();
  if (net_mark == true)
  {
    if (mqtt_mark == false)
    {
      connectMQTT();
    }
  }
}
void devicePing(DynamicJsonDocument source_doc)
{
  sendData();
}
void deviceRandomFunction(DynamicJsonDocument source_doc)
{
}