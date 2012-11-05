#include "xbee.h"

#include <HardwareSerial.h>

#define TIME_IP_0 96
#define TIME_IP_1 47
#define TIME_IP_2 67
#define TIME_IP_3 105
#define TIME_PORT 37
#define TIME_LOCAL_PORT 30999
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24UL)
#define DAYS_PER_WEEK (7UL)
#define SECS_PER_WEEK (SECS_PER_DAY * DAYS_PER_WEEK)
#define SECS_PER_YEAR (SECS_PER_WEEK * 52UL)
#define SECS_YR_2000  (946684800UL) // the time at the start of y2k

#define ATAP 2

#define START_BYTE 0x7e
#define ESCAPE 0x7d
#define XON 0x11
#define XOFF 0x13

#define API_AT_REQUEST 0x8
#define API_AT_RESPONSE 0x88
#define API_IP_RESPONSE 0xB0
#define API_IP_REQUEST 0x20
#define API_IP_STATUS 0x89

const prog_char PROGMEM responseNotFound[] = "HTTP/1.1 404 Not Found\r\nServer: ard/0.1\r\nContent-Type: text/plain\r\nContent-Length: 3\r\nConnection: close\r\n\r\n404";
const prog_char PROGMEM responseFound[] = "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\nServer: ard/0.1\r\nConnection: close\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: text/plain\r\nExpires:";
const prog_char PROGMEM responseFoundNonCached[] = "Sun, 30 Oct 2011 01:00:00 GMT\r\nCache-Control: no-cache\r\n\r\n";
const prog_char PROGMEM responseFoundCached[] = "Tue, 30 Oct 2029 01:00:00 GMT\r\nCache-Control: public\r\n\r\n";
const prog_char PROGMEM responseTerminator[] = "0\r\n\r\n\r\n";
const prog_char PROGMEM responseTerminatorShort[] = "\r\n";

XbeeService::XbeeService(int pin_in, int pin_out, int rate)
  : _xbee(pin_in, pin_out), _rate(rate), _sync(at_sync_async)
{
  reset();
}

void XbeeService::setAtCallback(Closure<XBeeAtCallback> callback)
{
  _atCallback = callback;
}

void XbeeService::setRequestCallback(Closure<HttpService::HttpRequestCallback> callback)
{
  _http.setRequestCallback(callback);
}

void XbeeService::atCommand(const char* command)
{
  uint8_t l = 0;
  for (const char* c = command; *c; ++c, ++l);
  
  atCommand((uint8_t*)command, l);
}

void XbeeService::atCommand(uint8_t command[], uint8_t count)
{
  sendByte(START_BYTE, false);

  uint16_t length = count + 2;
  sendShort(length);

  uint8_t check = 0;
  sendByte(API_AT_REQUEST); check += API_AT_REQUEST;
  sendByte(0x1); check += 0x1;

  for (uint8_t i = 0; i < count; ++i)
  {
    sendByte(command[i]);
    check += command[i];
  }

  sendByte((uint8_t)0xff - check);

  _xbee.flush();
}

size_t getLength(const uint8_t* data)
{
  size_t l = 0;
  for (const uint8_t* c = data; *c; ++c, ++l);    
  return l;
}

void XbeeService::ipSendInternal(IpAddress* address, HttpResponseType type, const uint8_t* parameterData, size_t parameterLength, bool close)
{
  if (parameterData && !parameterLength) 
    parameterLength = getLength(parameterData);
    
  uint8_t packetSize = parameterLength > 0 ? 6 + parameterLength + 2 : 0;
  if (close)
    packetSize += 7;
    
  switch (type)
  {
    case ip_raw:
    {
      uint8_t check = beginIpRequestFrame(*address, parameterLength, true);
      if (parameterData)
        check = sendIpFromData(check, parameterData, parameterLength);
      completeIpRequestFrame(check);
      break;
    }
    case http_headerless:
    {
      uint8_t check = beginIpRequestFrame(*address, 6 + parameterLength + 2, true);
      check = sendIpFromChunkSize(check, parameterLength);
      check = sendIpFromData(check, parameterData, parameterLength);
      check = sendIpFromProgMem(check, responseTerminatorShort, sizeof(responseTerminatorShort) - 1);
      completeIpRequestFrame(check);
      break;
    }
    case http_not_found:
    {
      uint8_t check = beginIpRequestFrame(*address, sizeof(responseNotFound), close);
      check = sendIpFromProgMem(check, responseNotFound, sizeof(responseNotFound));
      completeIpRequestFrame(check);
      break;
    }
    case http_found_cached:
    {
      uint8_t check = beginIpRequestFrame(*address, packetSize + sizeof(responseFound) - 1 + sizeof(responseFoundCached) - 1, close);
      check = sendIpFromProgMem(check, responseFound, sizeof(responseFound) - 1);
      check = sendIpFromProgMem(check, responseFoundCached, sizeof(responseFoundCached) - 1);
      if (parameterLength > 0)
      {
        check = sendIpFromChunkSize(check, parameterLength);
        check = sendIpFromData(check, parameterData, parameterLength);
        check = sendIpFromProgMem(check, responseTerminatorShort, sizeof(responseTerminatorShort) - 1);
      }
      if (close)
        check = sendIpFromProgMem(check, responseTerminator, sizeof(responseTerminator) - 1);
      completeIpRequestFrame(check);
      break;
    }
    case http_found_non_cached:
    {
      uint8_t check = beginIpRequestFrame(*address, packetSize + sizeof(responseFound) - 1 + sizeof(responseFoundNonCached) - 1, close);
      check = sendIpFromProgMem(check, responseFound, sizeof(responseFound) - 1);
      check = sendIpFromProgMem(check, responseFoundNonCached, sizeof(responseFoundNonCached) - 1);
      if (parameterLength > 0)
      {
        check = sendIpFromChunkSize(check, parameterLength);
        check = sendIpFromData(check, parameterData, parameterLength);
        check = sendIpFromProgMem(check, responseTerminatorShort, sizeof(responseTerminatorShort) - 1);
      }
      if (close)
        check = sendIpFromProgMem(check, responseTerminator, sizeof(responseTerminator) - 1);
      completeIpRequestFrame(check);
      break;
    }
  }
}

void XbeeService::ipSend(IpAddress* address, HttpResponseType type, const uint8_t* parameterData, size_t parameterLength)
{
  ipSendInternal(address, type, parameterData, parameterLength, true);
}

void XbeeService::ipBegin(IpAddress* address, HttpResponseType type, const uint8_t* parameterData, size_t parameterLength)
{
  if (parameterData && !parameterLength) 
    parameterLength = getLength(parameterData);
    
  ipSendInternal(address, type, parameterData, parameterLength, false);
}

void XbeeService::ipChunk(IpAddress* address, HttpResponseType type, const uint8_t* parameterData, size_t parameterLength)
{
  if (parameterData && !parameterLength) 
    parameterLength = getLength(parameterData);
    
  ipSendInternal(address, type == ip_raw ? ip_raw : http_headerless, parameterData, parameterLength, false);
}

void XbeeService::ipEnd(IpAddress* address, HttpResponseType type)
{
  if (type == ip_raw)
  {
    uint8_t check = beginIpRequestFrame(*address, 0, true);
    completeIpRequestFrame(check);
  }
  else
  {
    uint8_t check = beginIpRequestFrame(*address, sizeof(responseTerminator) - 1, true);
    check = sendIpFromProgMem(check, responseTerminator, sizeof(responseTerminator) - 1);
    completeIpRequestFrame(check);
  }
}

bool XbeeService::ipIsReading() const
{
  return _state >= state_ip_response && _state < state_ip_response_last;
}

void XbeeService::update(int mill)
{
  while (_xbee.available())
  {      
    // Read a byte from XBee and unescape it.
    // XBee should be in appropriate mode (API with escaping).
    int port = _xbee.read();
    if (port == -1)
      continue;        
    uint8_t ch = (uint8_t)port;
    
    // Begin packet.
    if (ch == START_BYTE)
    {
      reset();
      _state = state_new_packet;
      continue;
    }
    
    // Escape character.
    if (ch == ESCAPE)
    {
      if (_xbee.available())
      {
        ch = 0x20 ^ _xbee.read();
      }
      else
      {
        _escape = true;
        continue;
      }
    }
    
    if (_escape)
    {
      ch = 0x20 ^ ch;
      _escape = false;
    }
    
    switch (_state)
    {
      case state_ignore: // Ignoring this packet.
        break;
        
      // Packet reading.

      case state_new_packet: // Read msb of data length.
        _dataLength = ch;
        _state = state_lsb_length;
        break;

      case state_lsb_length: // Read lsb of data length.
        _dataLength = (_dataLength << 8) | ch;
        _state = state_api_id;
        break;

      case state_api_id: // Read api description.
        _checksum = ch;
        _apiId = ch;
        Serial.print("Frame:");
        Serial.println(_apiId, HEX);
        switch (_apiId)
        {
          case API_AT_RESPONSE: // AT response.
            _state = state_at_response;
            break;

          case API_IP_RESPONSE: // IP response/data.
            _state = state_ip_response;
            break;

          case API_IP_STATUS: // IP send status.
            _state = state_ip_status_frame_read;
            break;

          default:
            _state = state_ignore;
            break;
        }
        break;

      // IP send status.
      case state_ip_status_frame_read:
        _state = state_ip_status_read;
        break;

      case state_ip_status_read:
        // Read command status.
        Serial.print("Status:");
        Serial.println(ch, HEX);
        _state = state_ignore;
        break;

      // IP command response.
      case state_ip_response: // Read byte 0 of source IP.
        _checksum += ch;
        _address.sourceIp[0] = ch;
        _state = state_ip_source1;
        break;

      case state_ip_source1: // Read byte 1 of source IP.
        _checksum += ch;
        _address.sourceIp[1] = ch;
        _state = state_ip_source2;
        break;

      case state_ip_source2: // Read byte 2 of source IP.
        _checksum += ch;
        _address.sourceIp[2] = ch;
        _state = state_ip_source3;
        break;

      case state_ip_source3: // Read byte 3 of source IP.
        _checksum += ch;
        _address.sourceIp[3] = ch;
        _state = state_ip_destination_port_msb;
        break;

      case state_ip_destination_port_msb: // Read destination port msb.
        _checksum += ch;
        _address.destinationPort[0] = ch;
        _state = state_ip_destination_port_lsb;
        break;

      case state_ip_destination_port_lsb: // Read destination port lsb.
        _checksum += ch;
        _address.destinationPort[1] = ch;
        _state = state_ip_source_port_msb;
        break;

      case state_ip_source_port_msb: // Read source port msb.
        _checksum += ch;
        _address.sourcePort[0] = ch;
        _state = state_ip_source_port_lsb;
        break;

      case state_ip_source_port_lsb: // Read source port lsb.
      {
        _checksum += ch;
        _address.sourcePort[1] = ch;

        _http.begin(&_address);
          
        _state = state_ip_protocol;
        break;
      }

      case state_ip_protocol: // Read protocol.
        _checksum += ch;
        _ipProtocol = ch;
        _state = state_ip_reserved;
        break;

      case state_ip_reserved: // Read reserved part.
        _checksum += ch;
        _state = state_ip_read_data;
        _dataLength -= 11;
        if (_dataLength == 0) 
        {
          _state = state_ip_checksum;
        }
        break;

      case state_ip_read_data: // Read data while there's something to read.
        _checksum += ch;
        _http.add(ch);
        if (--_dataLength == 0)
        {
          _http.donePacket();
          _state = state_ip_checksum;
        }
        break;

      case state_ip_checksum: // Read IP packet checksum.
        _state = state_ignore;
        break;

      // AT command response.
      case state_at_response: // Get API frame id.
        _checksum += ch;
        _apiFrameId = ch;
        _state = state_at_command_msb;
        break;

      case state_at_command_msb: // Read msb of original AT command.
        _checksum += ch;
        _responseCommand = ch;
        _state = state_at_command_lsb;
        break;

      case state_at_command_lsb: // Read lsb of original AT command.
        _checksum += ch;
        _responseCommand = (_responseCommand << 8) | ch;
        _state = state_at_command_status;
        break;

      case state_at_command_status: // Read AT command status.      
        _checksum += ch;
        _responseStatus = ch;
        _state = state_at_command_read_data;
        atBufferReset();
        // Calculate length of remaining data.
        _dataLength -= 5;
        // No data packet included, just skip to checksum.
        if (_dataLength == 0)
          _state = state_at_checksum;
        break;

      case state_at_command_read_data:
        _checksum += ch;
        atBuffer(ch);
        if (--_dataLength == 0)
          _state = state_at_checksum;
        break;

      case state_at_checksum:
        if (0xff - _checksum == ch)
        {
          if (_sync == at_sync_waiting) 
          {
            _sync = _responseStatus == 0 ? at_sync_ok : at_sync_failed;
          }
          else
          {
            if (_atCallback)
              _atCallback(_responseCommand, _apiFrameId, _responseStatus, _atBuffer, _atBufferCount);
          }
        }
        _state = state_ignore;
        break;
    }
  }
}

void XbeeService::setup()
{
  _xbee.begin(_rate);
  
  if (atCommandSync("C0"))
  {    
    if (_atBufferCount > 1)
      listeningOnPort((_atBuffer[0] << 8) | _atBuffer[1]);
  }
}

void XbeeService::listeningOnPort(uint16_t port)
{
  _http.listeningOnPort(port);
}

uint8_t XbeeService::beginIpRequestFrame(IpAddress address, size_t dataLength, bool closeSocket)
{
  sendByte(START_BYTE, false);

  uint16_t length = dataLength + 12;
  sendShort(length);

  uint8_t check = 0;
  sendByte(API_IP_REQUEST); check += API_IP_REQUEST;
  // Frame id.
  sendByte(0x1); check += 0x1;

  sendByte(address.sourceIp[0]); check += address.sourceIp[0];
  sendByte(address.sourceIp[1]); check += address.sourceIp[1];
  sendByte(address.sourceIp[2]); check += address.sourceIp[2];
  sendByte(address.sourceIp[3]); check += address.sourceIp[3];

  sendByte(address.sourcePort[0]); check += address.sourcePort[0];
  sendByte(address.sourcePort[1]); check += address.sourcePort[1];

  sendByte(address.destinationPort[0]); check += address.destinationPort[0];
  sendByte(address.destinationPort[1]); check += address.destinationPort[1];

  sendByte(0x1); check += 0x1;

  uint8_t close = closeSocket ? 0x1 : 0x0;
  sendByte(close); check += close;

  return check;
}

uint8_t XbeeService::sendIpFromProgMem(uint8_t check, PGM_P data, size_t length)
{
  for (int i = 0; i < length; ++i)
  {
    uint8_t byte = pgm_read_byte(data);
    data++;

    sendByte(byte);
    check += byte;
  }
  
  return check;
}

uint8_t XbeeService::sendIpFromChunkSize(uint8_t check, int number)
{
  char n[] = {
    ((number >> 12) & 0xf),
    ((number >>  8) & 0xf),
    ((number >>  4) & 0xf),
    ((number      ) & 0xf),
    '\r',
    '\n',
    0
  };
  
  n[0] += n[0] > 9 ? ('A' - 10) : '0';
  n[1] += n[1] > 9 ? ('A' - 10) : '0';
  n[2] += n[2] > 9 ? ('A' - 10) : '0';
  n[3] += n[3] > 9 ? ('A' - 10) : '0';
  
  return sendIpFromData(check, (const uint8_t*)n + 0, 6); 
}

uint8_t XbeeService::sendIpFromData(uint8_t check, const uint8_t* data, size_t length)
{
  for (int i = 0; i < length; ++i)
  {
    uint8_t byte = *(data++);
    sendByte(byte);
    check += byte;
  }

  return check;
}

void XbeeService::completeIpRequestFrame(uint8_t check)
{
  sendByte((uint8_t)0xff - check);
  _xbee.flush();
}

void XbeeService::atBuffer(uint8_t ch)
{
  if (_atBufferCount < AT_BUFFER_LENGTH)
    _atBuffer[_atBufferCount++] = ch;
}

void XbeeService::atBufferReset()
{
  _atBufferCount = 0;
}

void XbeeService::sendByte(uint8_t ch, bool escape)
{
  if (escape && (ch == START_BYTE || ch == ESCAPE || ch == XON || ch == XOFF))
  {
    _xbee.write(ESCAPE);
    _xbee.write(ch ^ 0x20);
  }
  else
    _xbee.write(ch);
}

void XbeeService::sendShort(uint16_t v, bool escape)
{
  sendByte((v >> 8) && 0xff, escape);
  sendByte(v & 0xff, escape);
}

void XbeeService::reset()
{
  _checksum = 0;
  _escape = false;
  _state = state_start;
  _atBufferCount = 0;
}

bool XbeeService::waitAtResponseSync()
{
  _sync = at_sync_waiting;

  AtResponseStateSync result;
  for (;;)
  {
    update(1);
    if (_sync != at_sync_waiting)
    {
      result = _sync;
      _sync = at_sync_async;
      break;
    }
  }

  return result == at_sync_ok; 
}

bool XbeeService::atCommandSync(uint8_t command[], uint8_t count)
{
  atCommand(command, count);
  
  return waitAtResponseSync();
}
  
bool XbeeService::atCommandSync(const char* command)
{
  atCommand(command);
  
  return waitAtResponseSync();
}

const uint8_t* XbeeService::atBufferSync() const
{
  return _atBuffer;
}

uint8_t XbeeService::atBufferCountSync() const
{
  return _atBufferCount;
}

void XbeeService::requestCurrentTime()
{
  IpAddress addr;
  addr.set(TIME_IP_0, TIME_IP_1, TIME_IP_2, TIME_IP_3, TIME_PORT, TIME_LOCAL_PORT);      
  
  // It seems XBee won't send empty packet, so try to send something.
  ipSend(&addr, XbeeService::ip_raw, (uint8_t*)"Time");
}

bool XbeeService::isTimeResponse(int8_t timeZoneOffset, IpAddress* address, HttpService::RequestType type, const uint8_t* data, size_t parameterLength, uint8_t& hours, uint8_t& minutes, uint8_t& seconds)
{
  if (type == HttpService::request_raw)
  {     
    if (address->sourceIp[0] == TIME_IP_0 && address->sourceIp[1] == TIME_IP_1 && address->sourceIp[1] == TIME_IP_1 && address->sourceIp[1] == TIME_IP_1)
    {
      if (parameterLength >= 4)
      {
        uint32_t time = ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) | ((uint32_t)data[2] << 8) | ((uint32_t)data[3]);
        time -= 2208988800UL; // Adjust for unit epoc. But is not really necessary.
        time += timeZoneOffset * SECS_PER_HOUR; // Adjust for time zone.
        seconds = time % SECS_PER_MIN;
        minutes = (time / SECS_PER_MIN) % SECS_PER_MIN;
        hours = (time % SECS_PER_DAY) / SECS_PER_HOUR;
          
        return true;
      }
    }
  }
  
  return false;
}

