Pi To Infinity
==============

A simple game engine on the Raspberry Pi using OpenGL ES and C.

It turns out that X isn't required to draw 3D graphics to the framebuffer, and
keyboard and mouse controls can be implemented at the console. In the interest
of raw performance this seems like a nice approach. It remains to be seen 
whether or not there are compatiblity problems with other mice and keyboards.

This project should be updated to use OpenGL ES 2.0 and buffer objects. Drawing
voxels without using VBO is very limiting.

There's a good tutorial on creating voxel based worlds similar to Minecraft
called [Glescraft](http://en.wikibooks.org/wiki/OpenGL_Programming/Glescraft_1)
but it starts with C and then adopts C++. [Kazmath](https://github.com/Kazade/kazmath)
might be useful. It could be interesting to use [Go](http://golang.org/).

This project compiles and runs with an earlier version of Arch Linux ARM.
It's likely that this needs to be updated, and the current version of the
Raspberry Pi GL examples should be helpful.

GLES 1 vs. GLES 2
-----------------

Unavailable functions:

  * glPushMatrix
  * glTranslatef
  * glPopMatrix
  * glEnableClientState
  * glVertexPointer
  * glTexCoordPointer
  * glMatrixMode
  * glLoadIdentity
  * glFrustumf
  * glMultMatrixf

Unavailable constants:

  * GL_VERTEX_ARRAY
  * GL_TEXTURE_COORD_ARRAY
  * GL_MODELVIEW
