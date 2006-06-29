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

CppTemplateParser::CppTemplateParser(shared_ptr<ObjectBase> obj, unistring _template)
: TemplateParser(obj,_template)
{
	m_useRelativePath = false;
	m_i18n = false;
}
unistring CppTemplateParser::RootWxParentToCode()
{
	return _T("this");
}

shared_ptr<TemplateParser> CppTemplateParser::CreateParser(shared_ptr<ObjectBase> obj, unistring _template)
{
	shared_ptr<TemplateParser> newparser(new CppTemplateParser(obj,_template));
	return newparser;
}

void CppTemplateParser::UseRelativePath(bool relative, unistring basePath)
{
	m_useRelativePath = relative;

	if (m_useRelativePath)
	{
		bool result = wxFileName::DirExists( basePath.c_str() );
		m_basePath = (result ? basePath : _T(""));
	}
}

void CppTemplateParser::UseI18n(bool i18n)
{
	m_i18n = i18n;
}

/**
* Convert the value of the property to C++ code
*/
unistring CppTemplateParser::ValueToCode( PropertyType type, unistring value )
{
	uniostringstream result;

	switch ( type )
	{
	case PT_WXSTRING:
		{
			result << _T("wxT(\"") << CppCodeGenerator::ConvertCppString( value ) << _T("\")");
			break;
		}
	case PT_WXSTRING_I18N:
		{
			if ( m_i18n )
			{
				result << _T("_(\"") << CppCodeGenerator::ConvertCppString(value) << _T("\")");
			}
			else
			{
				result << _T("wxT(\"") << CppCodeGenerator::ConvertCppString(value) << _T("\")");
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
			result.str( value.empty() ? _T("0") : value );
			break;
		}
	case PT_WXPOINT:
		{
			if ( value.empty() )
			{
				result.str( _T("wxDefaultPosition") );
			}
			else
			{
				result << _T("wxPoint( ") << value << _T(" )");
			}
			break;
		}
	case PT_WXSIZE:
		{
			if ( value.empty() )
			{
				result.str( _T("wxDefaultSize") );
			}
			else
			{
				result << _T("wxSize( ") << value << _T(" )");
			}
			break;
		}
	case PT_BOOL:
		{
			result.str( value == _T("0") ? _T("false") : _T("true") );
			break;
		}
	case PT_WXFONT:
		{
			if ( !value.empty() )
			{
				wxFont font = TypeConv::StringToFont( value.c_str() );
				result	<< _T("wxFont( ")
					<< font.GetPointSize()	<< _T(", ")
					<< font.GetFamily()		<< _T(", ")
					<< font.GetStyle()		<< _T(", ")
					<< font.GetWeight()		<< _T(", ")
					<< ( font.GetUnderlined() ? _T("true, ") : _T("false, ") )
					<< _T("wxT(\"") << font.GetFaceName().c_str() << _T("\") )");
			}
			else
			{
				result.str( _T("wxFont()") );
			}
			break;
		}
	case PT_WXCOLOUR:
		{
			if ( !value.empty() )
			{
				if ( value.find_first_of( _T("wx") ) == 0 )
				{
					// System Colour
					result << _T("wxSystemSettings::GetColour( ") << value << _T(" )");
				}
				else
				{
					wxColour colour = TypeConv::StringToColour( value.c_str() );
					result << _T("wxColour( ") << (int)colour.Red() << _T(", ") << (int)colour.Green() << _T(", ") << (int)colour.Blue() << _T(" )");
				}
			}
			else
			{
				result.str( _T("wxColour()") );
			}
			break;
		}
	case PT_BITMAP:
		{
			// Splitting bitmap resource property value - it is of the form "path; source"
			size_t semicolonIndex = value.find_first_of( _T(";") );
			unistring path;
			unistring source;
			if ( semicolonIndex != value.npos )
			{
				path = value.substr( 0, semicolonIndex );
				source = value.substr( semicolonIndex + 2 ); // Separated by "; "
			}
			else
			{
				path = value;
				source = _T("Load From File");
			}

			if ( path.empty() )
			{
				// Empty path, generate Null Bitmap
				result.str( _T("wxNullBitmap") );
				break;
			}

			if ( source == _T("Load From File") )
			{
				wxString absPath = TypeConv::MakeAbsolutePath( path.c_str(), GlobalData()->GetProjectPath() );
				unistring file = ( m_useRelativePath ? TypeConv::MakeRelativePath( absPath, m_basePath.c_str() ) : absPath ).c_str();

				unistring cppString = CppCodeGenerator::ConvertCppString( file );

				wxFileName bmpFileName( path.c_str() );
				if ( bmpFileName.GetExt().Upper() == wxT("XPM") )
				{
					// If the bitmap is an XPM we will embed it in the code, otherwise it will be loaded from the file at run time.
					result << _T("wxBitmap( ") << CppCodeGenerator::ConvertXpmName( path ) << _T(" )");
				}
				else
				{
					result << _T("wxBitmap( wxT(\"") << cppString << _T("\"), wxBITMAP_TYPE_ANY )");
				}
			}
			else if ( source == _T("Load From Resource") )
			{
				result << _T("wxBitmap( wxT(\"") << path << _T("\"), wxBITMAP_TYPE_RESOURCE )");
			}
			else if ( source == _T("Load From Icon Resource") )
			{
				result << _T("wxICON( ") << path << _T(" )");
			}

			break;
		}
	case PT_STRINGLIST:
		{
			// Stringlists are generated like a sequence of wxString separated by ', '.
			wxArrayString array = TypeConv::StringToArrayString( value.c_str() );
			if ( array.Count() > 0 )
			{
				result.str( ValueToCode( PT_WXSTRING_I18N, array[0].c_str() ) );
			}

			for ( size_t i = 1; i < array.Count(); i++ )
			{
				result << result << _T(", ") << ValueToCode( PT_WXSTRING_I18N, array[i].c_str() );
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

unistring CppCodeGenerator::ConvertCppString( unistring text )
{
	unistring result;

	for ( size_t i = 0; i < text.length(); i++ )
	{
		unichar c = text[i];
		
		switch ( c )
		{
		case _T('"'):
			result += _T("\\\"");
			break;

		case _T('\\'):
			result += _T("\\\\");
			break;

		case _T('\t'):
			result += _T("\\t");
			break;

		case _T('\n'):
			result += _T("\\n");
			break;

		case _T('\r'):
			result += _T("\\r");
			break;

		default:
			result += c;
			break;
		}
	}
	return result;
}

unistring CppCodeGenerator::ConvertXpmName( const unistring& text )
{
	unistring name = text;
	// the name consists of extracting the name of the file (without the directory)
	// and replacing the character '.' by ' _ '.

	size_t last_slash = name.find_last_of( _T("\\/") );
	if ( last_slash != name.npos )
	{
		name = name.substr( last_slash + 1 );
	}

	name.replace( name.rfind( _T(".") ), 1, _T("_") );

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

	shared_ptr< Property > useEnumProperty = project->GetProperty( _T("use_enum") );
	if (useEnumProperty && useEnumProperty->GetValueAsInteger())
		useEnum = true;

	m_i18n = false;
	shared_ptr< Property > i18nProperty = project->GetProperty( _T("internationalize") );
	if (i18nProperty && i18nProperty->GetValueAsInteger())
		m_i18n = true;

	m_header->Clear();
	m_source->Clear();
	unistring date( _T(__DATE__) );
	unistring time( _T(__TIME__) );
	unistring code_header (
		_T("///////////////////////////////////////////////////////////////////////////\n")
		_T("// C++ code generated with wxFormBuilder (version ") _T(__DATE__) _T(")\n")
		_T("// http://wxformbuilder.sourceforge.net/\n")
		_T("//\n")
		_T("// PLEASE DO \"NOT\" EDIT THIS FILE!\n")
		_T("///////////////////////////////////////////////////////////////////////////\n") );

	m_header->WriteLn( code_header );
	m_source->WriteLn( code_header );

	shared_ptr<Property> propFile = project->GetProperty( _T("file") );
	if (!propFile)
	{
		wxLogError( wxT("Missing \"file\" property on Project Object") );
		return false;
	}

	unistring file = propFile->GetValue();
	if ( file.empty() )
	{
		file = _T("noname");
	}

	m_header->WriteLn( _T("#ifndef __") + file + _T("__") );
	m_header->WriteLn( _T("#define __") + file + _T("__") );
	m_header->WriteLn( _T("") );

	code_header = GetCode( project, _T("header_preamble") );
	m_header->WriteLn(code_header);

	// Generate the libraries
	set< unistring > libraries;
	GenLibraries( project, &libraries );

	// Write the library lines
	WriteLibrariesBlock( libraries );

	// generamos en el h los includes de las dependencias de los componentes.
	set<unistring> includes;
	GenIncludes(project, &includes);

	// Write the include lines
	set<unistring>::iterator include_it;
	for ( include_it = includes.begin(); include_it != includes.end(); ++include_it )
	{
		m_header->WriteLn( *include_it );
	}
	if ( !includes.empty() )
	{
		m_header->WriteLn( _T("") );
	}

	code_header = GetCode( project, _T("header_epilogue") );
	m_header->WriteLn( code_header );
	m_header->WriteLn( _T("") );

	// en el cpp generamos el include del .h generado y los xpm
	code_header = GetCode( project, _T("cpp_preamble") );
	m_source->WriteLn( code_header );
	m_source->WriteLn( _T("") );

	m_source->WriteLn( _T("#include \"") + file + _T(".h\""));
	m_source->WriteLn( _T("") );
	GenXpmIncludes( project );

	code_header = GetCode( project, _T("cpp_epilogue") );
	m_source->WriteLn( code_header );

	// generamos los defines de las macros
	if ( !useEnum )
	{
		GenDefines( project );
	}

	for ( unsigned int i = 0; i < project->GetChildCount(); i++ )
	{
		GenClassDeclaration( project->GetChild( i ), useEnum );
		GenConstructor( project->GetChild( i ) );
	}

	m_header->WriteLn( _T("#endif //__") + file + _T("__") );

	return true;
}

void CppCodeGenerator::GenAttributeDeclaration(shared_ptr<ObjectBase> obj, Permission perm)
{
	unistring typeName = obj->GetObjectTypeName();
	if (typeName == _T("notebook")			||
		typeName == _T("flatnotebook")		||
		typeName == _T("listbook")			||
		typeName == _T("choicebook")		||
		typeName == _T("widget")			||
		typeName == _T("expanded_widget")	||
		typeName == _T("statusbar")			||
		typeName == _T("component")			||
		typeName == _T("container")			||
		typeName == _T("menubar")			||
		typeName == _T("toolbar")			||
		typeName == _T("splitter")
		)
	{
		unistring perm_str = obj->GetProperty( _T("permission") )->GetValue();

		if ((perm == P_PUBLIC && perm_str == _T("public") ) ||
			(perm == P_PROTECTED && perm_str == _T("protected") ) ||
			(perm == P_PRIVATE && perm_str == _T("private") ) )
		{
			// Generate the declaration
			unistring code = GetCode( obj, _T("declaration") );
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

unistring CppCodeGenerator::GetCode(shared_ptr<ObjectBase> obj, unistring name)
{
	unistring _template;
	shared_ptr<CodeInfo> code_info = obj->GetObjectInfo()->GetCodeInfo( _T("C++") );
	if (!code_info)
	{
		wxString msg( wxString::Format( wxT("Missing \"%s\" template for \"%s\" class. Review your XML object description"),
			name.c_str(), obj->GetClassName().c_str()));
		wxLogError(msg);
		return _T("");
	}

	_template = code_info->GetTemplate(name);

	CppTemplateParser parser(obj,_template);
	parser.UseRelativePath(m_useRelativePath, m_basePath);
	parser.UseI18n(m_i18n);
	unistring code = parser.ParseTemplate();

	return code;
}

void CppCodeGenerator::GenClassDeclaration(shared_ptr<ObjectBase> class_obj, bool use_enum)
{
	shared_ptr<Property> propName = class_obj->GetProperty( _T("name") );
	if ( !propName )
	{
		wxLogError(wxT("Missing \"name\" property on \"%s\" class. Review your XML object description"),
			class_obj->GetClassName().c_str());
		return;
	}

	unistring class_name = propName->GetValue();
	if ( class_name.empty() )
	{
		wxLogError( wxT("Object name can not be null") );
		return;
	}

	m_header->WriteLn( _T("/**") );
	m_header->WriteLn( _T(" * Class ") + class_name);
	m_header->WriteLn( _T(" */") );

	m_header->WriteLn( _T("class ") + class_name + _T(" : ") + GetCode( class_obj, _T("base") ) );
	m_header->WriteLn( _T("{") );
	m_header->Indent();

	// private
	m_header->WriteLn( _T("private:") );
	m_header->Indent();

	if (use_enum)
		GenEnumIds(class_obj);

	GenAttributeDeclaration(class_obj,P_PRIVATE);
	m_header->Unindent();
	m_header->WriteLn( _T("") );

	// protected
	m_header->WriteLn( _T("protected:") );
	m_header->Indent();
	GenAttributeDeclaration(class_obj,P_PROTECTED);
	m_header->Unindent();
	m_header->WriteLn( _T("") );

	// public
	m_header->WriteLn( _T("public:") );
	m_header->Indent();
	GenAttributeDeclaration(class_obj,P_PUBLIC);

	// The constructor is also included within public
	m_header->WriteLn( GetCode( class_obj, _T("cons_decl") ) );
	m_header->Unindent();
	m_header->WriteLn( _T("") );

	m_header->Unindent();
	m_header->WriteLn( _T("};") );
	m_header->WriteLn( _T("") );
}

void CppCodeGenerator::GenEnumIds(shared_ptr< ObjectBase > class_obj)
{
	set<unistring> macro_set;
	FindMacros(class_obj,macro_set);

	set<unistring>::iterator it = macro_set.begin();
	if ( it != macro_set.end())
	{
		m_header->WriteLn( _T("enum") );
		m_header->WriteLn( _T("{") );
		m_header->Indent();

		uniostringstream firstEnum;
		firstEnum << *it << _T(" = ") << m_firstID << _T(",");
		m_header->WriteLn( firstEnum.str() );
		it++;
		while ( it != macro_set.end() )
		{
			m_header->WriteLn( *it + _T(",") );
			it++;
		}

		//m_header->WriteLn(id);
		m_header->Unindent();
		m_header->WriteLn( _T("};") );
		m_header->WriteLn( _T("") );
	}
}

void CppCodeGenerator::GenIncludes( shared_ptr<ObjectBase> project, set<unistring>* includes)
{
	// Call GenIncludes on all children as well
	for ( unsigned int i = 0; i < project->GetChildCount(); i++ )
	{
		GenIncludes( project->GetChild(i), includes );
	}

	// Fill the set
	shared_ptr<CodeInfo> code_info = project->GetObjectInfo()->GetCodeInfo( _T("C++") );
	if (code_info)
	{
		CppTemplateParser parser(project,code_info->GetTemplate( _T("include") ) );
		unistring include = parser.ParseTemplate();
		if ( !include.empty() )
		{
			includes->insert( include );
		}
	}
}

void CppCodeGenerator::GenLibraries( shared_ptr< ObjectBase > project, set< unistring >* libraries )
{
	// Call GenIncludes on all children as well
	for ( unsigned int i = 0; i < project->GetChildCount(); i++ )
	{
		GenLibraries( project->GetChild(i), libraries );
	}

	// Fill the set
	shared_ptr<CodeInfo> code_info = project->GetObjectInfo()->GetCodeInfo( _T("C++") );
	if (code_info)
	{
		CppTemplateParser parser(project,code_info->GetTemplate( _T("library") ) );
		unistring library = parser.ParseTemplate();
		if ( !library.empty() )
		{
			libraries->insert( library );
		}
	}
}

void CppCodeGenerator::WriteLibrariesBlock( const set< unistring >& libraries )
{
	if ( libraries.empty() )
	{
		return;
	}

	m_header->WriteLn( _T("#ifdef __WXMSW__") );
	m_header->Indent();
		m_header->WriteLn( _T("#ifdef _MSC_VER") );
		m_header->Indent();
			m_header->WriteLn( _T("#ifdef _DEBUG") );
			m_header->Indent();
				m_header->WriteLn( _T("#ifdef UNICODE  // __WXMSW__, _MSC_VER, _DEBUG, UNICODE") );
				m_header->Indent();
					WriteLibraries( libraries, _T("#pragma comment( lib, \"wxmsw26ud_"), _T(".lib\" )") );
				m_header->Unindent();
				m_header->WriteLn( _T("#else  // __WXMSW__, _MSC_VER, _DEBUG") );
				m_header->Indent();
					WriteLibraries( libraries, _T("#pragma comment( lib, \"wxmsw26d_"), _T(".lib\" )") );
				m_header->Unindent();
				m_header->WriteLn( _T("#endif") );
			m_header->Unindent();
			m_header->WriteLn( _T("#else") );
			m_header->Indent();
				m_header->WriteLn( _T("#ifdef UNICODE  // __WXMSW__, _MSC_VER, UNICODE") );
				m_header->Indent();
					WriteLibraries( libraries, _T("#pragma comment( lib, \"wxmsw26u_"), _T(".lib\" )") );
				m_header->Unindent();
				m_header->WriteLn( _T("#else // __WXMSW__, _MSC_VER") );
				m_header->Indent();
					WriteLibraries( libraries, _T("#pragma comment( lib, \"wxmsw26_"), _T(".lib\" )") );
				m_header->Unindent();
				m_header->WriteLn( _T("#endif") );
			m_header->Unindent();
			m_header->WriteLn( _T("#endif") );
		m_header->Unindent();
		m_header->WriteLn( _T("#endif") );
	m_header->Unindent();
	m_header->WriteLn( _T("#endif") );
	m_header->WriteLn( _T("") );
}

void CppCodeGenerator::WriteLibraries( const set< unistring >& libraries, const unistring& prefix, const unistring& suffix )
{
	set< unistring >::const_iterator library;
	uniostringstream line;
	for ( library = libraries.begin(); library != libraries.end(); ++library )
	{
		line << prefix << *library << suffix;
		m_header->WriteLn( line.str() );
		line.str( _T("") );
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
	m_source->WriteLn( _T("") );
	m_source->WriteLn( GetCode( class_obj, _T("cons_def") ) );
	m_source->WriteLn( _T("{") );
	m_source->Indent();

	unistring settings = GetCode( class_obj, _T("settings") );
	if ( !settings.empty() )
	{
		m_source->WriteLn( settings );
	}

	for ( unsigned int i = 0; i < class_obj->GetChildCount(); i++ )
	{
		GenConstruction( class_obj->GetChild( i ), true );
	}

	m_source->Unindent();
	m_source->WriteLn( _T("}") );
}

void CppCodeGenerator::GenConstruction(shared_ptr<ObjectBase> obj, bool is_widget)
{
	unistring type = obj->GetObjectTypeName();

	if (type == _T("notebook")			||
		type == _T("flatnotebook")		||
		type == _T("listbook")			||
		type == _T("choicebook")		||
		type == _T("widget")			||
		type == _T("expanded_widget")	||
		type == _T("statusbar")			||
		type == _T("container")			||
		type == _T("menubar")			||
		type == _T("toolbar")			||
		type == _T("splitter")
		)
	{
		// comprobamos si no se ha declarado como atributo de clase
		// en cuyo caso lo declaramos en el constructor

		unistring perm_str = obj->GetProperty( _T("permission") )->GetValue();
		if ( perm_str == _T("none") )
		{
			m_source->WriteLn( GetCode( obj, _T("declaration") ) );
		}

		m_source->WriteLn( GetCode( obj, _T("construction") ) );
		GenSettings( obj->GetObjectInfo(), obj );

		for ( unsigned int i = 0; i < obj->GetChildCount(); i++ )
		{
			shared_ptr< ObjectBase > child = obj->GetChild( i );
			GenConstruction( child, true );

			if ( type == _T("toolbar") )
			{
				GenAddToolbar(child->GetObjectInfo(), child);
			}
		}

		if ( type == _T("splitter") )
		{
			// generamos el split
			if (obj->GetChildCount() == 2)
			{
				shared_ptr<ObjectBase> sub1,sub2;
				sub1 = obj->GetChild(0)->GetChild(0);
				sub2 = obj->GetChild(1)->GetChild(0);

				unistring _template;
				if ( obj->GetProperty( _T("splitmode") )->GetValue() == _T("wxSPLIT_VERTICAL") )
				{
					_template = _T("$name->SplitVertically(");
				}
				else
				{
					_template = _T("$name->SplitHorizontally(");
				}

				_template = _template + sub1->GetProperty( _T("name") )->GetValue() +
					_T(",") + sub2->GetProperty( _T("name") )->GetValue() + _T(",$sashpos);");

				CppTemplateParser parser(obj,_template);
				parser.UseRelativePath(m_useRelativePath, m_basePath);
				parser.UseI18n(m_i18n);
				m_source->WriteLn(parser.ParseTemplate());
			}
			else
				wxLogError( wxT("Missing subwindows for wxSplitterWindow widget.") );
		}


		if (type == _T("menubar") || type == _T("toolbar") || type == _T("listbook") ||
			type == _T("notebook") || type == _T("flatnotebook") )
		{
			unistring afterAddChild = GetCode( obj, _T("after_addchild") );
			if ( !afterAddChild.empty() )
			{
				m_source->WriteLn( afterAddChild );
			}
			m_source->WriteLn( _T("") );
		}

	}
	else if ( type == _T("sizer") )
	{
		m_source->WriteLn( GetCode( obj, _T("declaration") ) );
		m_source->WriteLn( GetCode( obj, _T("construction") ) );
		GenSettings( obj->GetObjectInfo(), obj );

		for ( unsigned int i = 0; i < obj->GetChildCount(); i++ )
		{
			GenConstruction(obj->GetChild(i),false);
		}

		if (is_widget)
		{
			// the parent object is not a sizer. There is no template for
			// this so we'll make it manually.
			// It's not a good practice to embed templates into the source code,
			// because you will need to recompile...

			unistring _template =	_T("#wxparent $name->SetSizer( $name );\n")
									_T("#wxparent $name->Layout();");
			

			CppTemplateParser parser(obj,_template);
			parser.UseRelativePath(m_useRelativePath, m_basePath);
			parser.UseI18n(m_i18n);
			m_source->WriteLn(parser.ParseTemplate());
		}
	}
	else if ( type == _T("menu") || type == _T("submenu") )
	{
		m_source->WriteLn( GetCode( obj, _T("declaration") ) );
		m_source->WriteLn( GetCode( obj, _T("construction") ) );

		for ( unsigned int i = 0; i < obj->GetChildCount(); i++ )
		{
			GenConstruction( obj->GetChild(i), false );
		}

		m_source->WriteLn( GetCode( obj, _T("menu_add") ) );

	}
	else if ( type == _T("spacer") )
	{
		// En lugar de modelar un "spacer" como un objeto que se incluye en
		// un sizer item, los vamos a considerar un como un tipo de
		// de "sizeritem" capaz de existir por sí solo. De esta forma será
		// más facil la exportación XRC.
		m_source->WriteLn( GetCode( obj, _T("spacer_add") ) );
	}
	else if ( type == _T("sizeritem") )
	{
		// El hijo, hay que añadirlo al sizer teniendo en cuenta el tipo
		// del objeto hijo (hay 3 rutinas diferentes)
		GenConstruction(obj->GetChild(0),false);

		unistring child_type = obj->GetChild(0)->GetObjectTypeName();
		unistring temp_name;
		if (child_type == _T("notebook")		||
			child_type == _T("flatnotebook")	||
			child_type == _T("listbook")		||
			child_type == _T("choicebook")		||
			child_type == _T("widget")			||
			child_type == _T("expanded_widget")	||
			child_type == _T("statusbar")		||
			child_type == _T("container")		||
			child_type == _T("splitter")
			)
		{
			temp_name = _T("window_add");
		}
		else if ( child_type == _T("sizer") )
		{
			temp_name = _T("sizer_add");
		}
		else
		{
			assert( false );
		}

		m_source->WriteLn( GetCode( obj, temp_name ) );
	}
	else if (	type == _T("notebookpage")		||
				type == _T("flatnotebookpage")	||
				type == _T("listbookpage")		||
				type == _T("choicebookpage")
			)
	{
		GenConstruction( obj->GetChild( 0 ), false );
		m_source->WriteLn( GetCode( obj, _T("page_add") ) );
		GenSettings( obj->GetObjectInfo(), obj );
	}
	else if ( type == _T("menuitem") )
	{
		m_source->WriteLn( GetCode( obj, _T("construction") ) );
		m_source->WriteLn( GetCode( obj, _T("menuitem_add") ) );
		GenSettings( obj->GetObjectInfo(), obj );
	}
	else if ( type == _T("tool") )
	{
		m_source->WriteLn( GetCode( obj, _T("construction") ) );
	}
	else
	{
		// Generate the children
		for ( unsigned int i = 0; i < obj->GetChildCount(); i++ )
		{
			GenConstruction( obj->GetChild( i ), false );
		}
	}
}

void CppCodeGenerator::FindMacros( shared_ptr< ObjectBase > obj, set< unistring >& macro_set )
{
	// recorre cada propiedad de cada objeto identificando aquellas
	// que sean macros, en cuyo caso la añade al conjunto.
	unsigned int i;

	for ( i = 0; i < obj->GetPropertyCount(); i++ )
	{
		shared_ptr<Property> prop = obj->GetProperty( i );
		if ( prop->GetType() == PT_MACRO )
		{
			unistring value = prop->GetValue();
			set< unistring >::iterator it = m_predMacros.find( value );
			if ( it == m_predMacros.end() )
			{
				macro_set.insert( prop->GetValue() );
			}
		}
	}

	for ( i = 0; i < obj->GetChildCount(); i++ )
	{
		FindMacros( obj->GetChild( i ), macro_set );
	}
}

void CppCodeGenerator::GenDefines( shared_ptr< ObjectBase > project)
{
	set< unistring > macro_set;
	FindMacros( project, macro_set );

	// la macro por defecto tiene valor -1
	m_header->WriteLn( _T("#define ID_DEFAULT wxID_ANY // Default") );

	// debemos quitar la macro por defecto del conjunto
	set<unistring>::iterator it;
	it = macro_set.find( _T("ID_DEFAULT") );
	if ( it != macro_set.end() )
	{
		macro_set.erase(it);
	}

	unsigned int id = m_firstID;
	if ( id < 1000 )
	{
		wxLogWarning(wxT("First ID is Less than 1000"));
	}
	for (it = macro_set.begin() ; it != macro_set.end(); it++)
	{
		uniostringstream define;
		define << _T("#define ") << *it << _T(" ") << id;
		m_header->WriteLn(define.str());
		id++;
	}

	m_header->WriteLn( _T("") );
}

void CppCodeGenerator::GenSettings(shared_ptr<ObjectInfo> info, shared_ptr<ObjectBase> obj)
{
	unistring _template;
	shared_ptr<CodeInfo> code_info = info->GetCodeInfo( _T("C++") );

	if ( !code_info )
	{
		return;
	}

	_template = code_info->GetTemplate( _T("settings") );

	if ( !_template.empty() )
	{
		CppTemplateParser parser(obj,_template);
		parser.UseRelativePath(m_useRelativePath, m_basePath);
		parser.UseI18n(m_i18n);
		unistring code = parser.ParseTemplate();
		if ( !code.empty() )
		{
			m_source->WriteLn(code);
		}
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
	unistring _template;
	shared_ptr<CodeInfo> code_info = info->GetCodeInfo( _T("C++") );

	if (!code_info)
		return;

	_template = code_info->GetTemplate( _T("toolbar_add") );

	if ( !_template.empty() )
	{
		CppTemplateParser parser(obj,_template);
		parser.UseRelativePath(m_useRelativePath, m_basePath);
		parser.UseI18n(m_i18n);
		unistring code = parser.ParseTemplate();
		if ( !code.empty() )
		{
			m_source->WriteLn(code);
		}
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
	set< unistring > include_set;

	// lo primero es obtener la lista de includes
	FindXpmProperties( project, include_set );

	if ( include_set.empty() )
	{
		return;
	}

	// y los generamos
	set<unistring>::iterator it;
	for ( it = include_set.begin() ; it != include_set.end(); it++ )
	{
		if ( !it->empty() )
		{
			m_source->WriteLn( *it );
		}
	}

	m_source->WriteLn( _T("") );
}

void CppCodeGenerator::FindXpmProperties( shared_ptr<ObjectBase> obj, set<unistring>& set )
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
			unistring path = property->GetValue();
			size_t semicolonindex = path.find_first_of( _T(";") );
			if ( semicolonindex != path.npos )
			{
				path = path.substr( 0, semicolonindex );
			}

			wxFileName bmpFileName( path.c_str() );
			if ( bmpFileName.GetExt().Upper() == wxT("XPM") )
			{
				wxString absPath = TypeConv::MakeAbsolutePath( path.c_str(), GlobalData()->GetProjectPath() );

				// Se supone el path contiene la ruta completa del archivo y no
				// una relativa.
				unistring relPath = ( m_useRelativePath ? TypeConv::MakeRelativePath( absPath, GlobalData()->GetProjectPath() ).c_str() : absPath );

				unistring inc = _T("#include \"") + ConvertCppString( relPath ) + _T("\"");
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

void CppCodeGenerator::UseRelativePath(bool relative, unistring basePath)
{
	bool result;
	m_useRelativePath = relative;

	if (m_useRelativePath)
	{
		result = wxFileName::DirExists( basePath.c_str() );
		m_basePath = ( result ? basePath : _T("") );
	}
}
/*
unistring CppCodeGenerator::ConvertToRelativePath(unistring path, unistring basePath)
{
unistring auxPath = path;
if (basePath != "")
{
wxFileName filename(_WXSTR(auxPath));
if (filename.MakeRelativeTo(_WXSTR(basePath)))
auxPath = _STDSTR(filename.GetFullPath());
}
return auxPath;
}*/

#define ADD_PREDEFINED_MACRO(x) m_predMacros.insert( _T(#x) )
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
