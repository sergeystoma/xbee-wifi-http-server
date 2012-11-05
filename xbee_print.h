#ifndef _header_xbee_print_
#define _header_xbee_print_

#include "xbee.h"

#include <Print.h>

#define XBEE_PRINT_BUFFER 16

class XbeePrint: public Print
{
public:
  XbeePrint(XbeeService* xbee, IpAddress* address, XbeeService::HttpResponseType type);
  ~XbeePrint();

  virtual size_t write(uint8_t ch);

  void flush();

private:
  XbeeService* _xbee;
  IpAddress* _ip;
  XbeeService::HttpResponseType _type;
  
  bool _started;
  uint8_t _buffer[XBEE_PRINT_BUFFER];
  int _count;
};


#endif

