#!/bin/bash

# Skrip untuk mengotomatiskan pengujian protokol Needham-Schroeder

echo "=========================================="
echo "      Needham-Schroeder Test Script       "
echo "=========================================="

cleanup() {
    echo ""
    echo "--- Cleaning up ---"
    if [ ! -z "$KDC_PID" ]; then
        kill $KDC_PID 2>/dev/null
        echo "KDC server (PID: $KDC_PID) stopped."
    fi
    if [ ! -z "$BOB_PID" ]; then
        kill $BOB_PID 2>/dev/null
        echo "Bob server (PID: $BOB_PID) stopped."
    fi
    echo "Running 'make clean'..."
    make clean > /dev/null
    echo "Cleanup complete."
}

trap cleanup EXIT

echo "--- Building project ---"
make
if [ $? -ne 0 ]; then
    echo "Build failed. Aborting."
    exit 1
fi
echo "Build successful."
echo ""

if [ ! -f "bin/kdc" ] || [ ! -f "bin/bob" ] || [ ! -f "bin/alice" ]; then
    echo "One or more executables not found in bin/. Aborting."
    exit 1
fi

echo "--- Starting servers ---"
./bin/kdc &
KDC_PID=$!
echo "KDC server started with PID: $KDC_PID"

./bin/bob &
BOB_PID=$!
echo "Bob server started with PID: $BOB_PID"
echo ""

sleep 2

echo "--- Starting Alice to initiate the protocol ---"
./bin/alice
echo "--- Alice's process finished ---"
echo ""

sleep 1

exit 0