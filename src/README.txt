Compiling wxFormBuilder
-----------------------

In order to compile wxFormBuilder you need these libraries:

- wxWidgets - 2.6.0 (compiled with SHARED=1)
 
- boost     - 1.31
  Used only for automatic memory management with "smart pointers".
  See http://www.boost.org/libs/smart_ptr/shared_ptr.htm for more info.

- tinyxml   - 2.3.2
  Used for loading/saving XML files.
  
- wxpropgrid 1.0.4

- wxStyledTextCtrl (wxWidgets contrib)


Change appropiate paths on Makefile for Linux, or into Dev-C++ project
settings for Windows.

NOTES:
  1. You don't have to build boost libraries (it only uses header files).
  2. compile tinyxml as a static library with -DTINYXML_USE_STL
     (copy Makefile.tinyxml into tinyxml sources and build it)


On Linux I use this path structure:

--------------------------------------------------------------
<project>
    +- src
        +- xml : all xml files
        +- xpm : all icon files
        +- projects
              +- example  : example projects
    +- libs
         +- boost_1_31_0  : boost library
         +- tinyxml       : tinyxml library
	 
    +- bin
        +- xml
	+- resources
	+- xpm
         
--------------------------------------------------------------     

How-to build on Linux
---------------------

Steps to build wxFormBuilder on Linux.

1. Download and build wxGTK-2.6.0, wxpropgrid 1.0.4 and wxStyledTextCtrl as shared library.
   Also you need download wxformbuilder-libs.zip and unzip it following the directory
   structure showed above.
2. Build tinyxml as static library with Makefile.tinyxml.
3. Create "bin" directory at the same level that src directory (see above).
4. Copy src/xml/ src/resources/ and src/xpm/ subdirectories into bin subdirectory.
   Also, copy splash.png into bin directory.
5. Edit src/Makefile and set LIBS var with the aproppiate libraries names
   (because it depends of wxWidgets configuration).
6. Repeat step 5 with src/plugins/standard/Makefile
7. Build src/
8. Build src/plugins/standard
9. Check if libcommon.so and liblayout.so are placed into bin/xml/
10. go to bin/ a run ./wxFormBuilder


NOTE:
 
 This makefiles are edited by hand, if you are familiar with bakefile tool, please
 help us to make easy the build process. Thanks.


How-to build on Windows
-----------------------

1. Install all required libs with the same directory structure showed above.
2. Build tinyxml as static library with Makefile.tinyxml.
3. Create "bin" directory at the same level that src directory (see above).
4. Copy src/xml/ src/resources/ and src/xpm/ subdirectories into bin subdirectory.
   Also, copy splash.png into bin directory.
5. Edit src/plugins/standard/Makefile.260 and set LIBS var with the aproppiate libraries names
   (because it depends of wxWidgets configuration).
6. Use codeblocks project to build wxFormBuilder. Modify directory settings for wxWidgets if you need it.
7. Build standard plugins with Makefile.260.
8. Check if libcommon.dll and liblayout.dll are placed into bin/xml/
9. Run wxFormBuilder.exe

     
Enjoy :-)


José Antonio Hurtado.

joseantonio.hurtado@gmail.com

2005-06-26

