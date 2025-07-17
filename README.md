# pico2-rasterizer
Tested and works with the Pico2/2W accompanied with the waveshare 1.3inch LCD (https://www.waveshare.com/pico-lcd-1.3.htm)
Set to rasterize the default teapot, using all the buttons you can move around the shape and rotate it.
The near plane clipping is rudimentary and will simply not draw the entire triangle if any Z coordinate is equal to or less than 0
The back face culling implemented means that shapes drawn must be closed and being inside them will not work.

The python script to parse obj files is set to look for a file called "teapot.obj" but this can be easily changed.
The file will create a file for edges and a file for verticies, in resources.c paste these values into shapeVerticies and shapeTriangles.
