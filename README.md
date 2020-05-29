# Computer Aided Diagnosis

An ISEN's project.


LIBRARIES USED, SETTINGS:
-------------------------

Some compilation settings can be changed. In order to do so,
some lines in the file 'compiler_settings.mk' can be commented/uncommented:


- High performance library OpenBLAS for fast matrix product on CPUs.
  Optional, but highly recommended for fast learning. Line concerned:

HIGH_PERF_LIB = OPENBLAS
HIGH_PERF_PATH = /home/username/OpenBlas


- Graphical library used for drawing and user inputs, SDL2.
  Only necessary for the animation.


- API used to communicate with the SSH server: MySQL C API.


INSTALLING (Ubuntu):
--------------------

Only Linux is supported at this time.


- Installing SDL2:

sudo apt-get install libsdl2-dev
sudo apt-get install libsdl2-image-dev
sudo apt-get install libsdl2-ttf-dev


- Installing some FreeType fonts:

sudo apt-get install libfreetype6-dev


- Installing the MySQL C API:

sudo apt-get install mysql-server libmysqlclient-dev

Documentation can be found here: https://dev.mysql.com/doc/refman/5.7/en/c-api.html


- Installing OpenBLAS (optional):

Follow the instructions from: https://www.openblas.net/

Building from source may take a while.


COMPILING:
----------

- For compiling all projects in the right order, type:

sh cleanAndBuild.sh


- For building each project separately, do in each folder:

make clean
make


- For cleaning and compressing the whole project to an archive
  placed in the same directory than the project is, type:

sh compress.sh


It is crucial to emphasize here that doing a 'make clean' call can be necessary
(before the 'make' one), in case only a header file has been modified, or when
a static library the code depends on have been updated.


LAUNCHING:
----------


Go to the desired project directory, find the executable name, and type
(by replacing 'executable_name' by the correct one):

./executable_name


FOR A WINDOWS PORT:
-------------------

Potential incompatibilities:

- Saved .txt files may have missing carriage returns (\r\n vs \n).
- functions for creating directories, moving files, etc...
- getFileSize()
- byte_swap_... functions
