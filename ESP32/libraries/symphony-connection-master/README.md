# SymphonyConnection

Library for ESP32 that manages a connection to the Distributed Symphony server.

## Installation

Like any arduino library, install this in your libraries folder.  default on mac is ``~/Documents/Arduino/libraries`

This also depends on the ESP32-Websocket library which can be found here:
https://github.com/larkin/ESP32-Websocket

Once installed, this will add an arduino example sketch called "SymphonyConnection_MessageHandler".

## Usage

1. Establish a wifi connection.
2. call connection.connect()
3. attach message handlers with connection.onMessage().

If you're using the example sketch, it will log messages to serial.  Test sending a message to your device with:

```
curl -XPOST "https://project-518.herokuapp.com/devices/<device_id>/command/Hello"
```

## TODO

* [ ] Better handling of network disconnects / etc.
* [ ] Profit?

https://github.com/larkin/symphony-connection/invitations