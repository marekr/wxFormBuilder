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

#include "codegen.h"
#include "utils/debug.h"
#include "utils/typeconv.h"
#include "wx/wx.h"
#include <wx/tokenzr.h>

void CodeWriter::WriteLn(unistring code)
{
	// no se permitirán saltos de linea dentro de "code"
	// si los hubiera, FixWrite toma la cadena y la trocea en líneas
	// e inserta una a una mediante WriteLn
	if ( !StringOk( code ) )
	{
		FixWrite( code );
	}
	else
	{
		Write( code );
		Write( _T("\n") );
		m_cols = 0;
	}
}  

bool CodeWriter::StringOk(unistring s)
{
	if ( s.find( _T("\n"), 0 ) == unistring::npos )
	{
		return true;
	}
	else
	{
		return false;
	}
}

void CodeWriter::FixWrite(unistring s)
{
	wxString str = s.c_str();

	wxStringTokenizer tkz( str, wxT("\n"), wxTOKEN_RET_EMPTY_ALL );
	bool prev_is_null = false;

	while ( tkz.HasMoreTokens() )
	{
		wxString line = tkz.GetNextToken();
		line.Trim( false );
		line.Trim( true );

		if ( !line.empty() || !prev_is_null )
		{
			WriteLn( line.c_str() );
		}

		//prev_is_null = line.empty();
	}
}


void CodeWriter::Write(unistring code)
{
	if (m_cols == 0)
	{
		// insertamos el indentado
		for ( int i = 0; i < m_indent; i++ )
		{
			DoWrite( _T("\t") );
		}

		m_cols = m_indent;
	}


	// aquí debemos comprobar si hemos sobrepasado el maximo de columnas
	//  if (m_cols + code.length() > GetColumns())
	//    BreakLine(code)


	DoWrite( code );
}  

TemplateParser::TemplateParser(shared_ptr<ObjectBase> obj, unistring _template)
: m_obj(obj), m_in(_template)
{
}

TemplateParser::Token TemplateParser::GetNextToken()
{
	// There are 3 special characters
	// #xxxx -> command
	// $xxxx -> property
	// %xxxx -> local variable
	// @x -> Escape a special character. Example: @# is the character #.

	Token result = TOK_ERROR;

	if (!m_in.eof())
	{
		unichar c = m_in.peek();
		if ( c == _T('#') )
			result = TOK_MACRO;
		else if (c == _T('$') )
			result = TOK_PROPERTY;
		else
			result = TOK_TEXT;
	}

	return result;
}


bool TemplateParser::ParseMacro()
{
	Ident ident;

	ident = ParseIdent();
	switch (ident)
	{
	case ID_WXPARENT:
		return ParseWxParent();
		break;
	case ID_PARENT:
		return ParseParent();
		break;
	case ID_IFNOTNULL:
		return ParseIfNotNull();
		break;
	case ID_IFNULL:
		return ParseIfNull();
		break;
	case ID_FOREACH:
		return ParseForEach();
		break;
	case ID_PREDEFINED:
		return ParsePred();
		break;
	case ID_CHILD:
		return ParseChild();
		break;
	case ID_NEWLINE:
		return ParseNewLine();
		break;
	case ID_IFEQUAL:
		ParseIfEqual();
		break;
	case ID_IFNOTEQUAL:
		ParseIfNotEqual();
		break;
	case ID_APPEND:
		ParseAppend();
		break;
	default:
		assert(false);
		return false;
		break;
	}

	return true;
}


TemplateParser::Ident TemplateParser::ParseIdent()
{
	Ident ident;

	if (!m_in.eof())
	{
		uniostringstream macro;
		m_in.get();

		while (m_in.peek() != uniistringstream::traits_type::eof() && m_in.peek() != _T('#') && m_in.peek() != _T('$')
			&& ( (m_in.peek() >= _T('a') && m_in.peek() <= _T('z') ) ||
			(m_in.peek() >= _T('A') && m_in.peek() <= _T('Z') ) ||
			(m_in.peek() >= _T('0') && m_in.peek() <= _T('9') )))
		{
			unichar c = m_in.get();
			macro << c;
		}

		// buscar el identificador
		ident = SearchIdent(macro.str());
	}
	return ident;
}

unistring TemplateParser::ParsePropertyName()
{
	unistring propname;

	if (!m_in.eof())
	{
		uniostringstream propstream;
		m_in.get();

		while (m_in.peek() != uniistringstream::traits_type::eof() && m_in.peek() != _T('#') && m_in.peek() != _T('$')
			&& ( (m_in.peek() >= _T('a') && m_in.peek() <= _T('z') ) ||
			(m_in.peek() >= _T('A') && m_in.peek() <= _T('Z') ) ||
			(m_in.peek() >= _T('0') && m_in.peek() <= _T('9') ) ||
			m_in.peek() == _T('_') ) )
		{
			unichar c = m_in.get(); 
			propstream << c << flush;
		}

		// buscar el comando
		propname = propstream.str();
	}
	return propname;
}

bool TemplateParser::ParseProperty()
{
	unistring propname = ParsePropertyName();

	shared_ptr<Property> property = m_obj->GetProperty(propname);
	if ( property.get() != NULL )
	{
		m_out << PropertyToCode(property);
	}
	else
	{
		wxLogError( wxT("Property does not exist: %s"), propname.c_str() );
	}

	//  Debug::Print("parsing property %s",propname.c_str());

	return true;
}

bool TemplateParser::ParseText()
{
	uniostringstream aux;

	while (m_in.peek() != uniistringstream::traits_type::eof() && m_in.peek() != _T('#') && m_in.peek() != _T('$') )
	{
		unichar c = m_in.get();
		if (c == _T('@') )
			c = m_in.get();

		aux << c;
	}
	
	// If text is all whitespace, ignore it
	unistring text = aux.str();
	if ( text.find_first_not_of( _T("\r\n\t ") ) != text.npos )
	{
		m_out << text;
	}

	//  Debug::Print("Parsed Text: %s",aux.str().c_str());
	return true;
}

bool TemplateParser::ParseInnerTemplate()
{
	return true;
}

shared_ptr< ObjectBase > TemplateParser::GetWxParent()
{
	shared_ptr<ObjectBase> wxparent;

	vector< shared_ptr<ObjectBase> > candidates;
	candidates.push_back( m_obj->FindNearAncestor( _T("container") ) );
	candidates.push_back( m_obj->FindNearAncestor( _T("notebook") ) );
	candidates.push_back( m_obj->FindNearAncestor( _T("splitter") ) );
	candidates.push_back( m_obj->FindNearAncestor( _T("flatnotebook") ) );
	candidates.push_back( m_obj->FindNearAncestor( _T("listbook") ) );
	candidates.push_back( m_obj->FindNearAncestor( _T("choicebook") ) );

	for ( size_t i = 0; i < candidates.size(); i++ )
	{
		if ( !wxparent )
		{
			wxparent = candidates[i];
		}
		else
		{
			if ( candidates[i] && candidates[i]->Deep() > wxparent->Deep() )
			{
				wxparent = candidates[i];
			}
		}
	}

	return wxparent;
}

bool TemplateParser::ParseWxParent()
{
	shared_ptr<ObjectBase> wxparent( GetWxParent() );

	if ( wxparent )
	{
		shared_ptr<Property> property = GetRelatedProperty( wxparent );
		m_out << PropertyToCode(property);
	}
	else
	{
		ignore_whitespaces();
		ParsePropertyName();
		m_out << RootWxParentToCode();
	}

	return true;
}

bool TemplateParser::ParseParent()
{
	shared_ptr<ObjectBase> parent(m_obj->GetParent());
	if (parent)
	{
		shared_ptr<Property> property = GetRelatedProperty( parent );
		m_out << PropertyToCode(property);
	}
	else
	{
		m_out << _T("ERROR");
	}

	return true;
}

bool TemplateParser::ParseChild()
{
	// Get the first child
	shared_ptr<ObjectBase> child(m_obj->GetChild(0));

	if (child)
	{
		shared_ptr<Property> property = GetRelatedProperty( child );
		m_out << PropertyToCode(property);
	}
	else
		m_out << RootWxParentToCode();

	return true;
}

shared_ptr<Property> TemplateParser::GetRelatedProperty( shared_ptr<ObjectBase> relative )
{
	ignore_whitespaces();
	unistring propname = ParsePropertyName();
	return relative->GetProperty( propname );
}

bool TemplateParser::ParseForEach()
{
	// ignoramos los espacios que pudiera haber al principio
	ignore_whitespaces();

	// parseamos la propiedad
	if (GetNextToken() == TOK_PROPERTY)
	{
		unistring propname = ParsePropertyName();
		unistring inner_template = ExtractInnerTemplate();

		shared_ptr<Property> property = m_obj->GetProperty(propname);
		unistring propvalue = property->GetValue();

		// el valor de la propiedad debe ser una cadena de caracteres
		// separada por ','. Se va a generar la plantilla anidada tantas
		// veces como tokens se encuentren el el valor de la propiedad.

		if (property->GetType() == PT_INTLIST)
		{
			// Para ello se utiliza la clase wxStringTokenizer de wxWidgets
			wxStringTokenizer tkz( propvalue.c_str(), wxT(","));
			while (tkz.HasMoreTokens())
			{
				wxString token;
				token = tkz.GetNextToken();
				token.Trim(true);
				token.Trim(false);

				// parseamos la plantilla interna
				{
					unistring code;
					shared_ptr<TemplateParser> parser = CreateParser(m_obj,inner_template);
					parser->SetPredefined( unistring(token.c_str()) );
					code = parser->ParseTemplate();
					m_out << endl << code;        
				}
			}
		}
		else if (property->GetType() == PT_STRINGLIST)
		{
			wxArrayString array = property->GetValueAsArrayString();
			for (unsigned int i=0 ; i<array.Count() ; i++)
			{
				unistring code;
				shared_ptr<TemplateParser> parser = CreateParser(m_obj,inner_template);
				parser->SetPredefined(ValueToCode(PT_WXSTRING_I18N,unistring(array[i].c_str())));
				code = parser->ParseTemplate();
				m_out << endl << code;        
			}
		}
		else
			wxLogError(wxT("Property type not compatible with \"foreach\" macro"));
	}

	return true;
}

shared_ptr< Property > TemplateParser::GetProperty()
{
	shared_ptr< Property > property( (Property*)NULL );

	// Check for #wxparent, #parent, or #child
	if ( GetNextToken() == TOK_MACRO )
	{
		Ident ident = ParseIdent();
		switch (ident)
		{
			case ID_WXPARENT:
			{
				shared_ptr<ObjectBase> wxparent( GetWxParent() );
				if ( wxparent )
				{
					property = GetRelatedProperty( wxparent );
				}
				break;
			}
			case ID_PARENT:
			{
				shared_ptr<ObjectBase> parent( m_obj->GetParent() );
				if ( parent )
				{
					property = GetRelatedProperty( parent );
				}
				break;
			}
			case ID_CHILD:
			{
				shared_ptr<ObjectBase> child( m_obj->GetChild( 0 ) );
				if ( child )
				{
					property = GetRelatedProperty( child );
				}
				break;
			}
			default:
				break;
		}
	}

	if ( !property )
	{
		if ( GetNextToken() == TOK_PROPERTY )
		{
			unistring propname = ParsePropertyName();
			property = m_obj->GetProperty( propname );
		}
	}

	return property;
}

void TemplateParser::ignore_whitespaces()
{
	while ( m_in.peek() != uniistringstream::traits_type::eof() && m_in.peek() == _T(' ') )
	{
		m_in.get();
	}
}


bool TemplateParser::ParseIfNotNull()
{      
	ignore_whitespaces();

	// Get the property
	shared_ptr< Property > property( GetProperty() );
	if ( !property )
	{
		return false;
	}

	unistring inner_template = ExtractInnerTemplate();

	if ( !property->GetValue().empty() )
	{
		// Generate the code from the block
		shared_ptr< TemplateParser > parser = CreateParser( m_obj, inner_template );
		m_out << parser->ParseTemplate();
	}

	return true;
}

bool TemplateParser::ParseIfNull()
{      
	ignore_whitespaces();

	// Get the property
	shared_ptr< Property > property( GetProperty() );
	if ( !property )
	{
		return false;
	}

	unistring inner_template = ExtractInnerTemplate();

	if ( property->GetValue().empty() )
	{
		// Generate the code from the block
		shared_ptr< TemplateParser > parser = CreateParser( m_obj, inner_template );
		m_out << parser->ParseTemplate();
	}

	return true;
}

unistring TemplateParser::ExtractLiteral()
{
	uniostringstream os;

	unichar c;

	// ignoramos los espacios que pudiera haber al principio
	ignore_whitespaces();

	c = m_in.get(); // comillas de inicio

	if ( c == _T('"') )
	{
		bool end = false;
		// comenzamos la extracción de la plantilla  
		while (!end && m_in.peek() != uniistringstream::traits_type::eof())
		{
			c = m_in.get(); // extraemos un caracter

			// comprobamos si estamos ante un posible cierre de comillas
			if ( c == _T('"') )
			{
				if ( m_in.peek() == _T('"') ) // caracter (") denotado por ("")
				{
					m_in.get(); // ignoramos la segunda comilla
					os << _T('"');
				}
				else // cierre
				{
					end = true;

					// ignoramos todo los caracteres siguientes hasta un espacio
					// así errores como "hola"mundo" -> "hola"
					while (m_in.peek() != uniistringstream::traits_type::eof() && m_in.peek() != _T(' ') )
						m_in.get();
				}
			}
			else // un caracter del literal
				os << c;
		}
	}  

	return os.str();
}

bool TemplateParser::ParseIfEqual()
{
	// ignore leading whitespace
	ignore_whitespaces();

	// Get the property
	shared_ptr< Property > property( GetProperty() );
	if ( property )
	{
		// Get the value to compare to
		unistring value = ExtractLiteral();

		// Get the template to generate if comparison is true
		unistring inner_template = ExtractInnerTemplate();

		if ( property->GetValue() == value )
		{
			// Generate the code
			shared_ptr<TemplateParser> parser = CreateParser(m_obj,inner_template);
			m_out << parser->ParseTemplate();
			return true;
		}
	}
	return false;
}

bool TemplateParser::ParseIfNotEqual()
{
	// ignore leading whitespace
	ignore_whitespaces();

	// Get the property
	shared_ptr< Property > property( GetProperty() );
	if ( property )
	{
		// Get the value to compare to
		unistring value = ExtractLiteral();

		// Get the template to generate if comparison is false
		unistring inner_template = ExtractInnerTemplate();

		if ( property->GetValue() != value )
		{
			// Generate the code
			shared_ptr<TemplateParser> parser = CreateParser( m_obj, inner_template );
			m_out << parser->ParseTemplate();;
			return true;
		}
	}

	return false;
}

TemplateParser::Ident TemplateParser::SearchIdent(unistring ident)
{
	//  Debug::Print("Parsing command %s",ident.c_str());

	if (ident == _T("wxparent") )
		return ID_WXPARENT;
	else if (ident == _T("ifnotnull") )
		return ID_IFNOTNULL;
	else if (ident == _T("ifnull") )
		return ID_IFNULL;
	else if (ident == _T("foreach") )
		return ID_FOREACH;
	else if (ident == _T("pred") )
		return ID_PREDEFINED;
	else if (ident == _T("child") )
		return ID_CHILD;
	else if (ident == _T("parent") )
		return ID_PARENT;
	else if (ident == _T("nl") )
		return ID_NEWLINE;
	else if (ident == _T("ifequal") )
		return ID_IFEQUAL;
	else if (ident == _T("ifnotequal") )
		return ID_IFNOTEQUAL;
	else if (ident == _T("append") )
		return ID_APPEND;
	else
		return ID_ERROR;  
}

unistring TemplateParser::ParseTemplate()
{
	while (!m_in.eof())
	{
		Token token = GetNextToken();
		switch (token)
		{
		case TOK_MACRO:
			ParseMacro();
			break;
		case TOK_PROPERTY:
			ParseProperty();
			break;
		case TOK_TEXT:
			ParseText();
			break;
		default:
			return _T("");
		}
	}

	return m_out.str();
}

/**
* Extrae la plantilla encerrada entre '@{' y '@}'.
* Nota: Los espacios al comienzo serán ignorados.
*/
unistring TemplateParser::ExtractInnerTemplate()
{
	//  bool error = false;
	uniostringstream os;

	unichar c1, c2;

	// ignoramos los espacios que pudiera haber al principio
	ignore_whitespaces();

	// los dos caracteres siguientes deberán ser '@{'
	c1 = m_in.get();
	c2 = m_in.get();

	if (c1 == _T('@') && c2 == _T('{') )
	{
		ignore_whitespaces();

		int level = 1;
		bool end = false;
		// comenzamos la extracción de la plantilla  
		while (!end && m_in.peek() != uniistringstream::traits_type::eof())
		{
			c1 = m_in.get();

			// comprobamos si estamos ante un posible cierre o apertura de llaves.
			if (c1 == _T('@') )
			{
				c2 = m_in.get();

				if (c2 == _T('}') )
				{
					level--;
					if (level == 0)
						end = true;
					else
					{
						// no es el cierre final, por tanto metemos los caracteres
						// y seguimos
						os << c1;
						os << c2;
					}
				}
				else
				{
					os << c1;
					os << c2;

					if (c2 == _T('{') )
						level++;
				}
			}
			else
				os << c1;
		}
	}  

	return os.str();
}


bool TemplateParser::ParsePred()
{
	if (m_pred != _T("") )
		m_out << m_pred;

	return true;
}

bool TemplateParser::ParseNewLine()
{
	m_out << _T('\n');
	return true;
}

void TemplateParser::ParseAppend()
{
	ignore_whitespaces();
}

unistring TemplateParser::PropertyToCode(shared_ptr<Property> property)
{
	return ValueToCode(property->GetType(), property->GetValue());
}

//////////////////////////////////////////////////////////////////////////////
CodeWriter::CodeWriter()
{
	m_indent = 0;
	m_cols = 0;
}  
