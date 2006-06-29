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

/**
@file
@author José Antonio Hurtado - joseantonio.hurtado@gmail.com
@author Juan Antonio Ortega  - jortegalalmolda@gmail.com
@note
The implementation of the generation of relative paths is a little hacky, and not a solution.
The value of all properties that are file or a directory paths must be absolute, otherwise the code generation will not work.
*/

#ifndef _CPP_CODE_GEN_
#define _CPP_CODE_GEN_

#include <set>
#include "codegen.h"
#include "utils/unistring.h"

using namespace std;

/**
* Parse the C++ templates.
*/
class CppTemplateParser : public TemplateParser
{
private:
	bool m_useRelativePath;
	bool m_i18n;
	unistring m_basePath;

public:
	CppTemplateParser( shared_ptr<ObjectBase> obj, unistring _template);

	// redefinidas para C++
	shared_ptr<TemplateParser> CreateParser( shared_ptr<ObjectBase> obj, unistring _template);
	unistring RootWxParentToCode();
	//unistring PropertyToCode( shared_ptr<Property> property);
	unistring ValueToCode( PropertyType type, unistring value);

	// genera rutas relativas en los nombres de archivo
	void UseRelativePath( bool relative = false, unistring basePath = unistring());
	void UseI18n( bool i18n);
};

/**
* Generate the C++ code
*/
class CppCodeGenerator : public CodeGenerator
{
private:
	typedef enum
	{
		P_PRIVATE,
		P_PROTECTED,
		P_PUBLIC
	} Permission;

	shared_ptr<CodeWriter> m_header;
	shared_ptr<CodeWriter> m_source;

	bool m_useRelativePath;
	bool m_i18n;
	unistring m_basePath;
	unsigned int m_firstID;

	/**
	* Las macros predefinidas no generarán defines.
	*/
	set<unistring> m_predMacros;

	void SetupPredefinedMacros();

	/**
	* Dado un objeto y el nombre de una plantilla, obtiene el código.
	*/
	unistring GetCode( shared_ptr<ObjectBase> obj, unistring name);

	/**
	* Guarda el conjunto de clases de objetos del proyecto para generar
	* los includes.
	*/
	void FindDependencies( shared_ptr< ObjectBase > obj, set< shared_ptr< ObjectInfo > >& info_set );

	/**
	* Guarda el conjunto de "includes" que hay que generar para las propiedades
	* PT_XPM_BITMAP.
	*/
	void FindXpmProperties( shared_ptr<ObjectBase> obj, set<unistring> &set);

	/**
	* Guarda todos las propiedades de objetos de tipo "macro" para generar
	* su posterior '#define'.
	*/
	void FindMacros( shared_ptr<ObjectBase> obj, set<unistring> &macro_set);

	/**
	* Genera la declaración de clases en el fichero de cabecera.
	*/
  void GenClassDeclaration( shared_ptr<ObjectBase> class_obj, bool use_enum);

	/**
	* Función recursiva para la declaración de atributos, usada dentro
	* de GenClassDeclaration.
	*/
	void GenAttributeDeclaration( shared_ptr<ObjectBase> obj, Permission perm);

	/**
	* Genera la sección de '#include' fichero.
	*/
	void GenIncludes( shared_ptr<ObjectBase> project, set<unistring>* includes);

	/**
	* Generate compiler directives for linking libraries ( mostly for widgets that aren't in the standard distribution )
	*/
	void GenLibraries( shared_ptr< ObjectBase > project, set< unistring >* libraries );

	/**
	* Using the Generated Library names, create #ifdef block to link correct library for the application settings
	*/
	void WriteLibrariesBlock( const set< unistring >& libraries );

	/**
	* Write a set of libraries with the given prefix and suffic
	*/
	void WriteLibraries( const set< unistring >& libraries, const unistring& prefix, const unistring& suffix );

	/**
	* Genera la sección de '#include' para las propiedades XPM.
	*/
	void GenXpmIncludes( shared_ptr<ObjectBase> project);

	/**
	* Genera la sección de '#define' macro.
	*/
	void GenDefines( shared_ptr<ObjectBase> project);

	/**
	* Generate a enum with wxWindow identifiers.
	*/
	void GenEnumIds( shared_ptr<ObjectBase> class_obj);
  
	/**
	* Generate the constructor of a classs
	*/
	void GenConstructor( shared_ptr<ObjectBase> class_obj );

	/**
	* Realiza la construcción de los objetos, configurando las propiedades del
	* objeto y las de layout.
	* El algoritmo es similar al de generación de la vista previa en el designer.
	*/
	void GenConstruction( shared_ptr<ObjectBase> obj, bool is_widget);

	/**
	* Configura las propiedades del objeto, tanto las propias como las heredadas.
	* Se le pasa la información de la clase porque recursivamente, realizará
	* la configuración en las super-clases.
	*/
	void GenSettings( shared_ptr<ObjectInfo> info, shared_ptr<ObjectBase> obj);

	/**
	* Añade un control a una toolbar. Hay que pasarle el objectinfo de tipo
	* wxWindow, donde se encuentra la plantilla, y el objectbase del control
	*/
	void GenAddToolbar( shared_ptr<ObjectInfo> info, shared_ptr<ObjectBase> obj );

public:
	/**
	* Convert a unistring to the "C/C++" format.
	*/
	static unistring ConvertCppString( unistring text);

	/**
	* Convert a path to a relative path.
	*/
	//static unistring ConvertToRelativePath( unistring path, unistring basePath);

	/**
	* Convert an XPM filename to the name of the XPM's internal character array.
	*/
	static unistring ConvertXpmName( const unistring& text );

	CppCodeGenerator();

	/**
	* Set the codewriter for the header file
	*/
	void SetHeaderWriter( shared_ptr<CodeWriter> cw )
	{
		m_header = cw;
	}

	/**
	* Set the codewriter for the source file
	*/
	void SetSourceWriter( shared_ptr<CodeWriter> cw )
	{
		m_source = cw;
	}


	/**
	* Configura el path de referencia para generar las rutas relativas
	* al path que se pasa como parámetro.
	*
	* @note path is generated with the separators, '/', since on Windows
	*		the compilers interpret path correctly.
	*/
	void UseRelativePath( bool relative = false, unistring basePath = unistring() );

	/**
	* Set the First ID used during Code Generation.
	*/
	void SetFirstID( const unsigned int id ){ m_firstID = id; }

	/**
	* Generate the project's code
	*/
	bool GenerateCode( shared_ptr<ObjectBase> project );
};


#endif //_CPP_CODE_GEN_
