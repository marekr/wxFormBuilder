///////////////////////////////////////////////////////////////////////////////
//
// wxFormBuilder - A Visual Dialog Editor for wxWidgets.
// Copyright (C) 2005 José Antonio Hurtado
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// Written by
//   José Antonio Hurtado - joseantonio.hurtado@gmail.com
//   Juan Antonio Ortega  - jortegalalmolda@gmail.com
//
///////////////////////////////////////////////////////////////////////////////

#include "cppcg.h"
#include "utils/typeconv.h"
#include <wx/filename.h>
#include "rad/global.h"
#include <sstream>

CppTemplateParser::CppTemplateParser(shared_ptr<ObjectBase> obj, string _template)
: TemplateParser(obj,_template)
{
	m_useRelativePath = false;
	m_i18n = false;
}
string CppTemplateParser::RootWxParentToCode()
{
	return "this";
}

shared_ptr<TemplateParser> CppTemplateParser::CreateParser(shared_ptr<ObjectBase> obj, string _template)
{
	shared_ptr<TemplateParser> newparser(new CppTemplateParser(obj,_template));
	return newparser;
}

void CppTemplateParser::UseRelativePath(bool relative, string basePath)
{
	m_useRelativePath = relative;

	if (m_useRelativePath)
	{
		bool result = wxFileName::DirExists(_WXSTR(basePath));
		m_basePath = (result ? basePath : "");
	}
}

void CppTemplateParser::UseI18n(bool i18n)
{
	m_i18n = i18n;
}

/**
* Convert the value of the property to C++ code
*/
string CppTemplateParser::ValueToCode( PropertyType type, string value )
{
	std::ostringstream result;

	switch ( type )
	{
	case PT_WXSTRING:
		{
			result << "wxT(\"" << CppCodeGenerator::ConvertCppString( value ) << "\")";
			break;
		}
	case PT_WXSTRING_I18N:
		{
			if ( m_i18n )
			{
				result << "_(\"" << CppCodeGenerator::ConvertCppString(value) << "\")";
			}
			else
			{
				result << "wxT(\"" << CppCodeGenerator::ConvertCppString(value) << "\")";
			}
			break;
		}
	case PT_MACRO:
	case PT_TEXT:
	case PT_OPTION:
	case PT_FLOAT:
		{
			result.str( value );
			break;
		}
	case PT_BITLIST:
		{
			result.str( value.empty() ? "0" : value );
			break;
		}
	case PT_WXPOINT:
		{
			if ( value.empty() )
			{
				result.str( "wxDefaultPosition" );
			}
			else
			{
				result << "wxPoint( " << value << " )";
			}
			break;
		}
	case PT_WXSIZE:
		{
			if ( value.empty() )
			{
				result.str( "wxDefaultSize" );
			}
			else
			{
				result << "wxSize( " << value << " )";
			}
			break;
		}
	case PT_BOOL:
		{
			result.str( value == "0" ? "false" : "true" );
			break;
		}
	case PT_WXFONT:
		{
			if ( !value.empty() )
			{
				wxFont font = TypeConv::StringToFont( _WXSTR(value) );
				result	<< "wxFont( "
					<< font.GetPointSize()	<< ", "
					<< font.GetFamily()		<< ", "
					<< font.GetStyle()		<< ", "
					<< font.GetWeight()		<< ", "
					<< ( font.GetUnderlined() ? "true, " : "false, " )
					<< "wxT(\"" << _STDSTR( font.GetFaceName() ) << "\") )";
			}
			else
			{
				result.str( "wxFont()" );
			}
			break;
		}
	case PT_WXCOLOUR:
		{
			if ( !value.empty() )
			{
				if ( value.find_first_of( "wx" ) == 0 )
				{
					// System Colour
					result << "wxSystemSettings::GetColour( " << value << " )";
				}
				else
				{
					wxColour colour = TypeConv::StringToColour( _WXSTR(value) );
					result << "wxColour( " << (int)colour.Red() << ", " << (int)colour.Green() << ", " << (int)colour.Blue() << " )";
				}
			}
			else
			{
				result.str( "wxColour()" );
			}
			break;
		}
	case PT_BITMAP:
		{
			// Splitting bitmap resource property value - it is of the form "path; source"
			size_t semicolonIndex = value.find_first_of( ";" );
			std::string path;
			std::string source;
			if ( semicolonIndex != value.npos )
			{
				path = value.substr( 0, semicolonIndex );
				source = value.substr( semicolonIndex + 2 ); // Separated by "; "
			}
			else
			{
				path = value;
				source = "Load From File";
			}

			if ( path.empty() )
			{
				// Empty path, generate Null Bitmap
				result.str( "wxNullBitmap" );
				break;
			}

			if ( source == "Load From File" )
			{
				wxString absPath = TypeConv::MakeAbsolutePath( _WXSTR(path), GlobalData()->GetProjectPath() );
				string file = _STDSTR( m_useRelativePath ? TypeConv::MakeRelativePath( absPath, _WXSTR(m_basePath) ) : absPath );

				std::string cppString = CppCodeGenerator::ConvertCppString( file );

				wxFileName bmpFileName( _WXSTR(path) );
				if ( bmpFileName.GetExt().Upper() == wxT("XPM") )
				{
					// If the bitmap is an XPM we will embed it in the code, otherwise it will be loaded from the file at run time.
					result << "wxBitmap( " << CppCodeGenerator::ConvertXpmName( path ) << " )";
				}
				else
				{
					result << "wxBitmap( wxT(\"" << cppString << "\"), wxBITMAP_TYPE_ANY )";
				}
			}
			else if ( source == "Load From Resource" )
			{
				result << "wxBitmap( wxT(\"" << path << "\"), wxBITMAP_TYPE_RESOURCE )";
			}
			else if ( source == "Load From Icon Resource" )
			{
				result << "wxICON( " << path << " )";
			}

			break;
		}
	case PT_STRINGLIST:
		{
			// Stringlists are generated like a sequence of wxString separated by ', '.
			wxArrayString array = TypeConv::StringToArrayString( _WXSTR(value) );
			if ( array.Count() > 0 )
			{
				result.str( ValueToCode( PT_WXSTRING_I18N, _STDSTR(array[0]) ) );
			}

			for ( size_t i = 1; i < array.Count(); i++ )
			{
				result << result << ", " << ValueToCode( PT_WXSTRING_I18N, _STDSTR( array[i] ) );
			}
			break;
		}
	default:
		break;
	}

	return result.str();
}

///////////////////////////////////////////////////////////////////////////////

CppCodeGenerator::CppCodeGenerator()
{
	SetupPredefinedMacros();
	m_useRelativePath = false;
	m_i18n = false;
	m_firstID = 1000;
}

string CppCodeGenerator::ConvertCppString( string text )
{
	string result;

	for ( size_t i = 0; i < text.length(); i++ )
	{
		char c = text[i];

		switch ( c )
		{
		case '"':
			result += "\\\"";
			break;

		case '\\':
			result += "\\\\";
			break;

		case '\t':
			result += "\\t";
			break;

		case '\n':
			result += "\\n";
			break;

		case '\r':
			result += "\\r";
			break;

		default:
			result += c;
			break;
		}
	}
	return result;
}

std::string CppCodeGenerator::ConvertXpmName( const std::string& text )
{
	std::string name = text;
	// the name consists of extracting the name of the file (without the directory)
	// and replacing the character '.' by ' _ '.

	size_t last_slash = name.find_last_of( "\\/" );
	if ( last_slash != name.npos )
	{
		name = name.substr( last_slash + 1 );
	}

	name.replace( name.rfind( "." ), 1, "_" );

	return name;
}

bool CppCodeGenerator::GenerateCode( shared_ptr<ObjectBase> project )
{
	if (!project)
	{
		wxLogError(wxT("There is no project to generate code"));
		return false;
	}

	bool useEnum = false;

	shared_ptr< Property > useEnumProperty = project->GetProperty("use_enum");
	if (useEnumProperty && useEnumProperty->GetValueAsInteger())
		useEnum = true;

	m_i18n = false;
	shared_ptr< Property > i18nProperty = project->GetProperty("internationalize");
	if (i18nProperty && i18nProperty->GetValueAsInteger())
		m_i18n = true;

	m_header->Clear();
	m_source->Clear();
	string date(__DATE__);
	string time(__TIME__);
	string code_header (
		"///////////////////////////////////////////////////////////////////////////\n"
		"// C++ code generated with wxFormBuilder (version " __DATE__ ")\n"
		"// http://wxformbuilder.sourceforge.net/\n"
		"//\n"
		"// PLEASE DO \"NOT\" EDIT THIS FILE!\n"
		"///////////////////////////////////////////////////////////////////////////\n");

	m_header->WriteLn( code_header );
	m_source->WriteLn( code_header );

	shared_ptr<Property> propFile = project->GetProperty("file");
	if (!propFile)
	{
		wxLogError( wxT("Missing \"file\" property on Project Object") );
		return false;
	}

	string file = propFile->GetValue();
	if (file == "")
		file = "noname";

	m_header->WriteLn("#ifndef __" + file + "__");
	m_header->WriteLn("#define __" + file + "__");
	m_header->WriteLn("");

	string code = GetCode(project,"header_preamble");
	m_header->WriteLn( code );

	// Generate the libraries
	set< string > libraries;
	GenLibraries( project, &libraries );

	// Write the library lines
	WriteLibrariesBlock( libraries );

	// generamos en el h los includes de las dependencias de los componentes.
	set<string> includes;
	GenIncludes(project, &includes);

	// Write the include lines
	set<string>::iterator include_it;
	for ( include_it = includes.begin(); include_it != includes.end(); ++include_it )
	{
		m_header->WriteLn( *include_it );
	}
	if ( !includes.empty() )
	{
		m_header->WriteLn("");
	}

	code = GetCode(project,"header_epilogue");
	m_header->WriteLn( code );
	m_header->WriteLn("");

	// en el cpp generamos el include del .h generado y los xpm
	code = GetCode(project,"cpp_preamble");
	m_source->WriteLn( code );
	m_source->WriteLn("");

	m_source->WriteLn("#include \""+file+".h\"");
	m_source->WriteLn("");
	GenXpmIncludes( project );

	code = GetCode(project,"cpp_epilogue");
	m_source->WriteLn( code );

	// generamos los defines de las macros
	if ( !useEnum )
		GenDefines( project );

	for ( unsigned int i = 0; i < project->GetChildCount(); i++ )
	{
		GenClassDeclaration( project->GetChild( i ), useEnum );
		GenConstructor( project->GetChild( i ) );
	}

	m_header->WriteLn("#endif //__" + file + "__");

	return true;
}

void CppCodeGenerator::GenAttributeDeclaration(shared_ptr<ObjectBase> obj, Permission perm)
{
	std::string typeName = obj->GetObjectTypeName();
	if (typeName == "notebook"			||
		typeName == "flatnotebook"		||
		typeName == "listbook"			||
		typeName == "choicebook"		||
		typeName == "widget"			||
		typeName == "expanded_widget"	||
		typeName == "statusbar"			||
		typeName == "component"			||
		typeName == "container"			||
		typeName == "menubar"			||
		typeName == "toolbar"			||
		typeName == "splitter"
		)
	{
		string perm_str = obj->GetProperty("permission")->GetValue();

		if ((perm == P_PUBLIC && perm_str == "public") ||
			(perm == P_PROTECTED && perm_str == "protected") ||
			(perm == P_PRIVATE && perm_str == "private"))
		{
			// generamos la declaración
			string code = GetCode(obj,"declaration");
			m_header->WriteLn(code);
		}
	}

	// recursivamente generamos los demás atributos
	for (unsigned int i = 0; i < obj->GetChildCount() ; i++)
	{
		shared_ptr<ObjectBase> child = obj->GetChild(i);

		GenAttributeDeclaration(child,perm);
	}
}

string CppCodeGenerator::GetCode(shared_ptr<ObjectBase> obj, string name)
{
	string _template;
	shared_ptr<CodeInfo> code_info = obj->GetObjectInfo()->GetCodeInfo("C++");
	if (!code_info)
	{
		wxString msg( wxString::Format( wxT("Missing \"%s\" template for \"%s\" class. Review your XML object description"),
			name.c_str(), obj->GetClassName().c_str()));
		wxLogError(msg);
		return "";
	}

	_template = code_info->GetTemplate(name);

	CppTemplateParser parser(obj,_template);
	parser.UseRelativePath(m_useRelativePath, m_basePath);
	parser.UseI18n(m_i18n);
	string code = parser.ParseTemplate();

	return code;
}

void CppCodeGenerator::GenClassDeclaration(shared_ptr<ObjectBase> class_obj, bool use_enum)
{
	shared_ptr<Property> propName = class_obj->GetProperty("name");
	if ( !propName )
	{
		wxLogError(wxT("Missing \"name\" property on \"%s\" class. Review your XML object description"),
			class_obj->GetClassName().c_str());
		return;
	}

	string class_name = propName->GetValue();
	if (class_name == "")
	{
		wxLogError( wxT("Object name can not be null") );
		return;
	}

	m_header->WriteLn("/**");
	m_header->WriteLn(" * Class " + class_name);
	m_header->WriteLn(" */");

	m_header->WriteLn("class " + class_name + " : " + GetCode(class_obj,"base"));
	m_header->WriteLn("{");
	m_header->Indent();

	// private
	m_header->WriteLn("private:");
	m_header->Indent();

	if (use_enum)
		GenEnumIds(class_obj);

	GenAttributeDeclaration(class_obj,P_PRIVATE);
	m_header->Unindent();
	m_header->WriteLn("");

	// protected
	m_header->WriteLn("protected:");
	m_header->Indent();
	GenAttributeDeclaration(class_obj,P_PROTECTED);
	m_header->Unindent();
	m_header->WriteLn("");

	// public
	m_header->WriteLn("public:");
	m_header->Indent();
	GenAttributeDeclaration(class_obj,P_PUBLIC);

	// The constructor is also included within public
	m_header->WriteLn(GetCode(class_obj,"cons_decl"));
	m_header->Unindent();
	m_header->WriteLn("");

	m_header->Unindent();
	m_header->WriteLn("};");
	m_header->WriteLn("");
}

void CppCodeGenerator::GenEnumIds(shared_ptr< ObjectBase > class_obj)
{
	set<string> macro_set;
	FindMacros(class_obj,macro_set);

	set<string>::iterator it = macro_set.begin();
	if ( it != macro_set.end())
	{
		m_header->WriteLn("enum");
		m_header->WriteLn("{");
		m_header->Indent();

		std::ostringstream firstEnum;
		firstEnum << *it << " = " << m_firstID << ",";
		m_header->WriteLn( firstEnum.str() );
		it++;
		while ( it != macro_set.end() )
		{
			m_header->WriteLn( *it + "," );
			it++;
		}

		//m_header->WriteLn(id);
		m_header->Unindent();
		m_header->WriteLn("};");
		m_header->WriteLn("");
	}
}

void CppCodeGenerator::GenIncludes( shared_ptr<ObjectBase> project, set<string>* includes)
{
	// Call GenIncludes on all children as well
	for ( unsigned int i = 0; i < project->GetChildCount(); i++ )
	{
		GenIncludes( project->GetChild(i), includes );
	}

	// Fill the set
	shared_ptr<CodeInfo> code_info = project->GetObjectInfo()->GetCodeInfo("C++");
	if (code_info)
	{
		CppTemplateParser parser(project,code_info->GetTemplate("include"));
		string include = parser.ParseTemplate();
		if ( !include.empty() )
		{
			includes->insert( include );
		}
	}
}

void CppCodeGenerator::GenLibraries( shared_ptr< ObjectBase > project, set< string >* libraries )
{
	// Call GenIncludes on all children as well
	for ( unsigned int i = 0; i < project->GetChildCount(); i++ )
	{
		GenLibraries( project->GetChild(i), libraries );
	}

	// Fill the set
	shared_ptr<CodeInfo> code_info = project->GetObjectInfo()->GetCodeInfo("C++");
	if (code_info)
	{
		CppTemplateParser parser(project,code_info->GetTemplate("library"));
		string library = parser.ParseTemplate();
		if ( !library.empty() )
		{
			libraries->insert( library );
		}
	}
}

void CppCodeGenerator::WriteLibrariesBlock( const set< string >& libraries )
{
	if ( libraries.empty() )
	{
		return;
	}

	m_header->WriteLn( "#ifdef __WXMSW__" );
	m_header->Indent();
		m_header->WriteLn( "#ifdef _MSC_VER" );
		m_header->Indent();
			m_header->WriteLn( "#ifdef _DEBUG" );
			m_header->Indent();
				m_header->WriteLn( "#ifdef UNICODE  // __WXMSW__, _MSC_VER, _DEBUG, UNICODE" );
				m_header->Indent();
					WriteLibraries( libraries, "#pragma comment( lib, \"wxmsw26ud_", ".lib\" )" );
				m_header->Unindent();
				m_header->WriteLn( "#else  // __WXMSW__, _MSC_VER, _DEBUG" );
				m_header->Indent();
					WriteLibraries( libraries, "#pragma comment( lib, \"wxmsw26d_", ".lib\" )" );
				m_header->Unindent();
				m_header->WriteLn( "#endif" );
			m_header->Unindent();
			m_header->WriteLn( "#else" );
			m_header->Indent();
				m_header->WriteLn( "#ifdef UNICODE  // __WXMSW__, _MSC_VER, UNICODE" );
				m_header->Indent();
					WriteLibraries( libraries, "#pragma comment( lib, \"wxmsw26u_", ".lib\" )" );
				m_header->Unindent();
				m_header->WriteLn( "#else // __WXMSW__, _MSC_VER" );
				m_header->Indent();
					WriteLibraries( libraries, "#pragma comment( lib, \"wxmsw26_", ".lib\" )" );
				m_header->Unindent();
				m_header->WriteLn( "#endif" );
			m_header->Unindent();
			m_header->WriteLn( "#endif" );
		m_header->Unindent();
		m_header->WriteLn( "#endif" );
	m_header->Unindent();
	m_header->WriteLn( "#endif" );
	m_header->WriteLn("");
}

void CppCodeGenerator::WriteLibraries( const set< string >& libraries, const string& prefix, const string& suffix )
{
	set< string >::const_iterator library;
	ostringstream line;
	for ( library = libraries.begin(); library != libraries.end(); ++library )
	{
		line << prefix << *library << suffix;
		m_header->WriteLn( line.str() );
		line.str( "" );
	}
}

void CppCodeGenerator::FindDependencies( shared_ptr< ObjectBase > obj, set< shared_ptr< ObjectInfo > >& info_set )
{
	unsigned int ch_count = obj->GetChildCount();
	if (ch_count > 0)
	{
		unsigned int i;
		for (i = 0; i<ch_count; i++)
		{
			shared_ptr<ObjectBase> child = obj->GetChild(i);
			info_set.insert(child->GetObjectInfo());
			FindDependencies(child, info_set);
		}
	}
}

void CppCodeGenerator::GenConstructor(shared_ptr<ObjectBase> class_obj)
{
	m_source->WriteLn("");
	m_source->WriteLn(GetCode(class_obj,"cons_def"));
	m_source->WriteLn("{");
	m_source->Indent();

	string settings = GetCode(class_obj,"settings");
	if ( !settings.empty() )
	{
		m_source->WriteLn( settings );
	}

	for ( unsigned int i = 0; i < class_obj->GetChildCount(); i++ )
	{
		GenConstruction( class_obj->GetChild( i ), true );
	}

	m_source->Unindent();
	m_source->WriteLn("}");
}

void CppCodeGenerator::GenConstruction(shared_ptr<ObjectBase> obj, bool is_widget)
{
	string type = obj->GetObjectTypeName();

	if (type == "notebook"			||
		type == "flatnotebook"		||
		type == "listbook"			||
		type == "choicebook"		||
		type == "widget"			||
		type == "expanded_widget"	||
		type == "statusbar"			||
		type == "container"			||
		type == "menubar"			||
		type == "toolbar"			||
		type == "splitter"
		)
	{
		// comprobamos si no se ha declarado como atributo de clase
		// en cuyo caso lo declaramos en el constructor

		string perm_str = obj->GetProperty("permission")->GetValue();
		if (perm_str == "none")
			m_source->WriteLn(GetCode(obj,"declaration"));

		m_source->WriteLn(GetCode(obj,"construction"));
		GenSettings( obj->GetObjectInfo(), obj );

		for ( unsigned int i = 0; i < obj->GetChildCount(); i++ )
		{
			shared_ptr< ObjectBase > child = obj->GetChild( i );
			GenConstruction( child, true );

			if (type == "toolbar")
				GenAddToolbar(child->GetObjectInfo(), child);
			}

		if (type == "splitter")
		{
			// generamos el split
			if (obj->GetChildCount() == 2)
			{
				shared_ptr<ObjectBase> sub1,sub2;
				sub1 = obj->GetChild(0)->GetChild(0);
				sub2 = obj->GetChild(1)->GetChild(0);

				string _template;
				if (obj->GetProperty("splitmode")->GetValue()=="wxSPLIT_VERTICAL")
					_template = "$name->SplitVertically(";
				else
					_template = "$name->SplitHorizontally(";

				_template = _template + sub1->GetProperty("name")->GetValue() +
					"," + sub2->GetProperty("name")->GetValue() + ",$sashpos);";

				CppTemplateParser parser(obj,_template);
				parser.UseRelativePath(m_useRelativePath, m_basePath);
				parser.UseI18n(m_i18n);
				m_source->WriteLn(parser.ParseTemplate());
			}
			else
				wxLogError( wxT("Missing subwindows for wxSplitterWindow widget.") );
		}


		if (type == "menubar" || type == "toolbar" || type == "listbook" ||
			type == "notebook" || type == "flatnotebook" )
		{
			string afterAddChild = GetCode(obj,"after_addchild");
			if ( !afterAddChild.empty() )
			{
				m_source->WriteLn( afterAddChild );
			}
			m_source->WriteLn("");
		}

	}
	else if (type == "sizer")
	{
		m_source->WriteLn(GetCode(obj,"declaration"));
		m_source->WriteLn(GetCode(obj,"construction"));
		GenSettings( obj->GetObjectInfo(), obj );

		for ( unsigned int i = 0; i < obj->GetChildCount(); i++ )
			GenConstruction(obj->GetChild(i),false);

		if (is_widget)
		{
			// the parent object is not a sizer. There is no template for
			// this so we'll make it manually.
			// It's not a good practice to embed templates into the source code,
			// because you will need to recompile...

			string _template = "#wxparent $name->SetSizer( $name );\n"
				"#wxparent $name->Layout();";


			CppTemplateParser parser(obj,_template);
			parser.UseRelativePath(m_useRelativePath, m_basePath);
			parser.UseI18n(m_i18n);
			m_source->WriteLn(parser.ParseTemplate());
		}
	}
	else if (type == "menu" || type == "submenu")
	{
		m_source->WriteLn(GetCode(obj,"declaration"));
		m_source->WriteLn(GetCode(obj,"construction"));

		for ( unsigned int i = 0; i < obj->GetChildCount(); i++ )
			GenConstruction( obj->GetChild(i), false );

		m_source->WriteLn(GetCode(obj,"menu_add"));

	}
	else if (type == "spacer")
	{
		// En lugar de modelar un "spacer" como un objeto que se incluye en
		// un sizer item, los vamos a considerar un como un tipo de
		// de "sizeritem" capaz de existir por sí solo. De esta forma será
		// más facil la exportación XRC.
		m_source->WriteLn(GetCode(obj,"spacer_add"));
	}
	else if (type == "sizeritem")
	{
		// El hijo, hay que añadirlo al sizer teniendo en cuenta el tipo
		// del objeto hijo (hay 3 rutinas diferentes)
		GenConstruction(obj->GetChild(0),false);

		string child_type = obj->GetChild(0)->GetObjectTypeName();
		string temp_name;
		if (child_type == "notebook"		||
			child_type == "flatnotebook"	||
			child_type == "listbook"		||
			child_type == "choicebook"		||
			child_type == "widget"			||
			child_type == "expanded_widget"	||
			child_type == "statusbar"		||
			child_type == "container"		||
			child_type == "splitter"
			)
		{
			temp_name = "window_add";
		}
		else if (child_type == "sizer")
		{
			temp_name = "sizer_add";
		}
		else
			assert( false );

		m_source->WriteLn( GetCode( obj, temp_name ) );
	}
	else if (	type == "notebookpage"		||
		type == "flatnotebookpage"	||
		type == "listbookpage"		||
		type == "choicebookpage"
			)
	{
		GenConstruction( obj->GetChild( 0 ), false );
		m_source->WriteLn(GetCode(obj,"page_add"));
		GenSettings(obj->GetObjectInfo(), obj); // RJP 06/08/2006 - Added so that images can be added.
	}
	else if (type == "menuitem")
	{
		m_source->WriteLn(GetCode(obj,"construction"));
		m_source->WriteLn(GetCode(obj,"menuitem_add"));
		GenSettings( obj->GetObjectInfo(), obj );
	}
	else if (type == "tool")
	{
		m_source->WriteLn(GetCode(obj, "construction"));
	}
	else
	{
		//wxLogError(wxString::Format(wxT("Unknown type %s"),_WXSTR(type).c_str()));
		//assert(false);

		// generamos los hijos
		for ( unsigned int i = 0; i < obj->GetChildCount(); i++ )
			GenConstruction( obj->GetChild( i ), false );
		}
	}

void CppCodeGenerator::FindMacros(shared_ptr<ObjectBase> obj, set<string> &macro_set)
{
	// recorre cada propiedad de cada objeto identificando aquellas
	// que sean macros, en cuyo caso la añade al conjunto.
	unsigned int i;

	for ( i = 0; i < obj->GetPropertyCount(); i++ )
	{
		shared_ptr<Property> prop = obj->GetProperty( i );
		if ( prop->GetType() == PT_MACRO )
		{
			string value = prop->GetValue();
			set<string>::iterator it = m_predMacros.find(value);
			if ( it == m_predMacros.end() )
				macro_set.insert( prop->GetValue() );
			}
		}

	for ( i = 0; i < obj->GetChildCount(); i++ )
	{
		FindMacros( obj->GetChild( i ), macro_set );
	}
}

void CppCodeGenerator::GenDefines( shared_ptr< ObjectBase > project)
{
	set<string> macro_set;
	FindMacros( project, macro_set );

	// la macro por defecto tiene valor -1
	m_header->WriteLn("#define ID_DEFAULT wxID_ANY // Default");

	// debemos quitar la macro por defecto del conjunto
	set<string>::iterator it;
	it = macro_set.find("ID_DEFAULT");
	if ( it != macro_set.end() )
		macro_set.erase(it);

	unsigned int id = m_firstID;//FIRST_ID;
	if ( id < 1000 )
	{
		wxLogWarning(wxT("First ID is Less than 1000"));
	}
	for (it = macro_set.begin() ; it != macro_set.end(); it++)
	{
		ostringstream define;
		define << "#define " << *it << " " << id;
		m_header->WriteLn(define.str());
		id++;
	}

	m_header->WriteLn("");
}

void CppCodeGenerator::GenSettings(shared_ptr<ObjectInfo> info, shared_ptr<ObjectBase> obj)
{
	string _template;
	shared_ptr<CodeInfo> code_info = info->GetCodeInfo("C++");

	if ( !code_info )
		return;

	_template = code_info->GetTemplate("settings");

	if (_template != "")
	{
		CppTemplateParser parser(obj,_template);
		parser.UseRelativePath(m_useRelativePath, m_basePath);
		parser.UseI18n(m_i18n);
		string code = parser.ParseTemplate();
		if (code != "")
			m_source->WriteLn(code);
		}

	// procedemos recursivamente con las clases base
	for (unsigned int i=0; i< info->GetBaseClassCount(); i++)
	{
		shared_ptr<ObjectInfo> base_info = info->GetBaseClass(i);
		GenSettings(base_info,obj);
	}
}

void CppCodeGenerator::GenAddToolbar(shared_ptr<ObjectInfo> info, shared_ptr<ObjectBase> obj)
{
	string _template;
	shared_ptr<CodeInfo> code_info = info->GetCodeInfo("C++");

	if (!code_info)
		return;

	_template = code_info->GetTemplate("toolbar_add");

	if (_template != "")
	{
		CppTemplateParser parser(obj,_template);
		parser.UseRelativePath(m_useRelativePath, m_basePath);
		parser.UseI18n(m_i18n);
		string code = parser.ParseTemplate();
		if (code != "")
			m_source->WriteLn(code);
		}

	// procedemos recursivamente con las clases base
	for (unsigned int i=0; i< info->GetBaseClassCount(); i++)
	{
		shared_ptr<ObjectInfo> base_info = info->GetBaseClass(i);
		GenAddToolbar(base_info,obj);
	}

}

///////////////////////////////////////////////////////////////////////


void CppCodeGenerator::GenXpmIncludes( shared_ptr< ObjectBase > project)
{
	set< string > include_set;

	// lo primero es obtener la lista de includes
	FindXpmProperties( project, include_set );

	if ( include_set.empty() )
	{
		return;
	}

	// y los generamos
	set<string>::iterator it;
	for ( it = include_set.begin() ; it != include_set.end(); it++ )
	{
		if ( !it->empty() )
		{
			m_source->WriteLn( *it );
		}
	}

	m_source->WriteLn("");
}

void CppCodeGenerator::FindXpmProperties( shared_ptr<ObjectBase> obj, set<string>& set )
{
	// recorremos cada una de las propiedades del objeto obj, si damos con
	// alguna que sea de tipo PT_XPM_BITMAP añadimos la cadena del "include"
	// en set. Luego recursivamente hacemos lo mismo con los hijos.
	unsigned int i, count;

	count = obj->GetPropertyCount();

	for (i = 0; i < count; i++)
	{
		shared_ptr<Property> property = obj->GetProperty(i);
		if ( property->GetType() == PT_BITMAP )
		{
			std::string path = property->GetValue();
			size_t semicolonindex = path.find_first_of( ";" );
			if ( semicolonindex != path.npos )
			{
				path = path.substr( 0, semicolonindex );
			}

			wxFileName bmpFileName( _WXSTR( path ) );
			if ( bmpFileName.GetExt().Upper() == wxT("XPM") )
			{
				wxString absPath = TypeConv::MakeAbsolutePath( _WXSTR(path), GlobalData()->GetProjectPath() );

				// Se supone el path contiene la ruta completa del archivo y no
				// una relativa.
				string relPath = _STDSTR( m_useRelativePath ? TypeConv::MakeRelativePath( absPath, GlobalData()->GetProjectPath() ) : absPath );

				string inc = "#include \"" + ConvertCppString( relPath ) + "\"";
				set.insert(inc);
			}
		}
	}

	count = obj->GetChildCount();
	for (i = 0; i< count; i++)
	{
		shared_ptr<ObjectBase> child = obj->GetChild(i);
		FindXpmProperties( child, set );
	}
}

void CppCodeGenerator::UseRelativePath(bool relative, string basePath)
{
	bool result;
	m_useRelativePath = relative;

	if (m_useRelativePath)
	{
		result = wxFileName::DirExists(_WXSTR(basePath));
		m_basePath = ( result ? basePath : "" );
	}
}
/*
string CppCodeGenerator::ConvertToRelativePath(string path, string basePath)
{
string auxPath = path;
if (basePath != "")
{
wxFileName filename(_WXSTR(auxPath));
if (filename.MakeRelativeTo(_WXSTR(basePath)))
auxPath = _STDSTR(filename.GetFullPath());
}
return auxPath;
}*/

#define ADD_PREDEFINED_MACRO(x) m_predMacros.insert(#x)
void CppCodeGenerator::SetupPredefinedMacros()
{
	ADD_PREDEFINED_MACRO(wxID_LOWEST);

	ADD_PREDEFINED_MACRO(wxID_OPEN);
	ADD_PREDEFINED_MACRO(wxID_CLOSE);
	ADD_PREDEFINED_MACRO(wxID_NEW);
	ADD_PREDEFINED_MACRO(wxID_SAVE);
	ADD_PREDEFINED_MACRO(wxID_SAVEAS);
	ADD_PREDEFINED_MACRO(wxID_REVERT);
	ADD_PREDEFINED_MACRO(wxID_EXIT);
	ADD_PREDEFINED_MACRO(wxID_UNDO);
	ADD_PREDEFINED_MACRO(wxID_REDO);
	ADD_PREDEFINED_MACRO(wxID_HELP);
	ADD_PREDEFINED_MACRO(wxID_PRINT);
	ADD_PREDEFINED_MACRO(wxID_PRINT_SETUP);
	ADD_PREDEFINED_MACRO(wxID_PREVIEW);
	ADD_PREDEFINED_MACRO(wxID_ABOUT);
	ADD_PREDEFINED_MACRO(wxID_HELP_CONTENTS);
	ADD_PREDEFINED_MACRO(wxID_HELP_COMMANDS);
	ADD_PREDEFINED_MACRO(wxID_HELP_PROCEDURES);
	ADD_PREDEFINED_MACRO(wxID_HELP_CONTEXT);
	ADD_PREDEFINED_MACRO(wxID_CLOSE_ALL);

	ADD_PREDEFINED_MACRO(wxID_CUT);
	ADD_PREDEFINED_MACRO(wxID_COPY);
	ADD_PREDEFINED_MACRO(wxID_PASTE);
	ADD_PREDEFINED_MACRO(wxID_CLEAR);
	ADD_PREDEFINED_MACRO(wxID_FIND);

	ADD_PREDEFINED_MACRO(wxID_DUPLICATE);
	ADD_PREDEFINED_MACRO(wxID_SELECTALL);
	ADD_PREDEFINED_MACRO(wxID_DELETE);
	ADD_PREDEFINED_MACRO(wxID_REPLACE);
	ADD_PREDEFINED_MACRO(wxID_REPLACE_ALL);
	ADD_PREDEFINED_MACRO(wxID_PROPERTIES);

	ADD_PREDEFINED_MACRO(wxID_VIEW_DETAILS);
	ADD_PREDEFINED_MACRO(wxID_VIEW_LARGEICONS);
	ADD_PREDEFINED_MACRO(wxID_VIEW_SMALLICONS);
	ADD_PREDEFINED_MACRO(wxID_VIEW_LIST);
	ADD_PREDEFINED_MACRO(wxID_VIEW_SORTDATE);
	ADD_PREDEFINED_MACRO(wxID_VIEW_SORTNAME);
	ADD_PREDEFINED_MACRO(wxID_VIEW_SORTSIZE);
	ADD_PREDEFINED_MACRO(wxID_VIEW_SORTTYPE);

	ADD_PREDEFINED_MACRO(wxID_FILE1);
	ADD_PREDEFINED_MACRO(wxID_FILE2);
	ADD_PREDEFINED_MACRO(wxID_FILE3);
	ADD_PREDEFINED_MACRO(wxID_FILE4);
	ADD_PREDEFINED_MACRO(wxID_FILE5);
	ADD_PREDEFINED_MACRO(wxID_FILE6);
	ADD_PREDEFINED_MACRO(wxID_FILE7);
	ADD_PREDEFINED_MACRO(wxID_FILE8);
	ADD_PREDEFINED_MACRO(wxID_FILE9);

	// Standard button IDs
	ADD_PREDEFINED_MACRO(wxID_OK);
	ADD_PREDEFINED_MACRO(wxID_CANCEL);

	ADD_PREDEFINED_MACRO(wxID_APPLY);
	ADD_PREDEFINED_MACRO(wxID_YES);
	ADD_PREDEFINED_MACRO(wxID_NO);
	ADD_PREDEFINED_MACRO(wxID_STATIC);
	ADD_PREDEFINED_MACRO(wxID_FORWARD);
	ADD_PREDEFINED_MACRO(wxID_BACKWARD);
	ADD_PREDEFINED_MACRO(wxID_DEFAULT);
	ADD_PREDEFINED_MACRO(wxID_MORE);
	ADD_PREDEFINED_MACRO(wxID_SETUP);
	ADD_PREDEFINED_MACRO(wxID_RESET);
	ADD_PREDEFINED_MACRO(wxID_CONTEXT_HELP);
	ADD_PREDEFINED_MACRO(wxID_YESTOALL);
	ADD_PREDEFINED_MACRO(wxID_NOTOALL);
	ADD_PREDEFINED_MACRO(wxID_ABORT);
	ADD_PREDEFINED_MACRO(wxID_RETRY);
	ADD_PREDEFINED_MACRO(wxID_IGNORE);

	ADD_PREDEFINED_MACRO(wxID_UP);
	ADD_PREDEFINED_MACRO(wxID_DOWN);
	ADD_PREDEFINED_MACRO(wxID_HOME);
	ADD_PREDEFINED_MACRO(wxID_REFRESH);
	ADD_PREDEFINED_MACRO(wxID_STOP);
	ADD_PREDEFINED_MACRO(wxID_INDEX);

	ADD_PREDEFINED_MACRO(wxID_BOLD);
	ADD_PREDEFINED_MACRO(wxID_ITALIC);
	ADD_PREDEFINED_MACRO(wxID_JUSTIFY_CENTER);
	ADD_PREDEFINED_MACRO(wxID_JUSTIFY_FILL);
	ADD_PREDEFINED_MACRO(wxID_JUSTIFY_RIGHT);

	ADD_PREDEFINED_MACRO(wxID_JUSTIFY_LEFT);
	ADD_PREDEFINED_MACRO(wxID_UNDERLINE);
	ADD_PREDEFINED_MACRO(wxID_INDENT);
	ADD_PREDEFINED_MACRO(wxID_UNINDENT);
	ADD_PREDEFINED_MACRO(wxID_ZOOM_100);
	ADD_PREDEFINED_MACRO(wxID_ZOOM_FIT);
	ADD_PREDEFINED_MACRO(wxID_ZOOM_IN);
	ADD_PREDEFINED_MACRO(wxID_ZOOM_OUT);
	ADD_PREDEFINED_MACRO(wxID_UNDELETE);
	ADD_PREDEFINED_MACRO(wxID_REVERT_TO_SAVED);

	// System menu IDs (used by wxUniv):
	ADD_PREDEFINED_MACRO(wxID_SYSTEM_MENU);
	ADD_PREDEFINED_MACRO(wxID_CLOSE_FRAME);
	ADD_PREDEFINED_MACRO(wxID_MOVE_FRAME);
	ADD_PREDEFINED_MACRO(wxID_RESIZE_FRAME);
	ADD_PREDEFINED_MACRO(wxID_MAXIMIZE_FRAME);
	ADD_PREDEFINED_MACRO(wxID_ICONIZE_FRAME);
	ADD_PREDEFINED_MACRO(wxID_RESTORE_FRAME);

	// IDs used by generic file dialog (13 consecutive starting from this value)

	ADD_PREDEFINED_MACRO(wxID_FILEDLGG);

	ADD_PREDEFINED_MACRO(wxID_HIGHEST);

}
