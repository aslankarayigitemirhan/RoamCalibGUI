# Camera Calibration GUI

This project provides a graphical user interface (GUI) for camera calibration.  
It is built with Qt for the interface and OpenCV for the underlying calibration algorithms.

## Features

- **Device Management**
  - Automatically detects connected cameras.
  - Supports multiple backends such as V4L2 and GStreamer.

- **Live Preview**
  - Displays live frames from the camera.
  - Shows distorted (original) and undistorted (corrected) images side by side.

- **Calibration Patterns**
  - Checkerboard
  - Circle grid (symmetric and asymmetric)
  - AprilTag support
  - Fully configurable parameters.

- **Corner Detection Settings**
  - Sub-pixel refinement
  - Iteration count and epsilon threshold adjustments

- **Dataset Management**
  - Captured frames are displayed in a thumbnail gallery.
  - Import/export dataset functionality
  - Per-frame include/exclude selection

- **Calibration**
  - Supports both Pinhole and Fisheye camera models
  - Computes RMS error and per-view reprojection error
  - Estimates camera matrix and distortion coefficients
  - Provides an undistortion function for new images

- **Results Visualization**
  - Reprojection error charts (bar plots)
  - Linearity metrics over frames
  - Displays calibration parameters (camera matrix, distortion coefficients) in text form
  - Export results to JSON files

- **Dataset Optimization**
  - Identifies frames with high reprojection error
  - Allows automatic exclusion of outliers based on average + sigma statistical filtering

## Demo

The following videos illustrate the functionality:

- **Before Calibration**  
  [pre-calib-demo.mp4](pre-calib-demo.mp4)

- **After Calibration**  
  [calib_demo.mp4](calib_demo.mp4)

## Usage

1. Select and open a camera device.
2. Configure the calibration pattern.
3. Verify that the pattern is correctly detected in the live preview.
4. Capture sufficient frames.
5. Run calibration.
6. Inspect results and optionally optimize the dataset.

## Dependencies

- Qt 6 (Widgets + Charts modules)
- OpenCV (core, imgproc, calib3d, highgui)
- Eigen (for advanced math operations)

## Build Instructions

```bash
mkdir build && cd build
cmake ..
make
./calib_gui
