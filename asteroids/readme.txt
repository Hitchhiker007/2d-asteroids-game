* Visual studio solution is in vs folder.
* External libraries needed to compile OpenGL programs using visual studio are in libs.
* When creating new source files using visual studio, you should make sure they go in the src folder.
* Compiled executables are placed in this folder.
* Don't delete freeglut.dll, it's needed.
* Compile 64 bit versions only, as freeglut.dll is a 64 bit dll.

## Building on macOS
* Visual Studio is not available on macOS. Use the terminal to compile.
* Install freeglut via Homebrew: brew install freeglut
* Compile from the project root directory with:
  gcc src/main.c -framework OpenGL -framework GLUT -o asteroids
* Run with: ./asteroids

## Controls
* W - thrust forward
* A - rotate left
* D - rotate right
* Left click - shoot
* Q / ESC - quit

## Known Issues
* Score display only shows the last digit
* Puff particles are never freed from the linked list during gameplay, only on exit
* Screen resolution is hardcoded to 2560x1440 - change SCREEN_WIDTH and SCREEN_HEIGHT in main.c to match your display