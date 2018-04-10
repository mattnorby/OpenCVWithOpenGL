# OpenCVWithOpenGL
Proof of concept for loading an image from OpenCV as an OpenGL texture, and displaying 3D content on it.

The program expects an image file name as its first command-line argument.
The image is displayed as a texture on a rectangle that nearly covers the viewing volume.
On top of the image, a top-down view of a 3D gem is displayed.
The camera zooms in and out to demonstrate perspective rendering.

The purpose of the program is to make an incremental step toward augmented reality.
It shows that it is possible to load an image from OpenCV, and use it as an OpenGL texture.
It also shows that 3D objects can be positioned on top of the image.

Files in the repository:
- CodeBlocks project and layout
- C++ source code
- A sample image
