#include "ELClientWebServer.h"

typedef enum {
  WS_LOAD=0,  // page first load
  WS_REFRESH, // page refresh
  WS_BUTTON,  // button press
  WS_SUBMIT,  // form submission
} RequestReason;

typedef enum
{
  WEB_STRING=0,  // value type string
  WEB_NULL,      // value type null
  WEB_INTEGER,   // value type integer
  WEB_BOOLEAN,   // value type boolean
  WEB_FLOAT,     // value type float
  WEB_JSON       // value type json
} WebValueType;


static ELClientWebServer * ELClientWebServer::instance = 0;

ELClientWebServer::ELClientWebServer(ELClient* elc) :_elc(elc),handlers(0), arg_ptr(0) {
  // save the current packet handler and register a new one
  instance = this;
  
  webServerCb.attach(&ELClientWebServer::webServerPacketHandler);
}

// packet handler for web-server
static void ELClientWebServer::webServerPacketHandler(void * response)
{
  ELClientWebServer::getInstance()->processResponse((ELClientResponse*)response);
}

URLHandler * ELClientWebServer::createURLHandler(const char * URL)
{
  String s = URL;
  return createURLHandler(s);
}

URLHandler * ELClientWebServer::createURLHandler(const __FlashStringHelper * URL)
{
  String s = URL;
  return createURLHandler(s);
}

URLHandler * ELClientWebServer::createURLHandler(const String &URL)
{
  struct URL_HANDLER * hnd = new struct URL_HANDLER(); // "new" is used here instead of malloc to call String destructor at freeing. DOn't use malloc/free.
  hnd->URL = URL;             // handler URL
  hnd->next = handlers;       // next handler
  handlers = hnd;             // change the first handler
  return hnd;
}

void ELClientWebServer::destroyURLHandler(URLHandler * handler)
{
  struct URL_HANDLER *prev = 0;
  struct URL_HANDLER *hnd = handlers;
  while( hnd != 0 )
  {
    if( hnd == handler )
    {
      if( prev == 0 )
        handlers = hnd->next;
      else
        prev->next = hnd->next;
      
      delete hnd;
      return;
    }
    prev = hnd;
    hnd = hnd->next;
  }
}

// initialization
void ELClientWebServer::setup()
{
  // WebServer doesn't send messages to MCU only if asked
  // register here to the web callback
  // periodic reregistration is required in case of ESP8266 reset
  _elc->Request(CMD_WEB_SETUP, 0, 1);
  uint32_t cb = &webServerCb;
  _elc->Request(&cb, 4);
  _elc->Request();
}

void ELClientWebServer::processResponse(ELClientResponse *response)
{
  uint16_t shrt;
  response->popArg(&shrt, 2);
  RequestReason reason = (RequestReason)shrt; // request reason

  response->popArg(remote_ip, 4);    // remote IP address
  response->popArg(&remote_port, 2); // remote port

  char * url;
  int urlLen = response->popArgPtr(&url);
  
  struct URL_HANDLER *hnd = handlers;
  while( hnd != 0 )
  {
    if( hnd->URL.length() == urlLen && memcmp( url, hnd->URL.begin(), urlLen ) == 0 )
      break;
    hnd = hnd->next;
  }

  if( hnd == 0 ) // no handler found for the URL
  {
    _elc->_debug->print(F("Handler not found for URL:"));
    
    for(int i=0; i < urlLen; i++)
      _elc->_debug->print( url[i] );
    _elc->_debug->println();
    return;
  }

  switch(reason)
  {
    case WS_BUTTON: // invoked when a button pressed
      {
        char * idPtr;
        int idLen = response->popArgPtr(&idPtr);
  
        // add terminating 0
        char id[idLen+1];
        memcpy(id, idPtr, idLen);
        id[idLen] = 0;

        hnd->buttonCb(id);
      }
      break;
    case WS_SUBMIT: // invoked when a form submitted
      {
        int cnt = 4;

        while( cnt < response->argc() )
        {
          char * idPtr;
          int idLen = response->popArgPtr(&idPtr);
          int nameLen = strlen(idPtr+1);
          int valueLen = idLen - nameLen -2;

          // add terminating 0
          arg_ptr = (char *)malloc(valueLen+1);
          arg_ptr[valueLen] = 0;
          memcpy(arg_ptr, idPtr + 2 + nameLen, valueLen);

          hnd->setFieldCb(idPtr+1);
  
          free(arg_ptr);
          arg_ptr = 0;
          cnt++;
        }
      }
      return;
    case WS_LOAD: // invoked at refresh / load
    case WS_REFRESH:
      break;
    default:
      return;
  }
  
  // the response is generated here with the fields to refresh

  _elc->Request(CMD_WEB_DATA, 100, VARIABLE_ARG_NUM);
  _elc->Request(remote_ip, 4);               // send remote IP address
  _elc->Request((uint8_t *)&remote_port, 2); // send remote port

  if( reason == WS_LOAD )
    hnd->loadCb( hnd->URL.begin() );
  else
    hnd->refreshCb( hnd->URL.begin() );

  _elc->Request((uint8_t *)NULL, 0);         // end indicator
  _elc->Request();                           // finish packet
}


void ELClientWebServer::setArgJson(const char * name, const char * value)
{
  uint8_t nlen = strlen(name);
  uint8_t vlen = strlen(value);
  char buf[nlen+vlen+3];
  buf[0] = WEB_JSON;
  strcpy(buf+1, name);
  strcpy(buf+2+nlen, value);
  _elc->Request(buf, nlen+vlen+2);
}

void ELClientWebServer::setArgJson(const __FlashStringHelper * name, const __FlashStringHelper * value)
{
  const char * name_p = reinterpret_cast<const char *>(name);
  const char * value_p = reinterpret_cast<const char *>(value);
  
  uint8_t nlen = strlen_P(name_p);
  uint8_t vlen = strlen_P(value_p);
  char buf[nlen+vlen+3];
  buf[0] = WEB_JSON;
  strcpy_P(buf+1, name_p);
  strcpy_P(buf+2+nlen, value_p);
  _elc->Request(buf, nlen+vlen+2);
}

void ELClientWebServer::setArgJson(const __FlashStringHelper * name, const char * value)
{
  const char * name_p = reinterpret_cast<const char *>(name);
  
  uint8_t nlen = strlen_P(name_p);
  uint8_t vlen = strlen(value);
  char buf[nlen+vlen+3];
  buf[0] = WEB_JSON;
  strcpy_P(buf+1, name_p);
  strcpy(buf+2+nlen, value);
  _elc->Request(buf, nlen+vlen+2);
}


void ELClientWebServer::setArgString(const char * name, const char * value)
{
  uint8_t nlen = strlen(name);
  uint8_t vlen = strlen(value);
  char buf[nlen+vlen+3];
  buf[0] = WEB_STRING;
  strcpy(buf+1, name);
  strcpy(buf+2+nlen, value);
  _elc->Request(buf, nlen+vlen+2);
}

void ELClientWebServer::setArgString(const __FlashStringHelper * name, const __FlashStringHelper * value)
{
  const char * name_p = reinterpret_cast<const char *>(name);
  const char * value_p = reinterpret_cast<const char *>(value);
  
  uint8_t nlen = strlen_P(name_p);
  uint8_t vlen = strlen_P(value_p);
  char buf[nlen+vlen+3];
  buf[0] = WEB_STRING;
  strcpy_P(buf+1, name_p);
  strcpy_P(buf+2+nlen, value_p);
  _elc->Request(buf, nlen+vlen+2);
}

void ELClientWebServer::setArgString(const __FlashStringHelper * name, const char * value)
{
  const char * name_p = reinterpret_cast<const char *>(name);
  
  uint8_t nlen = strlen_P(name_p);
  uint8_t vlen = strlen(value);
  char buf[nlen+vlen+3];
  buf[0] = WEB_STRING;
  strcpy_P(buf+1, name_p);
  strcpy(buf+2+nlen, value);
  _elc->Request(buf, nlen+vlen+2);
}

void ELClientWebServer::setArgBoolean(const char * name, uint8_t value)
{
  uint8_t nlen = strlen(name);
  char buf[nlen + 4];
  buf[0] = WEB_BOOLEAN;
  strcpy(buf+1, name);
  buf[2 + nlen] = value;
  _elc->Request(buf, nlen+3);
}

void ELClientWebServer::setArgBoolean(const __FlashStringHelper * name, uint8_t value)
{
  const char * name_p = reinterpret_cast<const char *>(name);
  
  uint8_t nlen = strlen_P(name_p);
  char buf[nlen + 4];
  buf[0] = WEB_BOOLEAN;
  strcpy_P(buf+1, name_p);
  buf[2 + nlen] = value;
  _elc->Request(buf, nlen+3);
}

void ELClientWebServer::setArgInt(const char * name, int32_t value)
{
  uint8_t nlen = strlen(name);
  char buf[nlen + 7];
  buf[0] = WEB_INTEGER;
  strcpy(buf+1, name);
  memcpy(buf+2+nlen, &value, 4);
  _elc->Request(buf, nlen+6);
}

void ELClientWebServer::setArgInt(const __FlashStringHelper * name, int32_t value)
{
  const char * name_p = reinterpret_cast<const char *>(name);
  
  uint8_t nlen = strlen_P(name_p);
  char buf[nlen + 7];
  buf[0] = WEB_INTEGER;
  strcpy_P(buf+1, name_p);
  memcpy(buf+2+nlen, &value, 4);
  _elc->Request(buf, nlen+6);
}

void ELClientWebServer::setArgNull(const char * name)
{
  uint8_t nlen = strlen(name);
  char buf[nlen + 2];
  buf[0] = WEB_NULL;
  strcpy(buf+1, name);
  _elc->Request(buf, nlen+2);
}

void ELClientWebServer::setArgNull(const __FlashStringHelper * name)
{
  const char * name_p = reinterpret_cast<const char *>(name);
  
  uint8_t nlen = strlen_P(name_p);
  char buf[nlen + 2];
  buf[0] = WEB_NULL;
  strcpy_P(buf+1, name_p);
  _elc->Request(buf, nlen+2);
}

void ELClientWebServer::setArgFloat(const char * name, float value)
{
  uint8_t nlen = strlen(name);
  char buf[nlen + 7];
  buf[0] = WEB_FLOAT;
  strcpy(buf+1, name);
  memcpy(buf+2+nlen, &value, 4);
  _elc->Request(buf, nlen+6);
}

void ELClientWebServer::setArgFloat(const __FlashStringHelper * name, float value)
{
  const char * name_p = reinterpret_cast<const char *>(name);
  
  uint8_t nlen = strlen_P(name_p);
  char buf[nlen + 7];
  buf[0] = WEB_FLOAT;
  strcpy_P(buf+1, name_p);
  memcpy(buf+2+nlen, &value, 4);
  _elc->Request(buf, nlen+6);
}

int32_t ELClientWebServer::getArgInt()
{
  return (int32_t)atol(arg_ptr);
}

char * ELClientWebServer::getArgString()
{
  return arg_ptr;
}

uint8_t ELClientWebServer::getArgBoolean()
{
  if( strcmp_P(arg_ptr, PSTR("on")) == 0 )
    return 1;
  if( strcmp_P(arg_ptr, PSTR("true")) == 0 )
    return 1;
  if( strcmp_P(arg_ptr, PSTR("yes")) == 0 )
    return 1;
  if( strcmp_P(arg_ptr, PSTR("1")) == 0 )
    return 1;
  return 0;
}

float ELClientWebServer::getArgFloat()
{
  return atof(arg_ptr);
}
