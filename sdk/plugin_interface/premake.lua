package.name = "Plugin_Interface"
package.kind = "lib"
package.language = "c++"
package.files = { matchfiles( "*.h", "*.cpp" ) }
-- Change the default lib extention to .a instead of .lib
package.targetextension = "a"
-- Set object output directory.
package.config["Debug"].objdir = ".objsd"
package.config["Debug (Unicode)"].objdir = ".objsud"
package.config["Release"].objdir = ".objs"
package.config["Release (Unicode)"].objdir = ".objsu"
-- Set the targets.
package.config["Debug"].target = "fbPluginInterfaced"
package.config["Debug (Unicode)"].target = "fbPluginInterfaceud"
package.config["Release"].target = "fbPluginInterface"
package.config["Release (Unicode)"].target = "fbPluginInterfaceu"
-- Set the build options for the Unicode build Targets.
package.config["Debug (Unicode)"].buildflags = { "unicode" }
package.config["Release (Unicode)"].buildflags = { "no-symbols", "optimize", "unicode" }
-- Set include paths
package.includepaths = { "$(#WX.include)", "../tinyxml" }
-- Set defines.
package.config["Debug"].defines = 
{
	"DEBUG",
	"WIN32",
	"_WINDOWS",
	"HAVE_W32API_H",
	"__WX__",
	"__WXMSW__",
	"__WXDEBUG__",	
	"TIXML_USE_TICPP"
}
package.config["Debug (Unicode)"].defines = 
{
	"DEBUG",
	"WIN32",
	"_WINDOWS",
	"HAVE_W32API_H",
	"__WX__",
	"__WXMSW__",
	"__WXDEBUG__",	
	"TIXML_USE_TICPP",
	"UNICODE",
	"_UNICODE",
	"wxUSE_UNICODE"
}
package.config["Release"].defines = 
{
	"NDEBUG",
	"WIN32",
	"_WINDOWS",
	"HAVE_W32API_H",
	"__WX__",
	"__WXMSW__",
	"TIXML_USE_TICPP"
}
package.config["Release (Unicode)"].defines = 
{
	"NDEBUG",
	"WIN32",
	"_WINDOWS",
	"HAVE_W32API_H",
	"__WX__",
	"__WXMSW__",
	"TIXML_USE_TICPP",
	"UNICODE",
	"_UNICODE",
	"wxUSE_UNICODE"
}
