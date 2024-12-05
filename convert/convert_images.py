import cv2
import numpy as np
import os
import random

def convert_images_to_arrays(image_dir, label, img_size=(5, 15)):
    print(len(os.listdir(image_dir)))
    data = []
    labels = []
    for img_name in os.listdir(image_dir):
        img_path = os.path.join(image_dir, img_name)
        img = cv2.imread(img_path, cv2.IMREAD_GRAYSCALE)
        img = cv2.resize(img, img_size)
        img_array = [round(1-img/255, 3) for img in img.flatten().tolist()]
        data.append(img_array)
        labels.append(label)
    return data, labels

def split_data(data, labels, train_size, val_size, test_size):
    combined = list(zip(data, labels))
    random.shuffle(combined)
    data, labels = zip(*combined)
    train_data = data[:train_size]
    train_labels = labels[:train_size]
    val_data = data[train_size:train_size + val_size]
    val_labels = labels[train_size:train_size + val_size]
    test_data = data[train_size + val_size:train_size + val_size + test_size]
    test_labels = labels[train_size + val_size:train_size + val_size + test_size]
    return (train_data, train_labels), (val_data, val_labels), (test_data, test_labels)

# Load and split banana data
banana_data, banana_labels = convert_images_to_arrays('/Users/kaansirin/Downloads/dataset 2/banana-new', label=0)
(train_banana_data, train_banana_labels), (val_banana_data, val_banana_labels), (test_banana_data, test_banana_labels) = split_data(
    banana_data, banana_labels, train_size=49, val_size=15, test_size=6
)

# Load and split tomato data
tomato_data, tomato_labels = convert_images_to_arrays('/Users/kaansirin/Downloads/dataset 2/tomato-new', label=1)
(train_tomato_data, train_tomato_labels), (val_tomato_data, val_tomato_labels), (test_tomato_data, test_tomato_labels) = split_data(
    tomato_data, tomato_labels, train_size=49, val_size=15, test_size=6
)

# Combine training, validation, and test data
train_data = list(train_banana_data) + list(train_tomato_data)
train_labels = list(train_banana_labels) + list(train_tomato_labels)

validation_data = list(val_banana_data) + list(val_tomato_data)
validation_labels = list(val_banana_labels) + list(val_tomato_labels)

test_data = list(test_banana_data) + list(test_tomato_data)
test_labels = list(test_banana_labels) + list(test_tomato_labels)

# Save the data and labels to files
np.save('train_data.npy', train_data)
np.save('train_labels.npy', train_labels)
np.save('validation_data.npy', validation_data)
np.save('validation_labels.npy', validation_labels)
np.save('test_data.npy', test_data)
np.save('test_labels.npy', test_labels)

print("Data saved successfully!")