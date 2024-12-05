import numpy as np
import matplotlib.pyplot as plt

def visualize_image(data_file, labels_file, index, img_size=(28, 28)):
    """
    Visualize a single image from the dataset.

    Parameters:
        data_file (str): Path to the numpy file containing the image data.
        labels_file (str): Path to the numpy file containing the labels.
        index (int): Index of the image to visualize.
        img_size (tuple): Size of the image (height, width).
    """
    # Load the data and labels
    data = np.load(data_file)
    labels = np.load(labels_file)

    # Get the image and label
    image = np.array(data[index]).reshape(img_size)
    label = labels[index]

    # Plot the image
    plt.imshow(image, cmap='gray')
    plt.title(f"Label: {label}")
    plt.axis('off')
    plt.show()

# Visualize an image from the training dataset
visualize_image('train_data.npy', 'train_labels.npy', index=16, img_size=(5, 15))