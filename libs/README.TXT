wxFormBuilder third party libraries
-----------------------------------

Includes:

  1. A subset of boost library (v.1.31.0)
     
     wxFormBuilder just uses smart pointers from boost (shared_ptr/weak_ptr). There is no
     modifications with original boost, I only remove files that it's not necessary for
     building.
     
     You don't have to build this library because there are only headers files.
     
     You can find the complete boost library on 
     
  2. Tinyxml 2.3.2
  
     There is no modifications with the original Tinyxml, I only add a makefile named
     Makefile.tinyxml for build with the config used on wxFormBuilder. For building type:
     
     make -f Makefile.tinyxml
     
Notes:

  - You need to compile the contrib wxStyledTextControl (stc) of wxWidgets package.
  - For linux building, be sure of library paths are correctly on Makefile.
  
    I use this structure of directories:
    
    wxFormBuilder/
      bin/    
        xml/
        xpm/
        projects/

     libs/
       boost_1_31_0/
       tinyxml/
       
     src/
       ...	
  
  
  
  - For Win32, you have the Dev-C++ projects files, also verify library directories.
  
  
  
  Enjoy,
  
    José Antonio Hurtado.
    
    mail: joseantonio.hurtado@hispalinux.es
     
     
     
     