#include <SD.h>
#include <Wire.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>

#include "xbee.h"
#include "xbee_print.h"

// Pins used:

// Digital

// 2  - XBee DIN
#define XBEE_DIN 2
// 3  - XBee DOUT
#define XBEE_DOUT 3


#define WIFI_SSID "ssid"
#define WIFI_PASSWORD "password"

class CallbacksExample
{
public:
  CallbacksExample(XbeeService* xbee)
    : _xbee(xbee)
  {
  }

  void setup()
  {
    // Setup callbacks.
    
    _xbee->setAtCallback(closure_bind(this, &CallbacksExample::processStartupAtCommand));
    _xbee->setRequestCallback(closure_bind(this, &CallbacksExample::processIpRequest));
    
    // Get current time from NTP server.
    _xbee->requestCurrentTime();
  }

private:
  // Processes HTTP requests. 
  void processIpRequest(IpAddress* address, HttpService::RequestType type, uint8_t* location, int locationLength, uint8_t* data, int dataLength)
  {   
    // Process time responses. 
    if (_xbee->isTimeResponse(-4, address, type, data, dataLength, _hours, _minutes, _seconds))
    {
      Serial.print("Time is ");
      Serial.print(_hours);
      Serial.print(":");
      Serial.print(_minutes);
      Serial.print(":");
      Serial.println(_seconds);
    }
    else
    {    
      // Check type of HTTP request.
      if (type == HttpService::request_post)
      {
        // Return something back.
        _xbee->ipSend(address, XbeeService::http_found_non_cached, (uint8_t*)"POST not supported");        
      }
      // Process get requests.
      if (type == HttpService::request_get) 
      {
        // Get current time.
        if (locationLength >= 5 && memcmp(location, "/time", 5) == 0)
        {
          // Use printing class.
          XbeePrint r(_xbee, address, XbeeService::http_found_non_cached);
          r.print(_hours);
          r.print(":");
          r.println(_minutes);
          // Packet is sent when XbeePrint instance is destroyed.      
        }
        else
        {
          // Reply something for all other requests, otherwise requestor will hang until TCP timeout.       
          _xbee->ipSend(address, XbeeService::http_found_non_cached, (uint8_t*)"OK");
        }
      }
    }
  }
 
  
  // Processes async AT commands.
  void processStartupAtCommand(uint16_t atCommand, uint8_t frame, uint8_t status, uint8_t* data, int dataLength)
  {    
  }

private:
  uint8_t _hours;
  uint8_t _minutes;
  uint8_t _seconds;

  XbeeService* _xbee;
};

XbeeService _xbee(XBEE_DIN, XBEE_DOUT);

CallbacksExample _example(&_xbee);

void printHex(const uint8_t* data, uint8_t count)
{
  for (; count != 0; --count, ++data)
    Serial.print(*data, HEX);
}

void printChar(const uint8_t* data, uint8_t count)
{
  for (; count != 0; --count, ++data)
    Serial.print((char)*data);
}

void setup()
{
  Serial.begin(9600);

  pinMode(13, OUTPUT);
  
  _xbee.setup();

  // Reset xbee just in case, for consistency.
  if (_xbee.atCommandSync("FR"))
  {
    Serial.println("Reset...");
    delay(4000);
  }    
  
  // Set wifi network SSID.
  if (!_xbee.atCommandSync(WIFI_SSID))
    Serial.println("Error settings SSID");
  // Set wifi password.
  if (!_xbee.atCommandSync(WIFI_PASSWORD))
    Serial.println("Error setting password");    
  // Apply changes.
  if (!_xbee.atCommandSync("AC"))
    Serial.println("Error");
    
  // Get software version.
  if (_xbee.atCommandSync("VR"))
  {
    Serial.print("SV: ");    
    printHex(_xbee.atBufferSync(), _xbee.atBufferCountSync());
    Serial.println();
  }    
  
  // Wait till XBee joined network.
  for (;;) 
  {
    if (_xbee.atCommandSync("AI"))
    {         
      if (_xbee.atBufferCountSync() > 0 && _xbee.atBufferSync()[0] == 0x0)
      {
        break;
      }
    }      
    // Still in process of connecting to AP, request status.
    delay(250);
  }
  
  // Get current IP address.
  if (_xbee.atCommandSync("MY"))
  {
    Serial.print("IP: ");
    printChar(_xbee.atBufferSync(), _xbee.atBufferCountSync());
    Serial.println();
  }
  
  // Get current listening port.
  if (_xbee.atCommandSync("C0"))
  {
    Serial.print("Port: ");
    Serial.println((_xbee.atBufferSync()[0] << 8) +_xbee.atBufferSync()[1]);
  }  
  
  // Initialize our example app.
  _example.setup();
}

void loop()
{
  unsigned long mills = millis();

  // XBee update method must be called from time to time.
  _xbee.update(mills);
}


