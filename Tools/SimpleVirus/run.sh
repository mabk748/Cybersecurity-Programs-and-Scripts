#!/bin/bash

# Check if a folder path is provided
if [ -z "$1" ]; then
    echo -e "Usage: ./run.sh Path/to/folder\n"
    exit 1
fi

# Get the folder path from the first argument
TARGET_FOLDER="$1"

# Ensure the folder exists
if [ ! -d "$TARGET_FOLDER" ]; then
    echo -e "The specified folder does not exist: $TARGET_FOLDER\n"
    exit 1
fi

# Check if maliciousCode.txt exists
if [ ! -f "maliciousCode.txt" ]; then
    echo -e "The file 'maliciousCode.txt' is missing.\n"
    exit 1
fi

# Compile the C program
gcc main.c -o main
if [ $? -ne 0 ]; then
    echo -e "Compilation failed.\n"
    exit 1
fi

# Move the compiled binary and maliciousCode.txt to the target folder
cp maliciousCode.txt "$TARGET_FOLDER/"
mv main "$TARGET_FOLDER/"
if [ $? -ne 0 ]; then
    echo -e "Failed to move the required files to the target folder.\n"
    exit 1
fi

# Execute the binary in the target folder
cd "$TARGET_FOLDER" || exit
./main "maliciousCode.txt"

# Clean up: Delete the binary and maliciousCode.txt after execution
rm -f main maliciousCode.txt
if [ $? -eq 0 ]; then
    echo -e "Cleaned up: Removed binary and maliciousCode.txt.\n"
else
    echo -e "Failed to clean up files.\n"
    exit 1
fi

# Return to the original directory
cd - > /dev/null 2>&1

echo -e "Execution complete.\n"