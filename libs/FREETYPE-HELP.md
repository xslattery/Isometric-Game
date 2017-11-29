# NOTICE:
## OSX:
This project comes with a precompiled static library for freetype. To build the library for yourself complete the following instructions:

1. Download the latest source archive from: https://www.freetype.org/download.html
2. Configure the cmake build from terminal: *'./configure --without-zlib --without-png --without-bzip2'*
3. From terminal: *'make clean'*
4. From terminal: *'make'*
5. From terminal: *'cd objs/'*
6. From terminal: *'cd .libs/'*
7. From terminal: *'cp libfreetype.a <LOCATION YOU WANT IT>/libfreetype.a'*

- The last step is done because on mac you cant access a '.' directory from finder.

## Windows:
This project comes with a precompiled static library for freetype. To build the library yourself complete the following instructions:

1. Download the latest source archive from: https://www.freetype.org/download.html
2. Open the *builds\windows* directory
3. Open the latest visual studio solution
4. Build statically for x64

- NOTE: When using the library you will need to also link 'msvcrt.lib' and 'msvcmrt.lib' when linking freetype to the project I dont know why this is. But you will get a linker error if you don't.
