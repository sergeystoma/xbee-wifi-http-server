#include "http.h"

#include <stdlib.h>
#include <string.h>

HttpService::HttpService()
  : _length(0)
{
  _address.reset();
}

void HttpService::setRequestCallback(Closure<HttpRequestCallback> callback)
{
  _requestCallback = callback;
}

void HttpService::listeningOnPort(uint16_t port)
{
  _listenPort = port;
}

void HttpService::begin(IpAddress* address)
{
  _address = *address;

  _length = 0;

  uint16_t port = (_address.destinationPort[0] << 8) | _address.destinationPort[1];
  
  _state = port == _listenPort ? state_at_header_line_start : state_raw_data;
}

void HttpService::add(uint8_t ch)
{
  switch (_state)
  {
    case state_raw_data:
      addRawData(ch);
      break;
      
    case state_at_header_line_start:
      addHeader(ch);
      break;

    case state_read_location:
      addLocation(ch);
      break;

    case state_read_content_length:
      addContentLength(ch);
      break;

    case state_finish_line:
      addFinishLine(ch);
      break;

    case state_read_post_data:
      addPostData(ch);
      break;
  }
    
  _previous = ch;
}

void HttpService::donePacket() 
{
  if (_state == state_raw_data)
    if (_requestCallback)
      _requestCallback(&_address, request_raw, 0, 0, _buffer, _length);
}

void HttpService::addRawData(uint8_t ch)
{
  buffer(ch);
}

void HttpService::addHeader(uint8_t ch)
{
  // Could be GET or POST method, or some other one/
  if (ch == ' ')
  {
    if (_length == 3 && _buffer[0] == 'G' && _buffer[1] == 'E' && _buffer[2] == 'T')
    {
      _request = request_get;
    }
    else if (_length == 4 && _buffer[0] == 'P' && _buffer[1] == 'O' && _buffer[2] == 'S' && _buffer[3] == 'T')
    {
      _request = request_post;
    }
    else
    {
      _request = request_invalid;
    }

    _locationLength = 0;
    _contentLength = 0;
    
    _state = state_read_location;
  }
  else if (ch == ':')
  {  
    // Check content headers.
    if (_length == 14 && strncasecmp((char*)_buffer, "content-length", 14) == 0)
    {
      _length = 0;
      _state = state_read_content_length;
    }
    else {
      _state = state_finish_line;
    }
  }
  else if (_previous == '\r' && ch == '\n')
  {
    if (_length == 0)
    {
      // Empty line after request headers.
      if (_contentLength == 0)
      {
        // Done reading request.
        _state = state_at_header_line_start;
        _length = 0;
        if (_requestCallback)
          _requestCallback(&_address, _request, _locationBuffer, _locationLength, _buffer, _length);
      }
      else
      {
        _length = 0;
        _state = state_read_post_data;
      }
    }
    // End of line without hitting anything important.
    _length = 0;
  }
  else if (ch == '\r')
  {
    // Do nothing.
  }
  else
  {
    // Otherwise just buffer the character.
    buffer(ch);
  }
}

void HttpService::addLocation(uint8_t ch)
{
  // GET /Something/ HTTP/1.0
  // Spaces before and after the url.
  if (ch == ' ')
  {
    if (_locationLength == 0)
      return;

    _locationBuffer[_locationLength++] = 0;
    _state = state_finish_line;
    return;
  }

  if (_locationLength < HTTP_LOCATION_BUFFER)
    _locationBuffer[_locationLength++] = ch;
}

void HttpService::addContentLength(uint8_t ch)
{
  if (ch == '\r')
  {
    // Do nothing.
  }
  else if (_previous == '\r' && ch == '\n')
  {
    // Finished line.
    // Null terminate and calculate content length.
    _buffer[_length] = 0;
    _contentLength = atoi((char*)_buffer);
    _length = 0;
    _state = state_at_header_line_start;
  }
  else if (ch == ' ')
  {
    if (_length > 0)
    {
      _buffer[_length] = 0;
      _contentLength = atoi((char*)_buffer);
      _length = 0;
      _state = state_finish_line;
    }
  }
  else
    buffer(ch);
}

void HttpService::addFinishLine(uint8_t ch)
{
  if (_previous == '\r' && ch == '\n')
  {
    _length = 0;
    _state = state_at_header_line_start;
  }
}

void HttpService::addPostData(uint8_t ch)
{
  buffer(ch);

  if (--_contentLength == 0)
  {
    // Got all POSTed data.
    _state = state_at_header_line_start;
    if (_requestCallback)
      _requestCallback(&_address, _request, _locationBuffer, _locationLength, _buffer, _length);
  }
}

bool HttpService::buffer(uint8_t ch)
{
  if (_length < HTTP_BUFFER)
  {
    _buffer[_length++] = ch;
    return true;
  }

  return false;
}

