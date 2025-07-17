# pico2-rasterizer
Tested and works with the Pico2/2W accompanied with the waveshare 1.3inch LCD (https://www.waveshare.com/pico-lcd-1.3.htm).<br>
Set to rasterize the default teapot, using all the buttons you can move around the shape and rotate it.<br>
The near plane clipping is rudimentary and will simply not draw the entire triangle if any Z coordinate is equal to or less than 0.<br>
The back face culling implemented means that shapes drawn must be closed and being inside them will not work.<br>

The python script to parse obj files (parse_obj.py) is set to look for a file called "teapot.obj" but this can be easily changed.<br>
The script will create a files listing edges and triangles in the style of a C array. To use these values, paste them into shapeVerticies and shapeTriangles in resources.c and rebuild the project.
