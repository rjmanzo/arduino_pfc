#include "pfodESP8266BufferedClient.h"
/**
 (c)2015 Forward Computing and Control Pty. Ltd.
 This code may be freely used for both private and commerical use.
 Provide this copyright is maintained.
*/

// uncomment this next line and call setDebugStream(&Serial); to enable debug out
//#define DEBUG

void pfodESP8266BufferedClient::setDebugStream(Print* out) {
  debugOut = out;
}

pfodESP8266BufferedClient::pfodESP8266BufferedClient() {
  client = NULL;
  debugOut = NULL;
  // bufferSize for now always 1460
  sendDelayTime = DEFAULT_SEND_DELAY_TIME;
  sendBufferIdx = 0;
}

pfodESP8266BufferedClient* pfodESP8266BufferedClient::connect(WiFiClient* _client) {
#ifdef DEBUG	
	if (debugOut) {
		debugOut->println("called connect");
  }
#endif // DEBUG		
  client = _client;
  sendBufferIdx = 0;
  return this;
}

void pfodESP8266BufferedClient::stop() {	
  sendAfterDelay();
  if (!client) {
    return;
  }
  client->stop();
  client = 0;
}


uint8_t pfodESP8266BufferedClient::connected() {
	sendAfterDelay();
  if (!client) {
    return 0;
  }
  return  client->connected();
}

size_t pfodESP8266BufferedClient::write(const uint8_t *buf, size_t size) {
	sendAfterDelay();
  if (!client) {
    return 0;
  }
  for (size_t i = 0; i < size; i++) {
    _write(buf[i]); // may block if 1460 buffer fills and client starts sending packet
  }
  return size;
}

size_t pfodESP8266BufferedClient::write(uint8_t c) {
	sendAfterDelay();
  if (!client) {
    return 0;
  }
  return _write(c);
}

size_t pfodESP8266BufferedClient::_write(uint8_t c) {
  if (!client) {
    return 0;
  }
  size_t rtn = 1;
  sendTimerStart = millis();
  sendBuffer[sendBufferIdx++] = c;
  //push UART data to connected client, if buffer full OR send timer times out after data stops coming
  if (sendBufferIdx == DEFAULT_SEND_BUFFER_SIZE) {
#ifdef DEBUG
    if (debugOut != NULL) {
      debugOut->print("buffer full write "); debugOut->print(sendBufferIdx); debugOut->println(" bytes to client");
      debugOut->println(millis());
    }
#endif // DEBUG    
    if (client && client->connected()) {
      // buffer full
      // returns ((size_t)-1) if cannot write in 5 sec
      rtn = client->write((const uint8_t *)sendBuffer, sendBufferIdx); // this call may block if last packet not ACKed yet
#ifdef DEBUG
      if (debugOut != NULL) {
        debugOut->print(millis()); debugOut->println(" after write");
      }
#endif // DEBUG    
    } else {
#ifdef DEBUG
    if (debugOut != NULL) {
      debugOut->print("client not connected do nothing "); 
      debugOut->println(millis());
    }
#endif // DEBUG
      // just throw this data away
    }
    sendBufferIdx = 0;
  }
  delay(0); // yield
  return rtn;
}

void pfodESP8266BufferedClient::sendAfterDelay() {
	if ((!client) || (!sendBufferIdx)) {  // common cases
    return;
  }
  if (((millis() - sendTimerStart) > sendDelayTime)) {
    if (client->connected()) {
 //   if (client->isSendWaiting()) {
 //     // skip this write and keep adding data to buffer
 //   } else {
 //     // ESP not busy and this call will not block	
#ifdef DEBUG
       if (debugOut != NULL) {
         debugOut->print("sendAfterDelay() "); debugOut->print(sendBufferIdx); debugOut->println(" bytes to client");
         debugOut->println(millis());
       }
#endif // DEBUG    
       client->write((const uint8_t *)sendBuffer, sendBufferIdx);
       // client.flush();  // don't flush does not affect write but does clear inbound buffer :-(
#ifdef DEBUG
       if (debugOut != NULL) {
         debugOut->print(millis()); debugOut->println(" after sendAfterDelay() write");
       }
#endif // DEBUG    
       sendBufferIdx = 0;
//    }
    } else {
      // just throw this data away
      sendBufferIdx = 0;
    }
  }	
  delay(0); // yield
}

// force send the buffer block if necessary
void pfodESP8266BufferedClient::forceSend(){
	if ((!client) || (!sendBufferIdx)) {  // common cases
    return;
  }
	if (client->connected()) {
    client->write((const uint8_t *)sendBuffer, sendBufferIdx); // this call may block if last packet not ACKed yet
    sendBufferIdx = 0;
  }
  delay(0);
}


// expect available to ALWAYS called before read() so update timer here
int pfodESP8266BufferedClient::available() {
  sendAfterDelay();
  if (!client) {
		return 0;
	}
  if (!client->connected()) {
    return 0;
  }
  return client->available();
}

int pfodESP8266BufferedClient::read() {
  sendAfterDelay();
  if (!client) {
    return -1;
  }
  int c = client->read();
  delay(0);
  return c;
}

int pfodESP8266BufferedClient::peek() {
  sendAfterDelay();
  if (!client) {
    return -1;
  }
  return client->peek();
}

/** 
  Forces send of any buffered data now. May block if last packet no ACKed yet
*/
void pfodESP8266BufferedClient::flush() {
  forceSend();
  // do not call flush!! for WiFiClient flush() discards unread incoming data;
}
