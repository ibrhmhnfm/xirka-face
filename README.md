**Objectives:** Detects human face and its relative position in the frame
**Achieved Results:**
- **Data Collection:** none (uses available model (MTCNN-based [example](https://github.com/espressif/arduino-esp32/tree/07390157dfd968ad79449af1d98a2406302c8c33/libraries/ESP32/examples/Camera/CameraWebServer)))
- **Framework:** Implemented using Espressif's ESP-DL (Deep Learning) library.
- **Performance Mode:** Set to FAST, prioritizing detection speed over exhaustive accuracy.
- **Input Resolution:** Operates on a QVGA (320x240 pixels) image frame.
- **Minimum Face Size:** Configured to detect faces no smaller than 80x80 pixels.
- **Detection Limit:** The final stage (O-Net (70% confidence)) is set to output only the single best face detected in the frame.

Since this function relies on already existing model, there is no measurable achieved results such as accuracy. When tested, it is more than accurate enough for implementation, although a bit slow.

**Note:** The Github uses esp32 boards manager version v1.0.3. Newer versions will NOT have required libraries for MTCNN Face Recognition. Use Arduino IDE for main branch, a later ImpulseIO configuration will be uploaded to a separate branch.
