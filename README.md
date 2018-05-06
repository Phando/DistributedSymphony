# DistributedSymphony
Getting micro controllers to play nice.

***Arduino IDE Setup:***

Download the IDE from: 
[https://www.arduino.cc/en/Main/OldSoftwareReleases]()

***Add the ESP32 Functionality:***
<pre>
	<code>mkdir hardware
	cd hardware 
	mkdir espressif
	cd espressif
	git clone https://github.com/espressif/arduino-esp32.git esp32
</code></pre>

***Add the Salesforce32 Board to the IDE:***

In order to properly use the Salesforce32 (force32) board, you will need to copy several files from ./hardware to ~/Documents/Arduino/hardware. By copying these files you will see the Salesforce32 chip and debug options in the Arduino IDE. The changes also add new pin macros and increase the partition size of the board. 


*Below may be depricated*

Add the following url to the 'Additional Boards Manager URLs' section
*http://arduino.esp8266.com/stable/package_esp8266com_index.json*

**Delete the BLE library**

* Go to ~/Documents/Arduino/hardware/espressif/esp32/libraries
* Delete the directory called BLE

**Import the required libraries**

* Copy ./ESP32/libraries/* to your ~/Documents/Arduino/libraries folder