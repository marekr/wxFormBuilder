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

void CodeWriter::WriteLn(string code)
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
		Write( "\n" );
		m_cols = 0;
	}
}  

bool CodeWriter::StringOk(string s)
{
	if (s.find("\n",0) == string::npos)
		return true;
	else
		return false;
}

void CodeWriter::FixWrite(string s)
{
	wxString str = _WXSTR(s);

	wxStringTokenizer tkz( str, wxT("\n"), wxTOKEN_RET_EMPTY_ALL );
	bool prev_is_null = false;

	while ( tkz.HasMoreTokens() )
	{
		wxString line = tkz.GetNextToken();
		line.Trim( false );
		line.Trim( true );

		if ( !line.empty() || !prev_is_null )
		{
			WriteLn( _STDSTR(line) );
		}

		//prev_is_null = line.empty();
	}
}


void CodeWriter::Write(string code)
{
	if (m_cols == 0)
	{
		// insertamos el indentado
		for (int i=0 ; i< m_indent ; i++)
			DoWrite("\t");

		m_cols = m_indent;
	}


	// aquí debemos comprobar si hemos sobrepasado el maximo de columnas
	//  if (m_cols + code.length() > GetColumns())
	//    BreakLine(code)


	DoWrite(code);
}  

TemplateParser::TemplateParser(shared_ptr<ObjectBase> obj, string _template)
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
		char c = m_in.peek();
		if (c == '#')
			result = TOK_MACRO;
		else if (c == '$')
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
		ostringstream macro;
		m_in.get();

		while (m_in.peek() != EOF && m_in.peek() != '#' && m_in.peek() != '$'
			&& ( (m_in.peek() >= 'a' && m_in.peek() <= 'z') ||
			(m_in.peek() >= 'A' && m_in.peek() <= 'Z') ||
			(m_in.peek() >= '0' && m_in.peek() <= '9')))
		{
			char c = m_in.get();
			macro << c;
		}

		// buscar el identificador
		ident = SearchIdent(macro.str());
	}
	return ident;
}

string TemplateParser::ParsePropertyName()
{
	string propname;

	if (!m_in.eof())
	{
		ostringstream propstream;
		m_in.get();

		while (m_in.peek() != EOF && m_in.peek() != '#' && m_in.peek() != '$'
			&& ( (m_in.peek() >= 'a' && m_in.peek() <= 'z') ||
			(m_in.peek() >= 'A' && m_in.peek() <= 'Z') ||
			(m_in.peek() >= '0' && m_in.peek() <= '9') ||
			m_in.peek() == '_'))
		{
			char c = m_in.get(); 
			propstream << c << flush;
		}

		// buscar el comando
		propname = propstream.str();
	}
	return propname;
}

bool TemplateParser::ParseProperty()
{
	string propname = ParsePropertyName();

	shared_ptr<Property> property = m_obj->GetProperty(propname);
	if ( property.get() != NULL )
	{
		m_out << PropertyToCode(property);
	}
	else
	{
		wxLogError( wxT("The property '%s' does not exist for objects of class '%s'"), _WXSTR(propname).c_str(), _WXSTR(m_obj->GetClassName()).c_str() );
	}

	//  Debug::Print("parsing property %s",propname.c_str());

	return true;
}

bool TemplateParser::ParseText()
{
	ostringstream aux;

	while (m_in.peek() != EOF && m_in.peek() != '#' && m_in.peek() != '$')
	{
		char c = m_in.get();
		if (c == '@')
			c = m_in.get();

		aux << c;
	}
	
	// If text is all whitespace, ignore it
	std::string text = aux.str();
	if ( text.find_first_not_of( "\r\n\t " ) != text.npos )
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

	std::vector< shared_ptr<ObjectBase> > candidates;
	candidates.push_back( m_obj->FindNearAncestor("container") );
	candidates.push_back( m_obj->FindNearAncestor("notebook") );
	candidates.push_back( m_obj->FindNearAncestor("splitter") );
	candidates.push_back( m_obj->FindNearAncestor("flatnotebook") );
	candidates.push_back( m_obj->FindNearAncestor("listbook") );
	candidates.push_back( m_obj->FindNearAncestor("choicebook") );

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
		m_out << "ERROR";

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
	string propname = ParsePropertyName();
	return relative->GetProperty( propname );
}

bool TemplateParser::ParseForEach()
{
	// ignoramos los espacios que pudiera haber al principio
	ignore_whitespaces();

	// parseamos la propiedad
	if (GetNextToken() == TOK_PROPERTY)
	{
		string propname = ParsePropertyName();
		string inner_template = ExtractInnerTemplate();

		shared_ptr<Property> property = m_obj->GetProperty(propname);
		string propvalue = property->GetValue();

		// el valor de la propiedad debe ser una cadena de caracteres
		// separada por ','. Se va a generar la plantilla anidada tantas
		// veces como tokens se encuentren el el valor de la propiedad.

		if (property->GetType() == PT_INTLIST)
		{
			// Para ello se utiliza la clase wxStringTokenizer de wxWidgets
			wxStringTokenizer tkz( _WXSTR(propvalue), wxT(","));
			while (tkz.HasMoreTokens())
			{
				wxString token;
				token = tkz.GetNextToken();
				token.Trim(true);
				token.Trim(false);

				// parseamos la plantilla interna
				{
					string code;
					shared_ptr<TemplateParser> parser = CreateParser(m_obj,inner_template);
					parser->SetPredefined(string(token.mb_str()));
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
				string code;
				shared_ptr<TemplateParser> parser = CreateParser(m_obj,inner_template);
				parser->SetPredefined(ValueToCode(PT_WXSTRING_I18N,string(array[i].mb_str())));
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
			string propname = ParsePropertyName();
			property = m_obj->GetProperty( propname );
		}
	}

	return property;
}

void TemplateParser::ignore_whitespaces()
{
	while (m_in.peek() != EOF && m_in.peek() == ' ')
		m_in.get();
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

	string inner_template = ExtractInnerTemplate();

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

	string inner_template = ExtractInnerTemplate();

	if ( property->GetValue().empty() )
	{
		// Generate the code from the block
		shared_ptr< TemplateParser > parser = CreateParser( m_obj, inner_template );
		m_out << parser->ParseTemplate();
	}

	return true;
}

string TemplateParser::ExtractLiteral()
{
	ostringstream os;

	char c;

	// ignoramos los espacios que pudiera haber al principio
	ignore_whitespaces();

	c = m_in.get(); // comillas de inicio

	if (c == '"')
	{
		bool end = false;
		// comenzamos la extracción de la plantilla  
		while (!end && m_in.peek() != EOF)
		{
			c = m_in.get(); // extraemos un caracter

			// comprobamos si estamos ante un posible cierre de comillas
			if (c == '"')
			{
				if (m_in.peek() == '"') // caracter (") denotado por ("")
				{
					m_in.get(); // ignoramos la segunda comilla
					os << '"';
				}
				else // cierre
				{
					end = true;

					// ignoramos todo los caracteres siguientes hasta un espacio
					// así errores como "hola"mundo" -> "hola"
					while (m_in.peek() != EOF && m_in.peek() != ' ')
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
		string value = ExtractLiteral();

		// Get the template to generate if comparison is true
		string inner_template = ExtractInnerTemplate();

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
		string value = ExtractLiteral();

		// Get the template to generate if comparison is false
		string inner_template = ExtractInnerTemplate();

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

TemplateParser::Ident TemplateParser::SearchIdent(string ident)
{
	//  Debug::Print("Parsing command %s",ident.c_str());

	if (ident == "wxparent")
		return ID_WXPARENT;
	else if (ident == "ifnotnull")
		return ID_IFNOTNULL;
	else if (ident == "ifnull")
		return ID_IFNULL;
	else if (ident == "foreach")
		return ID_FOREACH;
	else if (ident == "pred")
		return ID_PREDEFINED;
	else if (ident == "child")
		return ID_CHILD;
	else if (ident == "parent")
		return ID_PARENT;
	else if (ident == "nl")
		return ID_NEWLINE;
	else if (ident == "ifequal")
		return ID_IFEQUAL;
	else if (ident == "ifnotequal")
		return ID_IFNOTEQUAL;
	else if (ident == "append")
		return ID_APPEND;
	else
		return ID_ERROR;  
}

string TemplateParser::ParseTemplate()
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
			return "";
		}
	}

	return m_out.str();
}

/**
* Extrae la plantilla encerrada entre '@{' y '@}'.
* Nota: Los espacios al comienzo serán ignorados.
*/
string TemplateParser::ExtractInnerTemplate()
{
	//  bool error = false;
	ostringstream os;

	char c1, c2;

	// ignoramos los espacios que pudiera haber al principio
	ignore_whitespaces();

	// los dos caracteres siguientes deberán ser '@{'
	c1 = m_in.get();
	c2 = m_in.get();

	if (c1 == '@' && c2 == '{')
	{
		ignore_whitespaces();

		int level = 1;
		bool end = false;
		// comenzamos la extracción de la plantilla  
		while (!end && m_in.peek() != EOF)
		{
			c1 = m_in.get();

			// comprobamos si estamos ante un posible cierre o apertura de llaves.
			if (c1 == '@')
			{
				c2 = m_in.get();

				if (c2 == '}')
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

					if (c2 == '{')
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
	if (m_pred != "")
		m_out << m_pred;

	return true;
}

bool TemplateParser::ParseNewLine()
{
	m_out << '\n';
	return true;
}

void TemplateParser::ParseAppend()
{
	ignore_whitespaces();
}

string TemplateParser::PropertyToCode(shared_ptr<Property> property)
{
	return ValueToCode(property->GetType(), property->GetValue());
}

//////////////////////////////////////////////////////////////////////////////
CodeWriter::CodeWriter()
{
	m_indent = 0;
	m_cols = 0;
}  
