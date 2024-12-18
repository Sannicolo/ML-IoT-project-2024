import numpy as np
import matplotlib.pyplot as plt
import serial

# Configure the serial port
ser = serial.Serial('/dev/cu.usbmodem1101', 9600)  # Update with your serial port and baud rate

def read_serial_data():
    data = []
    classified_class = None
    confidence = None
    print("Reading data...")    
    while True:
        line = ser.readline().decode('utf-8').strip()
        print(line)
        if line.startswith("Button clicked"):
            continue
        elif line.startswith("Printing image..."):
            data = list(map(float, line.split('[')[1].split(']')[0].split(',')))
            print(data)
        elif line.startswith("Classified class"):
            classified_class = int(line.split()[-1])
        elif line.startswith("Condifence:"):
            confidence = float(line.split()[-1])
            break
    return data, classified_class, confidence

plt.ion()  # Turn on interactive mode
fig, ax = plt.subplots()

while True:
    # Read data from the serial port
    data, classified_class, confidence = read_serial_data()

    # Reshape and visualize the data
    image = np.array(data).reshape((24, 24))
    ax.clear()
    ax.imshow(image, cmap='gray')
    ax.axis('off')

    # Add label
    label = f"Prediction: Banana (Confidence: {confidence:.2f})" if classified_class == 0 else f"Prediction: Tomato (Confidence: {confidence:.2f})"
    ax.set_title(label)

    plt.draw()
    plt.pause(0.1)  # Pause to allow the plot to update