// Required libraries for camera and face detection
#include "esp_camera.h"
#include "fb_gfx.h"
#include "fd_forward.h"

// Define the camera model
// Make sure this matches your camera. Other options include:
// CAMERA_MODEL_WROVER_KIT, CAMERA_MODEL_ESP_EYE, CAMERA_MODEL_M5STACK_PSRAM
#define CAMERA_MODEL_AI_THINKER 
#include "camera_pins.h"

// Configuration for the face detection model
static mtmn_config_t mtmn_config = {0};

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // Camera configuration structure
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; // Capture in JPEG format
  
  // Check for PSRAM and configure frame size accordingly
  // Higher resolution is used initially to allocate larger buffers.
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Initialize the camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  Serial.println("Camera init OK");

  // Get the camera sensor details
  sensor_t * s = esp_camera_sensor_get();
  // Set the frame size to QVGA for faster face detection
  s->set_framesize(s, FRAMESIZE_QVGA);

  // Initialize the face detection model configuration
  mtmn_config.type                   = FAST;
  mtmn_config.min_face               = 80;
  mtmn_config.pyramid                = 0.707;
  mtmn_config.pyramid_times          = 4;
  mtmn_config.p_threshold.score      = 0.6;
  mtmn_config.p_threshold.nms        = 0.7;
  mtmn_config.p_threshold.candidate_number = 20;
  mtmn_config.r_threshold.score      = 0.7;
  mtmn_config.r_threshold.nms        = 0.7;
  mtmn_config.r_threshold.candidate_number = 10;
  mtmn_config.o_threshold.score      = 0.7;
  mtmn_config.o_threshold.nms        = 0.7;
  mtmn_config.o_threshold.candidate_number = 1;

  Serial.println("Face detection models configured.");
  Serial.println("Starting main loop to detect faces...");
}

void loop() {
  camera_fb_t * fb = NULL;

  // 1. Get a frame from the camera
  fb = esp_camera_fb_get();
  if (!fb) {
      Serial.println("Camera capture failed");
      return;
  }

  // 2. Allocate memory for the image matrix (for RGB conversion)
  dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);
  if (!image_matrix) {
      Serial.println("dl_matrix3du_alloc failed");
      esp_camera_fb_return(fb); // Return frame buffer if matrix allocation fails
      return;
  }
  
  // 3. Convert the captured frame to RGB888 format, which is needed for detection
  if (!fmt2rgb888(fb->buf, fb->len, fb->format, image_matrix->item)) {
      Serial.println("Frame conversion to rgb888 failed");
      dl_matrix3du_free(image_matrix); // Free matrix memory
      esp_camera_fb_return(fb);        // Return frame buffer
      return;
  }

  // The frame buffer is no longer needed, so we can return it now
  esp_camera_fb_return(fb);

  // 4. Run the face detection model on the RGB image
  box_array_t *net_boxes = face_detect(image_matrix, &mtmn_config);

  // 5. Process the results if any faces were found
  if (net_boxes) {
      Serial.printf("Found %d face(s)\n", net_boxes->len);
      for (int i = 0; i < net_boxes->len; i++){
          // Get the coordinates of the bounding box for each face
          int x = (int)net_boxes->box[i].box_p[0];
          int y = (int)net_boxes->box[i].box_p[1];
          int w = (int)net_boxes->box[i].box_p[2] - x + 1;
          int h = (int)net_boxes->box[i].box_p[3] - y + 1;

          // Calculate the center point of the face
          int center_x = x + w / 2;
          int center_y = y + h / 2;
          
          // Print the coordinates and size to the Serial Monitor
          Serial.printf("Face %d: Center at (X: %d, Y: %d), Size (W: %d, H: %d)\n", i, center_x, center_y, w, h);
      }
      
      // Free the memory used by the bounding box results
      free(net_boxes->score);
      free(net_boxes->box);
      free(net_boxes->landmark);
      free(net_boxes);
  }

  // 6. Free the memory used by the image matrix
  dl_matrix3du_free(image_matrix);

  // Add a small delay to avoid overwhelming the processor
  delay(500);
}
