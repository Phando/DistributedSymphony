# DistributedSymphony
Getting micro controllers to play nice.

***Arduino IDE Setup:***

Download the IDE from: 
[https://www.arduino.cc/en/Main/OldSoftwareReleases]()

Add the following url to the 'Additional Boards Manager URLs' section  
*http://arduino.esp8266.com/stable/package_esp8266com_index.json*

**Delete the BLE library**

* Go to ~/Documents/Arduino/hardware/espressif/esp32/libraries
* Delete the directory called BLE

**Import the required libraries**

* Copy ./ESP32/libraries/* to your ~/Documents/Arduino/libraries folder