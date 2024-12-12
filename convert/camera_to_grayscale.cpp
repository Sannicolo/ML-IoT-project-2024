#include <TinyMLShield.h>
#include <Arduino.h>

byte image[176 * 144 * 2]; // QCIF: 176x144 x 2 bytes per pixel (RGB565)
float resizedImage[24 * 24];
int bytesPerFrame;

void convertToGrayscale(byte *image) {
  for (int i = 0; i < bytesPerFrame; i += 2)
  {
    uint16_t pixel = (image[i] << 8) | image[i + 1];
    uint8_t r = (pixel >> 11) & 0x1F;
    uint8_t g = (pixel >> 5) & 0x3F;
    uint8_t b = pixel & 0x1F;
    uint8_t gray = (r * 299 + g * 587 + b * 114) / 1000;
    image[i / 2] = gray;
  }
}

void resizeImage() {
  int srcWidth = 176;
  int srcHeight = 144;
  int destWidth = 24;
  int destHeight = 24;
  float xRatio = srcWidth / (float)destWidth;
  float yRatio = srcHeight / (float)destHeight;
  for (int y = 0; y < destHeight; y++)
  {
    for (int x = 0; x < destWidth; x++)
    {
      int srcX = (int)(x * xRatio);
      int srcY = (int)(y * yRatio);
      resizedImage[y * destWidth + x] = image[srcY * srcWidth + srcX] / 255.0;
    }
  }
}

void printFloatArray() {
  Serial.print("[");
  for (int i = 0; i < 50 * 50; i++) {
    Serial.print(resizedImage[i], 3);
    if (i != 50 * 50 - 1) {
      Serial.print(", ");
    }
  }
  Serial.println("]");
}

void setup()
{
  Serial.begin(9600);
  while (!Serial);

  initializeShield();

  if (!Camera.begin(QCIF, RGB565, 1, OV7675))
  {
    Serial.println("Failed to initialize camera");
    while (1);
  }
  bytesPerFrame = Camera.width() * Camera.height() * Camera.bytesPerPixel();

  Serial.println("Press button to take image\n");
}

void loop()
{
  bool clicked = readShieldButton();
  if (clicked) {
    Camera.readFrame(image);
    convertToGrayscale(image);
    resizeImage();
    printFloatArray();
    return;
  }
}
