wxFormBuilder Plugin SDK
------------------------

Includes:

	1. plugin_interface
		wxFormBuilder has a plug-in interface and every plug-in shares this as a
		dependancy. It builds to a static library and should be included in any
		and all plug-ins.
		
		You can build it use the included Code::Blocks project (.cbp). 
		There will be makefile support soon.
    
	2. Tinyxml++ 2.4.2
		wxFormBuilder uses TinyXML++ for writing and reading xml. For building TinyXML++
		use the included Code::Blocks project (.cbp). There will be makefile support soon.
		
		You can find TinyXML++ on http://code.google.com/p/ticpp/

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