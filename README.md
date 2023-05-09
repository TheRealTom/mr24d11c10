# mr24d11c10

The radar is distributed by the [SeeedStudio](https://www.seeedstudio.com/).
The project is a part of a thesis that aims on the using of mmWave radar technology to detect a person in a room that is stationary.

The `example` folder shows the implementation of the config.yaml file for the ESPHome for the ESP32 and the ESP8266. The project was tested within the local Home Assistant network and the .

The `pre-defined-config` is a configuration for the ESPHome contribution or for adding external component.



## GUIDE
### Prerequisites
- the running Home Assistance instance
- Installed ESPHome and file management Add-on
- Internet connection

### How to implement the radar device
- Add the `radar.h/.cpp` and `mr24d11c10.h/.cpp` in the file management Add-on.
- In ESPHome add new device, the ESPHome will guide you.
- In configuration file fill the appropriate ESP board that you use.
- Add the `include` parameter with the path to the component `mr24d11c10.h` file 
- Add the configuration from the repository `config.yaml` file.
- Optional: Define a static IP address for the device.
- Install the new configuration.
- In Home Assistant Integration add new integration based on hostname/IP address of the device.
- Now you can add the device GUI to your dashboard


Feel free to contribute. The project is published under the MIT License.
