# Surfboard Data Recorder by: Moamena Hijazi, Omima Hijazi and Mostafa Hammam.

Hi and welcome to our project "Surfboard Data Recorder".
- This project is part of the ICST Lab at the Technion Institute of Technology, In collaboration with BioMotion Lab - Technion Biomedical Eng.
- Our surfboard data recorder captures acceleration, gyroscope and force data via sensors, providing real-time insights into athlete movements and maneuvers. Elevate training and performance analysis with precision data collection.

# Folder Description:
- ESP32_accelerationSensor: source code for the esp. 
- ESP32_forceSensor: source code for the esp.
- Unit Tests: tests for individual hardware components.
- User_interface.

# Arduino/ESP32 libraries used in this project:
- HX711 - version 0.5.0
- SD - version 1.3.0
- DFRobot_BMI160 - version 1.0.0
- Adafruit Unified Sensor - version 1.1.14
- ArduinoQueue - version 1.2.5
- RTC - version 1.8.0

# Hardware: 
- 2 ESPs.
- BOSCH BMI160 module 6 axis.
- Micro SD card + USB PC adapter.
- Force / load sensor  + HX711.
- Real time clock.
- 3 Leds.
- Button.

# Wiring Diagram: 

![iot_SurfBoard diagram_bb](https://github.com/user-attachments/assets/8e3639b6-3017-4b42-83bf-ef7ab1cd16a8)

# Project Poster: 
![13 Surfboard data recorder IOT_page-0001](https://github.com/user-attachments/assets/b1430919-c4b2-4e2b-9617-c4fe7bcb412b)


# Usage: 
- To activate the recorder please plug both of the cables to a port you will see the red light turning on meaning that the device is powered on.
- Now the system will be waiting for you to click on the start button, this button will be used to start and stop the recording system. Note that in the first activation the device needs to be connected to Wi-fi so please make sure that you have a hotspot/ Wi-fi router ready to be connected to, specify the name of the hotspot and the password in the Arduino code of the BMI (search SSID, password and change them).
- After you press start it will take the device some seconds to boot up and then the recording will start. After the boot up you will see the red light going off and the two green lights turning on, that means that both of the sensors has started recording. If you notice that this is not the case that means that there is a problem, so restart the device and try again.
- After you are done, click the stop button, the device will stop recording and it will be waiting for another start press from you (no need for Wi-fi this time!)
- Open the box carefully and get off the SD card and plug it to your pc. you will see two files starting with Force/BMI indicating to which sensor they belong. If you have started another recording before you get out the SD, don't worry the files wont be overwritten and new files will be created.
- Start the Python program and you will see a window with three buttons indicating the type of the graph you want to view. Please click the wanted graph and choose the file that you have in the SD(if you want the Gyroscope graph/ Acceleration graph choose the BMI file, but for the Force Graph choose the Force file). Please make sure that you are choosing the right files as specified!
- you will see a graph that has a zoom in/out and dragging functionality, to activate those notice that you have a "Toggle Zoom" button that you have to click in order to activate zooming or dragging(red is off and green is on as expected).
- To select an interesting interval please make sure you turn OFF the Toggle Zoom button and click two spots to mark the wanted interval.
- After selecting the wanted interval you will notice a "CSV file" button popping on the bottom right corner, Click it to create a CSV file of the data in the Chosen interval. the file will also contain a timestamp of when the specific data was recorded.

That's All! Enjoy riding those waves and GOODLUCK!
The surfboard Data recorder Group.

# 

![תמונה של WhatsApp‏ 2024-08-25 בשעה 02 42 58_7e8094c3](https://github.com/user-attachments/assets/ceefeaf6-d52c-4968-ad4d-840f3396afd3)

![תמונה של WhatsApp‏ 2024-08-25 בשעה 02 42 58_358e90ca](https://github.com/user-attachments/assets/9d1c2bbe-bb86-49fe-b9b7-62107f0556d8)

