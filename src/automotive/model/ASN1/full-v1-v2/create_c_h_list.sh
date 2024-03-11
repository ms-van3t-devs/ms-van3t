#!/bin/bash

# Create a file with the list of full paths of .c files in alphabetical order
find . -name "*.c" -type f | sort > c_files.txt

# Create a file with the list of full paths of .h files in alphabetical order
find . -name "*.h" -type f | sort > h_files.txt

echo "Created c_files.txt with the list of full paths of .c files (alphabetical order)"
echo "Created h_files.txt with the list of full paths of .h files (alphabetical order)"