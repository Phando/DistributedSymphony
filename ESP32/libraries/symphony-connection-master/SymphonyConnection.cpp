/*
Symphony Server connectivity manager for project-518

Copyright 2018 Pete Richards 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

 */

// #define DEBUGGING

#include "SymphonyConnection.h"

static void Work(void * instance_pointer) {
    SymphonyConnection * self = static_cast<SymphonyConnection*>(instance_pointer);
    self->workConnection();
}

bool SymphonyConnection::connect() {
    if (connected) {
        return false;
    }
    
    configTime(GMT_OFFSET_SECONDS, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
    
#ifndef SYMPHONY_USE_LOCAL_SERVER
    socketClient.setCACert(SYMPHONY_DIGICERT_ROOT_CA);
#endif
    
    uint64_t chipid = ESP.getEfuseMac();
    sprintf(deviceId, "%04X%08X", (uint16_t)(chipid>>32), (uint32_t)chipid);

    Serial.print("[SYMPHONY] Device ID: ");
    Serial.println(deviceId);
    
#ifdef DEBUGGING
    Serial.println("[SYMPHONY] Starting connection to server...");
#endif
    if (!socketClient.connect(SYMPHONY_API_HOST, SYMPHONY_API_PORT)) {
#ifdef DEBUGGING
        Serial.println("[SYMPHONY] Connection failed!");
#endif
        return false;
    } else {
#ifdef DEBUGGING
        Serial.println("[SYMPHONY] Connection successful.");
#endif
        wsClient.path = SYMPHONY_API_PATH;
        wsClient.host = SYMPHONY_API_HOST;
        if (wsClient.handshake(socketClient)) {
#ifdef DEBUGGING
            Serial.println("[SYMPHONY] Websocket Handshake successful");
#endif
        } else {
#ifdef DEBUGGING
            Serial.println("[SYMPHONY] Websocket Handshake Failed");
#endif
            return false;
        }
        if (socketClient.connected()) {
            wsClient.sendData(String("DEVICEID:") + String(deviceId));
            serverTask.attach_ms(SYMPHONY_WORK_PERIOD, Work, (void*)this);
        }
    }
    connected = true;
    return connected;
}

bool SymphonyConnection::onMessage(message_listener listener) {
    if (listener_count >= SYMPHONY_MAX_CALLBACKS) {
#ifdef DEBUGGING
        Serial.println("Reached maximum number of callbacks possible");
#endif
        return false;
    }
    listeners[listener_count] = listener;
    listener_count++;
    return true;
}

void SymphonyConnection::workConnection() {
    String data;
    if (socketClient.connected()) {
        wsClient.getData(data);
        if (data.length() > 0) {
#ifdef DEBUGGING
            Serial.print("[SYMPHONY] Received data: ");
            Serial.println(data);
#endif
            if (data == "PING") {
                struct timeval tv = { .tv_sec = 0, .tv_usec = 0 };
                gettimeofday(&tv, NULL);
                char timestampMilliseconds[23];
                sprintf(timestampMilliseconds, "PONG:%d.%06d", tv.tv_sec, tv.tv_usec);
                String pongString = String(timestampMilliseconds);
                wsClient.sendData(pongString);
#ifdef DEBUGGING
                Serial.print("[SYMPHONY] Sending: ");
                Serial.println(pongString);
#endif
            } else {
                // Notify listeners
                for (int i=0; i < listener_count; i++) {
                    listeners[i](data);
                }
            }
        }
    }
}