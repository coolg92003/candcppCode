import ctypes

# Load the shared library
lib = ctypes.CDLL('./libc2python.so')  # Use 'example.dll' on Windows

# Call the C function
name = "Hello Cliff Chen!"

# Define the argument types for the function
lib.print_any_char.argtypes = [ctypes.c_char_p]  # Pass a pointer to a C string

# Prepare the string as a byte string
lib.print_any_char(name.encode('utf-8'))  # Ensure the string is passed as bytes

#lib.print_any_char(name)

