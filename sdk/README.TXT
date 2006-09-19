wxFormBuilder third party libraries
-----------------------------------

Includes:

  1. A subset of boost library (v.1.31.0)
     
     wxFormBuilder just uses smart pointers from boost (shared_ptr/weak_ptr). There is no
     modifications with original boost, I only remove files that it's not necessary for
     building.
     
     You don't have to build this library because there are only headers files.
     
     You can find the complete boost library on 
     
  2. Tinyxml++ 2.4.2
  
     There is no modifications with the original Tinyxml, I only add a makefile named
     Makefile.tinyxml for build with the config used on wxFormBuilder. For building type:
     
     Use the included Code::Blocks project (.cbp) if you want to build this library alone.
       
  3. wxAdditions
	 This is a complete set of additional components that is required to build wxFormBuilder.
	 You can get the source using Subversion. There are included Code::Blocks workspace for 
	 Linux and makefiles for Windows. This is going to change soon to makefiles for all
	 platforms, it just needs testing.
	 
	 Subversion command:
		svn co https://svn.sourceforge.net/svnroot/wxformbuilder/additions/trunk additions
		
	  - For Linux, you have the Code::Blocks projects files.
	  
	  - For Win32, you have makefiles.
  
  
Notes:
  - Code::Blocks is a free cross-platform IDE and it can be found here:
    http://codeblocks.org
	
  - Subversion is a great free cross-platform version control manager.
    It can be found here:
	http://subversion.tigris.org

  - I suggest you use the Code::Blocks workspace because it will place everything where
    it belongs. No need to worry about the files being located in the proper location.
	
  - Makefiles will be coming in the future.

Enjoy,
  The wxFormBuilder Team
  
  José Antonio Hurtado.
  mail: joseantonio.hurtado@gmail.com
