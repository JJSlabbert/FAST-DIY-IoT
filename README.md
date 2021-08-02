# FAST-DIY-IoT
Many IoT projects have a few requirement in common. If you have a basic template to satisfy all this requirements, development can speed up. Although most IoT projects use a fast variety of sensors and actuators to solve problems, the common requirements include:  
1) A easy why to connect to the internet. If you have a WiFi device, it needs to get your WiFi hotspot details to connect to the internet. 
2) You need to have a user-friendly, interactive method of communicating with your device. Reading sensors values, monitor and management of actuators (motors, speakers, heaters, air conditioners etc) must be easy, using good graphical user interface. Cloud based dashboards are useful. 
3) Updating of firmware must be hazel free.


This project will use strategies to solve the three basic requirements mentioned previously

Easy internet connection: https://github.com/tzapu/WiFiManager. WiFi manager use a user-friendly web interface to collect your WiFi credentials and log onto your WiFi access point. Custom parameters like MQTT details can also be saved with WiFi manager

Cayenne is the free version of mydevices and is a cloud based web dashboard. It collects data from your sensors, display the data on graphs and sent instruction back to your microcontroller to maybe switch on a lite, open your gate etc. Cayenne can also use rules (like switch on a lite when it is dark). An event scheduler is also included in cayenne. Caynne use MQTT

ArduinoOTA is a standard arduino library enabling you to update firmware of devices on your LAN.

This project was tested with an ESP32 lolin wemos

See https://www.instructables.com/FAST-DIY-IoT/ for building instructions
