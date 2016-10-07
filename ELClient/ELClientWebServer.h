// Copyright (c) 2016 by B. Runnels and T. von Eicken

#ifndef _EL_CLIENT_WEB_SERVER_H_
#define _EL_CLIENT_WEB_SERVER_H_

#include <Arduino.h>
#include "ELClient.h"
#include "FP.h"

typedef struct URL_HANDLER
{
  String                URL;         // the URL to handle
  FP<void, char*>       loadCb;      // callback for HTML page loading
  FP<void, char*>       refreshCb;   // callback for HTML page refresh
  FP<void, char*>       setFieldCb;  // callback for setting a field from an HTML form
  FP<void, char*>       buttonCb;    // callback for HTML button press
  struct URL_HANDLER *  next;        // next handler
} URLHandler;

// This class implements function for web-server
class ELClientWebServer {
public:
  // Initialize with an ELClient object
  ELClientWebServer(ELClient* elc);
  
  // initializes the web-server
  void    setup();
  
  // creates an URL handler
  URLHandler * createURLHandler(const char * URL);
  // creates an URL handler from flash
  URLHandler * createURLHandler(const __FlashStringHelper * URL);
  // creates an URL handler from String
  URLHandler * createURLHandler(const String &s);
  // destroys an URL handler
  void    destroyURLHandler(URLHandler * handler);
  
  // sets int value of an HTML field
  void    setArgInt(const char * name, int32_t value);
  // sets JSON value of an HTML field
  void    setArgJson(const char * name, const char * value);
  // sets string value of an HTML field
  void    setArgString(const char * name, const char * value);
  // sets boolean value of an HTML field
  void    setArgBoolean(const char * name, uint8_t value);
  // sets null value of an HTML field
  void    setArgNull(const char * name);
  // sets float value of an HTML field
  void    setArgFloat(const char * name, float f);

  // sets int value of an HTML field
  void    setArgInt(const __FlashStringHelper * name, int32_t value);
  // sets JSON value of an HTML field
  void    setArgJson(const __FlashStringHelper * name, const char * value);
  // sets JSON value of an HTML field
  void    setArgJson(const __FlashStringHelper * name, const __FlashStringHelper * value);
  // sets string value of an HTML field
  void    setArgString(const __FlashStringHelper * name, const char * value);
  // sets string value of an HTML field
  void    setArgString(const __FlashStringHelper * name, const __FlashStringHelper * value);
  // sets boolean value of an HTML field
  void    setArgBoolean(const __FlashStringHelper * name, uint8_t value);
  // sets null value of an HTML field
  void    setArgNull(const __FlashStringHelper * name);
  // sets float value of an HTML field
  void    setArgFloat(const __FlashStringHelper * name, float f);

  // setFieldCb: gets the value of the field as integer
  int32_t getArgInt();
  // setFieldCb: gets the value of the field as string
  char *  getArgString();
  // setFieldCb: gets the value of the field as boolean
  uint8_t getArgBoolean();
  // setFieldCb: gets the value of the field as float
  float   getArgFloat();

  // returns the web-server instance
  static ELClientWebServer * getInstance() { return instance; }
  
private:
  ELClient* _elc;
  
  static void webServerPacketHandler(void * packet);
  void processResponse(ELClientResponse *packet); // internal
  static ELClientWebServer * instance;
  
  uint8_t                    remote_ip[4];
  uint16_t                   remote_port;
  
  char *                     arg_ptr;
  
  FP<void, void*>            webServerCb;
  
  struct URL_HANDLER       * handlers;
};

#endif // _EL_CLIENT_WEB_SERVER_H_
