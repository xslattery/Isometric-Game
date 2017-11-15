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
This platform has not been setup yet.