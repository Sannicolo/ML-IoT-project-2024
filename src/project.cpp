#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <TinyMLShield.h>
#include "BLE_Communication.h"

// NN parameters, set these yourself!
#define LEARNING_RATE 0.01 // The learning rate used to train your network
#define EPOCH 50           // The maximum number of epochs
#define DATA_TYPE_FLOAT    // The data type used: Set this to DATA_TYPE_DOUBLE for higher precision. However, it is better to keep this Float if you want to submit the result via BT
#define EPOCH_RUN 9        // epochs to run during local training
extern const int first_layer_input_cnt;
extern const int classes_cnt;

// Camera parameters and image processing
byte image[160 * 120 * 2]; // QCIF: 176x144 x 2 bytes per pixel (RGB565)
float resizedImage[24 * 24];
int bytesPerFrame;

#define CENTRAL_OR_PERIPHERAL 0 // 0 for central, 1 for peripheral

// You define your network in NN_def
// Right now, the network consists of three layers:
// 1. An input layer with the size of your input as defined in the variable first_layer_input_cnt in cnn_data.h
// 2. A hidden layer with 20 nodes
// 3. An output layer with as many classes as you defined in the variable classes_cnt in cnn_data.h
static const unsigned int NN_def[] = {first_layer_input_cnt, 10, classes_cnt};
#include "data-more-test.h" // The data, labels and the sizes of all objects are stored here
#include "NN_functions.h"   // All NN functions are stored here

int iter_cnt = 0; // This keeps track of the number of epochs you've trained on the Arduino
#define DEBUG 0   // This prints the weights of your network in case you want to do debugging (set to 1 if you want to see that)

// Converts the RGB565 image to grayscale
void convertToGrayscale(byte *image)
{
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

// Resizes the image to desired size (24x24)
void resizeImage()
{
  int srcWidth = 160;
  int srcHeight = 120;
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

// Prints the image array
void printFloatArray()
{
  Serial.print("[");
  for (int i = 0; i < 24 * 24; i++)
  {
    Serial.print(resizedImage[i], 3);
    if (i != 24 * 24 - 1)
    {
      Serial.print(", ");
    }
  }
  Serial.println("]");
}

// This function contains your training loop
void do_training()
{

  // Print the epoch number
  Serial.print("Epoch count (training count): ");
  Serial.print(++iter_cnt);
  Serial.println();

  // reordering the index for more randomness and faster learning
  shuffleIndx();

  // starting forward + Backward propagation
  for (int j = 0; j < numTrainData; j++)
  {
    generateTrainVectors(j);
    forwardProp();
    backwardProp();
  }

  Serial.println("Accuracy after local training:");
  printAccuracy();
}

void classifyImage()
{
  for (int i = 0; i < IN_VEC_SIZE; i++)
  {
    input[i] = resizedImage[i];
  }

  forwardProp();
  int maxIndx = 0;

  if (y[0] > y[1])
  {
    maxIndx = 0;
  }
  else
  {
    maxIndx = 1;
  }

  Serial.print("Classified class: "); // 0 = banana, 1 = tomato
  Serial.println(maxIndx);
}

void setup()
{
  // put your setup code here, to run once:

  // Initialize random seed
  srand(0);

  Serial.begin(9600);

  while (!Serial)
    ;

  // Initialize the TinyML Shield
  initializeShield();

  // Calculate how many weights and biases we're training on the device.
  int weights_bias_cnt = calcTotalWeightsBias();

  Serial.print("The total number of weights and bias used for on-device training on Arduino: ");
  Serial.println(weights_bias_cnt);

  // Allocate common weight vector, and pass to setupNN
  DATA_TYPE *WeightBiasPtrOriginal = (DATA_TYPE *)calloc(weights_bias_cnt, sizeof(DATA_TYPE));

  setupNN(WeightBiasPtrOriginal); // CREATES THE NETWORK BASED ON NN_def[]
  Serial.println("The accuracy before training");
  printAccuracy();

  // Serial.println("Use the on-shield button to start and stop the loop code ");
  Serial.println("Training the network locally with: " + String(EPOCH_RUN) + " epochs");

  for (int epoch = 0; epoch < EPOCH_RUN; epoch++)
  {
    do_training();
  }

  if (!Camera.begin(QQVGA, RGB565, 1, OV7675))
  {
    // QCIF, QQVGA
    Serial.println("Failed to initialize camera");
    while (1)
      ;
  }
  bytesPerFrame = Camera.width() * Camera.height() * Camera.bytesPerPixel();

  Serial.println(" "); 
  Serial.println("Starting Federated Learning between two nodes (Central and Peripheral)"); 

  if (CENTRAL_OR_PERIPHERAL == 0)
  {
    // Central
    Serial.println("Central Device");
    Serial.println("Packing weights and biases for BLE transfer...");
    packUnpackVector(0);
    Serial.println("Packing completed. Starting BLE transfer...");
    BLECentralSetup(WeightBiasPtr);
    Serial.println("BLE transfer completed.");

    Serial.println("Swapping role to Peripheral Device...");
    delay(5000);

    Serial.println("Waiting for BLE transfer...");
    BLEPeripheralSetup(WeightBiasPtr);
    Serial.println("BLE transfer completed. Unpacking values...");
    packUnpackVector(1); // UNPACK
    Serial.println("Unpacking completed. Printing updated accuracy:");
    printAccuracy();

  }
  else
  {
    // Peripheral
    Serial.println("Peripheral Device");
    Serial.println("Waiting for BLE transfer...");
    BLEPeripheralSetup(WeightBiasPtr);
    Serial.println("BLE transfer completed. Unpacking values...");
    packUnpackVector(2); // AVERAGE, set 1 for just UNPACK
    Serial.println("Unpacking completed. Printing updated accuracy:");
    printAccuracy();

    Serial.println("Swapping role to Central Device...");
    delay(5000);
    
    Serial.println("Packing and sending weights and biases for BLE transfer...");
    packUnpackVector(0);
    BLECentralSetup(WeightBiasPtr);
    Serial.println("BLE transfer completed.");

  }

  Serial.println("Press the button to take in image");
}

void loop()
{

  bool clicked = readShieldButton();

  if (clicked)
  {
    Serial.print("Button clicked\n");
    Camera.readFrame(image);
    convertToGrayscale(image);
    Serial.print("grayscale");
    resizeImage();
    Serial.print("after resize");
    printFloatArray();
    classifyImage();
  }
}