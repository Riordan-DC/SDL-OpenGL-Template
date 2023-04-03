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
SDL's image and audio (and perhaps Sean Barrett's) loading functions.
cJSON for loading and writing JSON, a nice format.

## MacOS Build
You may need to install SDL2 with brew:
```
brew install SDL2

brew install SDL2_image

brew install SDL2_ttf
```
Because we installed SDL2 with brew we dont have a .framework folder.
You can use the follow commands to find where your SDL2 is installed:
```
sdl2-config --cflags
-I/opt/homebrew/include/SDL2 -D_THREAD_SAFE

sdl2-config --libs
-L/opt/homebrew/lib -lSDL2
```

The -I and -L are our header includes and library link respectively.
You may need to remove the last SDL2 in the include file path as it is
included in the code as SDL2/SDL.h, meaning it is looking for the SDL2 folder.

References:
How to compile with SDL2 on MacOS if installed via brew:
https://stackoverflow.com/questions/28016258/using-homebrew-installed-sdl2-with-xcode