# SDL-OpenGL-Template
"I always forget how to do this..."

C/C++ SDL OpenGL Project Template :-)
Run make to build a platform specific blank SDL window with OpenGL functions.

Simply edit the SConstruct file with correct SDL include/ and lib/ locations, then write code in src/ and include/ to your hearts content!

Middleware:
OpenGL functions are loaded with Glad. If you require specific support or extensions simply replace the glad files! :-)
Physics is processed using Bullet physics, statically linked.
The iconic cgltf library is used for loading gltf 3d files.
GUI system backend is Nuklear, so cool.
Sean Barrett's image and audio loading headers.
cJSON for loading and writing JSON, a nice format.
