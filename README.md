# Isometric Game

## About:
This project is currently a work in process. It is going to be an 2D Isometric view of a simulated 3D world where the user will be able to control what happens. It is inspired by Dwarf Fortress.

## Platforms:
Planed platforms include:
- MacOS / OSX
- Windows *(Development has not yet begun for this platform.)*

## Dependencies:
#### OSX:
As this project uses Apples API's it is macos / osx dependent. The libraries / frameworks it requires are:

 - Cocoa framework
 - Quartz framework
 - OpenGL framework

All the above libraries / frameworks can be found on all modern macos / osx installs. It is recomended that you have Xcode installed for this project.

#### Windows:
*This part of the project has not yet begun development.*

## Build Process:
This project includes a **bash build script** (for OSX) that when run will build the project.
The Compiler user by the build script is *Clang*.

- **./build_osx.sh**, will generate an OSX exectuable using *Clang*.
- Build products can be found inside the builds directory.

## Tools:
#### Sublime Text:
There is a Sublime Text 3 project included inside the **'other/'** folder for convenience. It can be used to build directly from inside the sublime editor.

#### Xcode (OSX):
An Xcode project has been setup inside the **'other/'** folder to allow for debugging the generated exectutable. The **'-g'** compiler option is required to be passed to *clang* for breakpoints and other propper debugging to work.

- **NOTE:** By default the build script is referenced relative to the project directory, however for the scheme you will need to point it to the location of the generated exectuable as this can not be relative.