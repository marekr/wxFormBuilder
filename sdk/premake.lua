project.name = "wxFormBuilder SDK"
if ( OS == "windows" ) then
	project.configs = { "Debug", "Debug (Unicode)", "Release", "Release (Unicode)" }
end
project.libdir = "lib"

-- Add sdk projects here.
dopackage( "plugin_interface" )
dopackage( "tinyxml" )
