#include "xbee_print.h"

#include <HardwareSerial.h>

XbeePrint::XbeePrint(XbeeService* xbee, IpAddress* address, XbeeService::HttpResponseType type)
  : _xbee(xbee), _ip(address), _type(type), _count(0), _started(false)
{  
}

XbeePrint::~XbeePrint()
{
  flush();
  
  if (_started)
    _xbee->ipEnd(_ip, _type);
}

size_t XbeePrint::write(uint8_t ch)
{
  _buffer[_count++] = ch;

  if (_count == XBEE_PRINT_BUFFER)
    flush();
}

void XbeePrint::flush()
{
  if (_count > 0)
  {
    if (!_started)
    {
      _xbee->ipBegin(_ip, _type);
      _started = true;
    }

    _xbee->ipChunk(_ip, _type, _buffer, _count);

    _count = 0;
  }
}

