#!/bin/bash
# Generate Xcode project for C++ Core
echo "Generating Xcode project for AgenticMVCpipe Core..."
cd AgenticMVCpipe
mkdir -p xcode_build
cd xcode_build
cmake -G Xcode ..

echo "------------------------------------------------"
echo "DONE! You can now:"
echo "1. Open 'AgenticMVCpipe/xcode_build/AgenticMVCpipe.xcodeproj' for C++ debugging."
echo "2. Open 'AgenticMVCpipe/SwiftClient/' in Xcode to run the Native Dashboard."
echo "------------------------------------------------"
