# OTC Project

![OTC_LOGO](https://raw.githubusercontent.com/JuraszekL/OTC_Project/master/Resources/OTC_Logo.png)

OTC or just Online Table Clock is, as it says, my concept of table clock that is able to get time/weather related data from the internet.

# Table of content
1. [**In nutshell**](#in-nutshell)
2. [**Motivation**](#motivation)
3. [**Functionality**](#functionality)
4. [**Milestones**](#milestones)
5. [**Detailed description**](#detailed-description)
   * [**Hardware**](#hardware)
   * [**Configuration**](#configuration)
   * [**Code structure**](#code-structure)
   * [**Tasks**](#tasks)
   * [**Most important functions**](#most-important-functions)

6.[**List of external tools/resources**](#List-of-external-tools/resources) 

# In nutshell

OTC project is based on	[ESP32 Terminal](https://www.elecrow.com/esp-terminal-with-esp32-3-5-inch-parallel-480x320-tft-capacitive-touch-display-rgb-by-chip-ili9488.html) module. The module is equipped with ESP32S3 MCU with 16MB Flash and 8MB RAM memory, 480x320 LCD screen with capacitive touch and USB-C port for serial programming. The firmware has been written in pure C, and is based on [FreeRTOS](https://www.freertos.org/) port for Espressif microcontrolers. The main tools used for this project are:
- [ESP-IDF v5.0.3](https://github.com/espressif/esp-idf/tree/release/v5.0)
- [Eclipse IDE, Version: 2022-09](https://www.eclipse.org/downloads/packages/release/2022-09/r) 
- [LVGL v8.3.7](https://github.com/lvgl/lvgl/tree/release/v8.3)
- [SquareLine Studio 1.3.1](https://squareline.io/downloads)

To obtain weather and location data, is uses free [Weather API](https://www.weatherapi.com/), and [cJSON](https://github.com/DaveGamble/cJSON) library to parse the data. Encryption and decryption of stored wifi passwords are done with Espressif's port of [Mbed-TLS](https://github.com/Mbed-TLS/mbedtls) library. 

![OTC Main Screen](https://raw.githubusercontent.com/JuraszekL/OTC_Project/master/Resources/OTC_MainScreenPhoto.png) ![OTC Weather Screen](https://raw.githubusercontent.com/JuraszekL/OTC_Project/master/Resources/OTC_WeatherScreenPhoto.png)
![OTC Wifi Screen](https://raw.githubusercontent.com/JuraszekL/OTC_Project/master/Resources/OTC_WifiScreenPhoto.png)

# Motivation

This project is for learning purposes only. Feel free to use it.

# Functionality

Current version - v0.0.8 (pre-release)

In this version OTC is able to:
- search for available wifi access points and represent the results on the screen. Each one AP within the list shows SSID name, signal strength and informs that AP is protected or not.
- connect with chosen AP, show details about connected AP
- ask user to enter AP password, save encrypted password, search and decrypt saved password if exists
- get current time, timezone and basic weather data for current location
- get detailed weather data for current location

# Milestones

The project is still under development. Next milestones are:
- add setup screen
- add theme change
- add stored passwords management
- v1.0.0 - first release
- add backlight and buzzer management
- add alarm function
- add RTC and low-power support
- add Li-Ion batter management
- add OTA firmware update

# Detailed description

## Hardware

Although The ESP32 Terminal module is ready to use dev board, one modification has to be done. Pin 4 of touch controller (PENI) generates an interrupt when touch is pressed; because of insufficient number of pins, that pin is disconnected with R35 jumper from GPIO0, so GPIO0 is used only to enter BOOT mode with "BOOT" switch. Observations showed that both functions (entering boot mode and recieveing touch interrupts) can be covered by GPIO0 without any unexpected behavior, so R35 has been jointed.

## Configuration

Internal flash memory is configured as Quad-SPI, and works with 80MHz. Internal RAM memory is configured as Octal-SPI and works with 80MHz as well. The .bss and Wifi/LwIP resources can be placed into external RAM. Instruction cache is set to 32kB, data cache to 64kB.
Internal Wifi task is pinned to core 1.

## Code structure

Main code structure:
![CODE_MAIN](https://raw.githubusercontent.com/JuraszekL/OTC_Project/master/Resources/OTC_main.png)

UI code structure:
![CODE_UI](https://raw.githubusercontent.com/JuraszekL/OTC_Project/master/Resources/OTC_UI.png)

Legend:
![CODE_UI](https://raw.githubusercontent.com/JuraszekL/OTC_Project/master/Resources/OTC_Legend.png)

## Tasks

The folowing tasks are created in this project:

- **app_main**

    The default task created by FreeRTOS. Here everything starts. It is used to initialize hardware, cJSON, and create all other tasks, then is deleted by itself.

- **Wifi_Task**

  - initialize Wifi and NetIF resources
  - manage WiFi events
  - run scan and connect to access points
  - report events to UI

- **SPIFFS_Task**

  - mount a filesystem to a partition within internal memory
  - create, restore, make backup  and check correctness of wifi password files
  - read AES 256-bit key stored in eFuse
  - encrypt/decrypt recieved password with AES CBC method
  - store encrypted password with all data needed do decrypt (as JSON object)

- **OnlineRequests_Task**

  - get current time from SNTP server
  - get timezone, current and forecast weather for current IP from HTTP
  - parse recieved data from HTTP (as JSON)
  - send the data to UI

- **Clock_Task**

  - update time to UI every one minute
  - monitor clock synchronisation status and report to UI
  - manage synchornisation requests

- **UI_Task**

  - initialize all elements within User Interface
  - manage UI events
  - manage UI elements in runtime

- **Display_Task**

  - initialize LCD driver and LVGL library
  - perform draw operations if required

- **TouchPad_Task**

  - initialize touchpad driver
  - read last touchpad data if interrupt occured

- **SDCard_Task**

  - initialize SDCARD peripherials and delete itself when it's done

## Most important functions

**Clock set/synchronize**

When the system date is before 2020 that means, the clock is not set properly. In this case, the time on the main screen is set to "--:--", and no other information are showed. *Clock_Task* sends *ONLINEREQ_TIME_UPDATE* request every minute. When wifi is connected, request is processed by *OnlineRequests_Task* - it starts then SNTP service. If the time was updated, *Clock_Task* is informed, then it sets clock status as "not_sync" and sends *ONLINEREQ_BASIC_UPDATE* request. Only if status of the clock differs from "not set", it sends it's value to the main screen. Now, even if the clock is still not synchronized, the time is displayed on the main screen. The missing part of "synchronization" is the timezone. If no any timezone has been set yet, the default GMT0 value is used. The proper timezone is obtained from HTTP service, together with current weather data. When HTTP request is processed, then parsed, the *Clock_Task* receives the new timezone. Now the clock has "sync" status, *Clock_Task* informs UI about time/date change, and about it's new status. The clock status is represented by the sync icon at the top left corner of main screen (two rounded arrows). There are two ways to set the clock status to "not sync" now - wifi is disconnected or no response from HTTP was obtained. Synchronization of the clock is performed once every hour, on time defined by *BASIC_DATA_RESFRESH_MINUTE* macro (together with update of timezone/current weather).

The video below shows the situations mentioned above. When OTC starts, the clock is not set. After connecting with access point "ABCD", the clock is set by SNTP service, then finally synchronized with timezone received from HTTP.

[![Video link](https://img.youtube.com/vi/nlv_j90KZcM/0.jpg)](https://www.youtube.com/watch?v=nlv_j90KZcM "Click to viev")

**Display current/forecast weather**

The current weather data are displayed on the main screen, below current time and date. This area contains numeric values on the right side, like temperature, pressure and humidity, and big icon on the left side that represents current weather in general. Data are obtained from HTTP together with timezone, and are updated once every hour. Location is based on OTC IP - function provided by weatherapi.com API. To see forecast weather data (only current day is supported now) user needs to click the weather icon on the main screen. UI then sends *ONLINEREQ_DETAILED_UPDATE* request, and waits for the event. The request is processed by *OnlineRequests_Task*, it recieves JSON 0from HTTP, parse it, and reports event to UI. Finally, UI sets all values and swipes the weather screen on top. User can swipe back the screen with back button. Every time when use clicks the main screen weather icon, new HTTP request is sent, and weather data are updated.

The weather screen contains:
- current location, city and country name are displayed at the top left corner
- weather icon that represents the weather during the day in general. It is located at the top right corner
- average temperature located at the middle of the screen. It is composed of 270deg arc and the number inside it. The color of the arc is changed according to the temperature.
- min/max temperature located at the bottom of arc. It shows minimum and maximum temperature during the day
- four icons located in the bottom half of the screen. They shows the sunrise and sunset time, maximum and average wind speed, total precipitation and percentage chance of rain and snow

The video below shows the situation when user wants to see forecast weather for today.

[![Video link](https://img.youtube.com/vi/8ntkoaaQR5k/0.jpg)](https://www.youtube.com/watch?v=8ntkoaaQR5k "Click to viev")

**Search and list for available Wifi networks**

Regardless of wifi connection status, user is able to scan and display avalible wifi access points. To perform the scan, user needs to click the wifi button on the main screen. When UI recieves this event, it deletes all elements within the list (if any were add before), then swipes the wifi screen on top, and sends "StartScan" request to *Wifi_Task*. When scan is done, internal IDF API calls *WIFI_EVENT_SCAN_DONE* event which is processed by *Wifi_Task* as well. All access points that were found, are sent one by one to *UI_Task* wit information about SSID (Wifi network name), RSSI (signal strength, in dBm) and either is the network protected or no. The *UI_Task* creates new element within the list, sets signal strength icon, and protection icon in according to received data.

The list is located on the wifi screen, below access point details. The list is scrollable when number of AP's exceeds the maximum number that fits to the screen. When AP name doesn't fit to the screen because it is too long, it will be scrolled horizontally from one side to the other.

The video below shows user request to scan for available wifi network.

[![Video link](https://img.youtube.com/vi/uG8Tw_aezV8/0.jpg)](https://www.youtube.com/watch?v=uG8Tw_aezV8 "Click to viev")

**Connect to selected Wifi network with stored password**

After one of available networks was clicked, the UI task sends *SPIFFS_GetPass* request to obtain the password that was saved before. The SPIFFS task reads an internal file where all passwords are saved, then uses cJSON library to parse the file.

If the file contains an object with the name of selected Wifi network's SSID, the task reads all data fields from the object like:
- *iv*,  input vector, 16-byte random value that is used to encrypt/decrypt a password, stored as string
- *pass*, encrypted password, it will be decrypted and used to connect with selected network, stored as string
- *pass_len*, length of decrypted password, stored as integer. This value is required if password's length differs from multiplications of number 16 - in this case original password needs to be padded with '0' characters. After decryption is done, the value is used to cut off those characters.

The encryption/decryption key is stored in *EFUSE_BLK_KEY0*, the eFuse was burned out manually with *espefuse.py* tool. The task reads the value of KEY0 efuse and uses *Mbed-TLS* library to decrypt the password. The encrypted password, together with SSID is then used to send *Wifi_ReportPass* request. This request, finally, leads to the connect attempt performed by Wifi Task.

The video below shows connection to the *ABCD* Wifi, the password for this AP was saved before:

[![Video link](https://img.youtube.com/vi/1o4b9FymgRU/0.jpg)](https://www.youtube.com/watch?v=1o4b9FymgRU "Click to viev")

**Connect to selected Wifi network with unknown password**

As mentioned above, the SPIFFS task search for saved password in the internal file. If the file doesn't contain the password, SPIFFS Task sends *UI_EVT_WIFI_GET_PASS* request to the UI Task.

This is the moment when user need to input the password for selected Wifi network. To make it possible, UI Task creates *wifi_popup* - the panel displayed at the top layer of the Wifi Screen. The panel contains:
- text area where user password is written
- two checkboxes and two buttons
- a keyboard, sliding up and down

To show the keyboard user needs to click on the password area, thereby, to hide the keyboard, user needs to click everywhere else. Checkbox *Hide* allows to replace the password with '*' characters, checkbox *Save* allows to save the password for the future use.

After clicking the right button (OK), UI task sends *Wifi_Connect* request, then Wifi Task performs connect attempt. If requested wifi was connected, and user allowed to save the password, the Wifi Task sends *SPIFFS_SavePass* request. This request is again processed by SPIFFS task. The internal file is opened, then parsed with cJSON library, then new JSON object is created. The name of the object is an SSID of connected network, and three data fields are created within the object:
- *iv*,  input vector, 16-byte random value that is used to encrypt/decrypt a password, stored as string
- *pass*, the encrypted password will be there, stored as string
- *pass_len*, length of decrypted password, stored as integer. This value is required if password's length differs from multiplications of number 16 - in this case original password needs to be padded with '0' characters. After decryption is done, the value is used to cut off those characters.

The task generates random value for input vector, reads KEY0 efuse (check details above) and performs password encryption with *Mbed-TLS* library. After that, stores all values to the JSON object, and saves modified file. Now the new password can be decrypted when user requests connect to this Wifi again.

The video below shows connection to the "ABCDE" Wifi. The password was never stored before, so user have to put it manually.

[![Video link](https://img.youtube.com/vi/nqn2JDltF1c/0.jpg)](https://www.youtube.com/watch?v=nqn2JDltF1c "Click to viev")

**Connecting status and AP info**

There are two indicators of current Wifi connection status:
- small icon in the top left corner of the Main screen. It indicates only if OTC is connected to any access point (and has obtained IP address)
- network details at the top of Wifi screen.

Information provided by the network details are:
  - SSID (name) of connected Wifi access point
  - RSSI (signal strength, in dBm) of connected AP
  - MAC, IP address and authentication method of connected AP

The RSSI value is shown by a number value that is placed inside an interactive Arc; the color of the Arc is set in according to signal strength (from green at -30dBm to red at -120dBm)

The pictures below shows some examples of different access points:

![OTC High RSSI](https://raw.githubusercontent.com/JuraszekL/OTC_Project/master/Resources/OTC_HighRSSI.png) ![OTC Medium RSSI](https://raw.githubusercontent.com/JuraszekL/OTC_Project/master/Resources/OTC_MedRSSI.png) ![OTC Low RSSI](https://raw.githubusercontent.com/JuraszekL/OTC_Project/master/Resources/OTC_LowRSSI.png)

# List of external tools/resources

Besides the components mentioned above, the OTC project was developed with use:
- [**Gluphr Studio**](https://www.glyphrstudio.com/online/)
- [**Font Editor Online**](https://tophix.com/assets/js/fonteditor/index-en.html)
- [**Freepik**](https://www.freepik.com/)
- [**Converio**](https://convertio.co/)
- [**draw.io**](https://app.diagrams.net/)
- [**nayarsystems/posix_tz_db**](https://github.com/nayarsystems/posix_tz_db/tree/master)
- [**Wifi Icons by Callum Smith**](https://www.vecteezy.com/vector-art/18881979-set-of-erorr-secure-and-no-wifi-signs)
- [**Varino font by Arterfak Project**](https://www.dafont.com/varino.font)
- [**Digital-7 font by Style-7**](https://www.1001fonts.com/digital-7-font.html)


