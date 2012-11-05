#ifndef _header_http_
#define _header_http_ 

#include "closure.h"

#include <inttypes.h>

class IpAddress
{
public:
  inline bool sameIp(const uint8_t* ip, const uint8_t* port) const
  {
    return sourcePort[0] == port[0] && sourcePort[1] == port[1] && sourceIp[3] == ip[3] && sourceIp[2] == ip[2] && sourceIp[1] == ip[1] && sourceIp[0] == ip[0];
  }

  inline void reset()
  {
    sourceIp[0] = 0;
    sourceIp[1] = 0;
    sourceIp[2] = 0;
    sourceIp[3] = 0;
  }
  
  void set(uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3, uint16_t srcPort, uint16_t dstPort)
  {
    sourceIp[0] = ip0;
    sourceIp[1] = ip1;
    sourceIp[2] = ip2;
    sourceIp[3] = ip3;
    sourcePort[0] = (srcPort >> 8) & 0xff;
    sourcePort[1] = srcPort & 0xff;
    destinationPort[0] = (dstPort >> 8) & 0xff;
    destinationPort[1] = dstPort & 0xff;
  }

public:
  uint8_t sourceIp[4];
  uint8_t sourcePort[2];
  uint8_t destinationPort[2];
};

#define HTTP_BUFFER 31
#define HTTP_LOCATION_BUFFER 31

class HttpService
{
public:
  enum RequestType
  {
    request_get,
    request_post,
    request_raw,
    request_invalid
  };

  typedef void (HttpRequestCallback)(IpAddress* address, RequestType type, uint8_t* location, int locationLength, uint8_t* data, int dataLength);
  
public:
  HttpService();

  void setRequestCallback(Closure<HttpRequestCallback> callback);
  
  void listeningOnPort(uint16_t port);

  void begin(IpAddress* address);

  inline bool sameIp(uint8_t* ip, uint8_t* port) const
  {
    return _address.sameIp(ip, port);
  }

  void add(uint8_t ch);
  
  void donePacket();
  
private:
  void addRawData(uint8_t ch);
  
  void addHeader(uint8_t ch);

  void addLocation(uint8_t ch);

  void addContentLength(uint8_t ch);

  void addFinishLine(uint8_t ch);

  void addPostData(uint8_t ch);

  bool buffer(uint8_t ch);;

private:
  enum State
  {
    state_raw_data,
    state_at_header_line_start,
    state_read_content_length,
    state_read_location,
    state_finish_line,
    state_finish_line_as_is,
    state_read_post_data
  };

  uint8_t _state;
  uint8_t _previous;
  
  uint16_t _listenPort;

  Closure<HttpRequestCallback> _requestCallback;

  RequestType _request;

  int _contentLength;

  IpAddress _address;

  uint8_t _buffer[HTTP_BUFFER + 1];
  uint8_t _length;

  uint8_t _locationBuffer[HTTP_LOCATION_BUFFER + 1];
  uint8_t _locationLength;  
};

#endif

