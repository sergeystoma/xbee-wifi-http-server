#ifndef _header_xbee_
#define _header_xbee_

#include "http.h"
#include "closure.h"

#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#include <SoftwareSerial.h>

#define AT_BUFFER_LENGTH 32

#define MAKE_COMMAND(x, y) ((((uint8_t)x) << 8) | (uint8_t)y)

class XbeeService
{
public:
  /// HTTP response type.
  enum HttpResponseType
  {
    /// Send data as-as.
    ip_raw,
    /// Don't frame response with HTTP headers.
    http_headerless,
    /// Content not found.
    http_not_found,
    /// Content found, can be cached forever (images, scripts, etc).
    http_found_cached,
    /// Content found, cannot be cached.
    http_found_non_cached
  };

  typedef void (XBeeAtCallback)(uint16_t atCommand, uint8_t frame, uint8_t status, uint8_t* data, int dataLength);

public:
  // 4800 baud seem to be fastest yet stable baud rate for my Arduino. 
  // Even at 9600 SoftwareSerial still gets an overflow, even on an empty read loop.
  XbeeService(int pin_in, int pin_out, int rate = 4800);

  /// Initializes XBee services.
  void setup();  

  void setAtCallback(Closure<XBeeAtCallback> callback);

  void setRequestCallback(Closure<HttpService::HttpRequestCallback> callback);  
  
  void listeningOnPort(uint16_t port);

  /// Sends AT command to XBee.
  void atCommand(uint8_t command[], uint8_t count);
  
  /// Sends AT command to XBee, must be null terminated.
  void atCommand(const char* command);

  /// Sends AT command to XBee.
  bool atCommandSync(uint8_t command[], uint8_t count);
  
  /// Sends AT command to XBee, must be null terminated.
  bool atCommandSync(const char* command);
  
  /// Gets current AT command response buffer.
  const uint8_t* atBufferSync() const;

  /// Gets a number of bytes currently in the AT command response buffer.
  uint8_t atBufferCountSync() const;
  
  /// Is XBee currently reading IP response/data frame?
  bool ipIsReading() const; 

  /// Sends HTTP response in a single chunk and closes the socket.
  void ipSend(IpAddress* address, HttpResponseType type, const uint8_t* parameterData = 0, size_t parameterLength = 0);

  /// Sends HTTP response headers, but keeps socket open so requestor could send extra data chunks.
  void ipBegin(IpAddress* address, HttpResponseType type, const uint8_t* parameterData = 0, size_t parameterLength = 0);

  /// Sends HTTP data chunk.
  void ipChunk(IpAddress* address, HttpResponseType type, const uint8_t* parameterData, size_t parameterLength = 0);

  /// Complete HTTP response.
  void ipEnd(IpAddress* address, HttpResponseType type);

  /// To be called periodically to process XBee API frames. 
  void update(int mill);  
  
  /// Requests current time from NTP server.  
  void requestCurrentTime();
  
  /// To be called from within request callback, checks if IP response has time information, and parses that timestamp.
  bool isTimeResponse(int8_t timeZoneOffset, IpAddress* address, HttpService::RequestType type, const uint8_t* parameterData, size_t parameterLength, uint8_t& hours, uint8_t& minutes, uint8_t& seconds);
  
private:
  void ipSendInternal(IpAddress* address, HttpResponseType type, const uint8_t* parameterData = 0, size_t parameterLength = 0, bool close = true);

  uint8_t beginIpRequestFrame(IpAddress address, size_t dataLength, bool closeSocket);

  uint8_t sendIpFromProgMem(uint8_t check, PGM_P data, size_t length);

  uint8_t sendIpFromData(uint8_t check, const uint8_t* data, size_t length);
  
  uint8_t sendIpFromChunkSize(uint8_t check, int number);

  void completeIpRequestFrame(uint8_t check);

  void atBuffer(uint8_t ch);

  void atBufferReset();

  void sendByte(uint8_t ch, bool escape = true);

  void sendShort(uint16_t v, bool escape = true);
  
  bool waitAtResponseSync();

  void reset();

private:
  SoftwareSerial _xbee;

  enum State
  {
    state_start = 1,
    state_ignore = 2,

    state_new_packet = 3,
    state_lsb_length = 4,

    state_api_id = 5,

    state_at_response = 20,
    state_at_command_msb = 21,
    state_at_command_lsb = 22,
    state_at_command_status = 23,
    state_at_command_read_data = 24,
    state_at_checksum = 25,
    state_at_response_last = 39,

    state_ip_response = 40,
    state_ip_source1 = 41,
    state_ip_source2 = 42,
    state_ip_source3 = 43,
    state_ip_destination_port_msb = 44,
    state_ip_destination_port_lsb = 45,
    state_ip_source_port_msb = 46,
    state_ip_source_port_lsb = 47,
    state_ip_protocol = 48,
    state_ip_reserved = 49,
    state_ip_read_data = 50,
    state_ip_checksum = 51,
    state_ip_response_last = 59,

    state_ip_status = 60,
    state_ip_status_frame_read = 61,
    state_ip_status_read = 62,
    state_ip_status_last = 79
  };

  int _rate;
  
  uint8_t _atBuffer[AT_BUFFER_LENGTH];
  uint8_t _atBufferCount;

  bool _escape;
  uint8_t _state;

  uint8_t _checksum;

  uint16_t _dataLength;
  uint8_t _check;
  uint8_t _apiId;
  uint8_t _apiFrameId;
  uint16_t _responseCommand;
  uint8_t _responseStatus;

  IpAddress _address;
  uint8_t _ipProtocol;
  
  HttpService _http;

  enum AtResponseStateSync
  {
    at_sync_waiting,
    at_sync_failed,
    at_sync_ok,
    at_sync_async
  };

  AtResponseStateSync _sync;

  Closure<XBeeAtCallback> _atCallback;
};

#endif

