import numpy as np

def npy_to_cpp_array(npy_file, var_name, dtype='float'):
    data = np.load(npy_file)
    if data.ndim == 1:
        # 1D array (e.g., labels)
        cpp_array = f"const {dtype} {var_name}[] = {{\n"
        cpp_array += "    " + ", ".join(map(str, data)) + "\n"
    elif data.ndim == 2:
        # 2D array (e.g., data)
        cpp_array = f"const {dtype} {var_name}[][] = {{\n"
        for row in data:
            cpp_array += "    {" + ", ".join(map(str, row)) + "},\n"
    else:
        raise ValueError("Unsupported array dimensionality")
    cpp_array = cpp_array.rstrip(",\n") + "\n};\n"
    return cpp_array

# Generate the C++ arrays
train_data_cpp = npy_to_cpp_array('train_data.npy', 'train_data', dtype='float')
train_labels_cpp = npy_to_cpp_array('train_labels.npy', 'train_labels', dtype='int')
test_data_cpp = npy_to_cpp_array('test_data.npy', 'test_data', dtype='float')
test_labels_cpp = npy_to_cpp_array('test_labels.npy', 'test_labels', dtype='int')
validation_data_cpp = npy_to_cpp_array('validation_data.npy', 'validation_data', dtype='float')
validation_labels_cpp = npy_to_cpp_array('validation_labels.npy', 'validation_labels', dtype='int')

# Write the data to a header file
with open('data.h', 'w') as f:
    f.write(train_labels_cpp)
    f.write(train_data_cpp)
    f.write(validation_labels_cpp)
    f.write(validation_data_cpp)
    f.write(test_labels_cpp)
    f.write(test_data_cpp)
   