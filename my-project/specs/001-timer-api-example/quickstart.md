# Quickstart & Validation Guide

## Prerequisites
- QNX SDP / Toolchain installed.
- Terminal with `make` and `qcc` access.

## Compilation
1. Navigate to the example directory: `cd Timer_API_Example`
2. Run `make` to compile the source code: `make`
3. The resulting binary will be outputted as `Timer_API_Example`.

## Execution
Run the compiled binary:
`./Timer_API_Example`

## Expected Outcomes
1. The program should print the current clock period for both CLOCK_REALTIME and CLOCK_MONOTONIC.
2. It will indicate that a timer was successfully created.
3. The program will print pulse notification events confirming that the timer is firing periodically (e.g., every 1 second).
4. After a set number of pulses, the program will delete the timer and exit cleanly.
