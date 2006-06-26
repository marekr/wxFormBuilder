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

// Atencion!!!!!!
// ObjectBase::GetPropertyCount() != ObjectInfo::GetPropertyCount()
//
// En el primer caso devolverá el numero total de propiedades del objeto.
// En el segundo caso sólo devolverá el número de propiedades definidas
// para esa clase.

#ifndef __OBJ__
#define __OBJ__

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <boost/smart_ptr.hpp>
#include "types.h"
#include "tinyxml.h"

#include "wx/wx.h"
#include "plugins/component.h"

using namespace std;
using namespace boost;

class ObjectBase;
class ObjectInfo;

///////////////////////////////////////////////////////////////////////////////

class OptionList
{
private:

	map< string, string > m_options;

public:

	void AddOption( string option, string description = "" )
	{
		m_options[option] = description;
	}
	unsigned int GetOptionCount()
	{
		return (unsigned int)m_options.size();
	}
	const map< string, string >& GetOptions()
	{
		return m_options;
	}
};

///////////////////////////////////////////////////////////////////////////////

class PropertyInfo
{
private:
	string       m_name;
	PropertyType m_type;
	string       m_def_value;
	shared_ptr<OptionList>  m_opt_list;
	bool m_hidden; // Juan. Determina si la propiedad aparece o no en XRC
	string		m_description;

public:

	PropertyInfo(string name, PropertyType type, string def_value, string description = "",
		bool hidden = false, shared_ptr<OptionList> opt_list = shared_ptr<OptionList>()); //Juan

	~PropertyInfo();

	string       GetDefaultValue()        { return m_def_value;  }
	PropertyType GetType()                { return m_type;       }
	string       GetName()                { return m_name;       }
	shared_ptr<OptionList>  GetOptionList ()         { return m_opt_list;   }
	string		 GetDescription	()		  { return m_description;}
	bool         IsHidden()               { return m_hidden; } // Juan
	void         SetHidden(bool hidden)   { m_hidden = hidden; } // Juan
};

///////////////////////////////////////////////////////////////////////////////

class Property
{
private:
	shared_ptr<PropertyInfo> m_info; // puntero a su descriptor
	weak_ptr<ObjectBase> m_object; // una propiedad siempre pertenece a un objeto

	string m_name;
	string m_value;

public:
	Property(shared_ptr<PropertyInfo> info, shared_ptr<ObjectBase> obj = shared_ptr<ObjectBase>())
	{
		m_name = info->GetName();
		m_object = obj;
		m_info = info;
	};

	shared_ptr<ObjectBase> GetObject() { return m_object.lock(); }
	string GetName() { return m_name; };
	string GetValue() { return m_value; };
	void SetValue(string val)
	{
		m_value = val;
	};
	shared_ptr<PropertyInfo> GetPropertyInfo() { return m_info; }
	PropertyType GetType()          { return m_info->GetType();  }

	bool IsDefaultValue();
	void SetDefaultValue();

	////////////////////
	void SetValue(const wxFont &font);
	void SetValue(const wxColour &colour);
	void SetValue(const wxString &str, bool format = false);
	void SetValue(const wxPoint &point);
	void SetValue(const wxSize &size);
	void SetValue(const int integer);
	void SetValue(const double val );

	wxFont   GetValueAsFont();
	wxColour GetValueAsColour();
	wxPoint  GetValueAsPoint();
	wxSize   GetValueAsSize();
	int      GetValueAsInteger();
	wxString GetValueAsString();
	wxBitmap GetValueAsBitmap();
	wxString GetValueAsText();   // sustituye los ('\n',...) por ("\\n",...)

	wxArrayString GetValueAsArrayString();
	double GetValueAsFloat();

};


///////////////////////////////////////////////////////////////////////////////

class ObjectBase : public IObject, public enable_shared_from_this<ObjectBase>
{
private:
	// number of instances of the class, será util para comprobar si
	// efectivamente no se están produciendo leaks de memoria
	static int s_instances;

	string m_class;            // clase que repesenta el objeto
	string m_type; //ObjectType m_type;         // RTTI
	weak_ptr<ObjectBase> m_parent;     // no parent

	vector< shared_ptr<ObjectBase> > m_children;   // children
	map< string,shared_ptr<Property> >  m_properties; // Properties of the object
	shared_ptr<ObjectInfo> m_info;


protected:
	// utilites for implementing the tree
	static const int INDENT;  // size of indent
	string GetIndentString(int indent); // obtiene la cadena con el indentado

	vector< shared_ptr<ObjectBase> >& GetChildren()   { return m_children; };
	map< string,shared_ptr<Property> >&  GetProperties() { return m_properties; };

	// Crea un elemento del objeto
	virtual TiXmlElement* SerializeObject();

	// devuelve el puntero "this"
	shared_ptr<ObjectBase> GetThis()
	{
		return shared_from_this();
	}


	//bool DoChildTypeOk (string type_child ,string type_parent);
	/*
	* Configura la instancia en su creación.
	*
	* Este método realiza todas las operaciones necesarias para configurar
	* el objeto en su creación.
	* Las clases derivadas (y esta misma) deberán llamar a esta función en
	* el método NewInstance. De esta forma, emulamos el comportamiento de un
	* constructor.
	*/
	//  void SetupInstance(shared_ptr<ObjectBase> parent = shared_ptr<ObjectBase>());


public:
	/**
	* Constructor. (debe ser "protegido" -> NewInstance)
	*/
	ObjectBase (string class_name);

	// Mejor es que sea el propio objeto quien construya todas sus propiedades...
	//ObjectBase(shared_ptr<ObjectInfo> obj_info);

	/*
	* Pseudo-Constructor.
	* Crea una instancia de un objeto de forma dinámica.
	* De esta forma evitamos los problemas de C++ para usar métodos virtuales
	* en un contructor, o el problema de devolver un shared_ptr en lugar
	* de un puntero normal.
	*/
	/*  static shared_ptr<ObjectBase> NewInstance
	(string class_name, shared_ptr<ObjectBase> parent = shared_ptr<ObjectBase>());*/


	/**
	* Destructor.
	*/
	virtual ~ObjectBase();

	/**
	* Obtiene el nombre del objeto.
	*
	* @note No confundir con la propiedad nombre que tienen algunos objetos.
	*       Cada objeto tiene un nombre, el cual será el mismo que el usado
	*       como clave en el registro de descriptores.
	*/
	string     GetClassName ()            { return m_class;  }

	/**
	* Obtiene el nodo padre.
	*/
	shared_ptr<ObjectBase> GetParent ()                   { return m_parent.lock();   }

	/**
	* Establece la conexión con el padre.
	*/
	void SetParent(shared_ptr<ObjectBase> parent)  { m_parent = parent; }

	/**
	* Obtiene la propiedad identificada por el nombre.
	*
	* @note Notar que no existe el método SetProperty, ya que la modificación
	*       se hace a través de la referencia.
	*/
	shared_ptr<Property> GetProperty (string name);

	/**
	* Añade una propiedad al objeto.
	*
	* Este método será usado por el registro de descriptores para crear la
	* instancia del objeto.
	* Los objetos siempre se crearán a través del registro de descriptores.
	*/
	void AddProperty (string propname, shared_ptr<Property> value);

	/**
	* Obtiene el número de propiedades del objeto.
	*/
	unsigned int GetPropertyCount() { return (unsigned int)m_properties.size(); }

	/**
	* Obtiene una propiedad del objeto.
	* @todo esta función deberá lanzar una excepción en caso de no encontrarse
	*       dicha propiedad, así se simplifica el código al no tener que hacer
	*       tantas comprobaciones.
	* Por ejemplo, el código sin excepciones sería algo así:
	*
	* @code
	*
	* shared_ptr<Property> plabel = obj->GetProperty("label");
	* shared_ptr<Property> ppos = obj->GetProperty("pos");
	* shared_ptr<Property> psize = obj->GetProperty("size");
	* shared_ptr<Property> pstyle = obj->GetProperty("style");
	*
	* if (plabel && ppos && psize && pstyle)
	* {
	*   wxButton *button = new wxButton(parent,-1,
	*    plabel->GetValueAsString(),
	*    ppos->GetValueAsPoint(),
	*    psize->GetValueAsSize(),
	*    pstyle->GetValueAsInteger());
	* }
	* else
	* {
	*   // manejo del error
	* }
	*
	* @endcode
	*
	* y con excepciones:
	*
	* @code
	*
	* try
	* {
	*   wxButton *button = new wxButton(parent,-1,
	*     obj->GetProperty("label")->GetValueAsString(),
	*     obj->GetProperty("pos")->GetValueAsPoint(),
	*     obj->GetProperty("size")->GetValueAsSize(),
	*     obj->GetProperty("style")->GetValueAsInteger());
	* }
	* catch (...)
	* {
	*   // manejo del error
	* }
	*
	* @endcode
	*
	*/
	shared_ptr<Property>    GetProperty (unsigned int idx); // throws ...;

	/**
	* Devuelve el primer antecesor cuyo tipo coincida con el que se pasa
	* como parámetro.
	*
	* Será útil para encontrar el widget padre.
	*/
	shared_ptr<ObjectBase> FindNearAncestor(string type);

	/**
	* Obtiene el documento xml del arbol tomando como raíz el nodo actual.
	*/
	TiXmlDocument* Serialize();

	/**
	* Añade un hijo al objeto.
	* Esta función es virtual, debido a que puede variar el comportamiento
	* según el tipo de objeto.
	*
	* @return true si se añadió el hijo con éxito y false en caso contrario.
	*/
	virtual bool AddChild    (shared_ptr<ObjectBase>);
	virtual bool AddChild    (unsigned int idx, shared_ptr<ObjectBase> obj);

	/**
	* Devuelve la posicion del hijo o GetChildCount() en caso de no encontrarlo
	*/
	unsigned int GetChildPosition(shared_ptr<ObjectBase> obj);
	bool ChangeChildPosition(shared_ptr<ObjectBase> obj, unsigned int pos);

	/**
	* devuelve la posición entre sus hermanos
	*/
	/*  unsigned int GetPosition();
	bool ChangePosition(unsigned int pos);*/


	/**
	* Elimina un hijo del objeto.
	*/
	void RemoveChild (shared_ptr<ObjectBase> obj);
	void RemoveChild (unsigned int idx);

	/**
	* Obtiene un hijo del objeto.
	*/
	shared_ptr<ObjectBase> GetChild (unsigned int idx);

	/**
	* Obtiene el número de hijos del objeto.
	*/
	unsigned int  GetChildCount()    { return (unsigned int)m_children.size(); }

	/**
	* Comprueba si el tipo de objeto pasado es válido como hijo del objeto.
	* Esta rutina es importante, ya que define las restricciónes de ubicación.
	*/
	//bool ChildTypeOk (string type);
	bool ChildTypeOk (PObjectType type);

	bool IsContainer() { return (GetObjectTypeName() == "container"); }

	shared_ptr<ObjectBase> GetLayout();

	/**
	* Devuelve el tipo de objeto.
	*
	* Deberá ser redefinida en cada clase derivada.
	*/
	string GetObjectTypeName() { return m_type; }
	void SetObjectTypeName(string type) { m_type = type; }

	/**
	* Devuelve el descriptor del objeto.
	*/
	shared_ptr<ObjectInfo> GetObjectInfo() { return m_info; };
	void SetObjectInfo(shared_ptr<ObjectInfo> info) { m_info = info; };

	/**
	* Devuelve la profundidad  del objeto en el arbol.
	*/
	int Deep();

	/**
	* Imprime el arbol en un stream.
	*/
	//virtual void PrintOut(ostream &s, int indent);

	/**
	* Sobrecarga del operador inserción.
	*/
	friend ostream& operator << (ostream &s, shared_ptr<ObjectBase> obj);

	/////////////////////////
	// Implementación de la interfaz IObject para su uso dentro de los
	// plugins
	bool     IsNull (const wxString& pname);
	int      GetPropertyAsInteger (const wxString& pname);
	wxFont   GetPropertyAsFont    (const wxString& pname);
	wxColour GetPropertyAsColour  (const wxString& pname);
	wxString GetPropertyAsString  (const wxString& pname);
	wxPoint  GetPropertyAsPoint   (const wxString& pname);
	wxSize   GetPropertyAsSize    (const wxString& pname);
	wxBitmap GetPropertyAsBitmap  (const wxString& pname);
	double	 GetPropertyAsFloat	  (const wxString& pname);

	wxArrayInt    GetPropertyAsArrayInt (const wxString& pname);
	wxArrayString GetPropertyAsArrayString  (const wxString& pname);
};

///////////////////////////////////////////////////////////////////////////////

/**
* Clase que guarda un conjunto de plantillas de código.
*/
class CodeInfo
{
private:
	typedef map<string,string> TemplateMap;
	TemplateMap m_templates;
public:
	string GetTemplate(string name);
	void AddTemplate(string name, string _template);
};

///////////////////////////////////////////////////////////////////////////////

/**
* Información de objeto o MetaObjeto.
*/
class ObjectInfo
{
public:
	/**
	* Constructor.
	*/
	ObjectInfo(string class_name, PObjectType type);

	virtual ~ObjectInfo() {};

	unsigned int GetPropertyCount() { return (unsigned int)m_properties.size(); }

	/**
	* Obtiene el descriptor de la propiedad.
	*/
	shared_ptr<PropertyInfo> GetPropertyInfo(string name);
	shared_ptr<PropertyInfo> GetPropertyInfo(unsigned int idx);

	/**
	* Añade un descriptor de propiedad al descriptor de objeto.
	*/
	void AddPropertyInfo(shared_ptr<PropertyInfo> pinfo);

	/**
	* Devuelve el tipo de objeto, será util para que el constructor de objetos
	* sepa la clase derivada de ObjectBase que ha de crear a partir del
	* descriptor.
	*/
	string GetObjectTypeName() { return m_type->GetName();   }

	PObjectType GetObjectType() { return m_type; }

	string GetClassName () { return m_class;  }

	/**
	* Imprime el descriptor en un stream.
	*/
	//virtual void PrintOut(ostream &s, int indent);


	/**
	* Sobrecarga del operador inserción.
	*/
	friend ostream& operator << (ostream &s, shared_ptr<ObjectInfo> obj);

	// nos serán utiles para generar el nombre del objeto
	unsigned int GetInstanceCount() { return m_numIns; }
	void IncrementInstanceCount()   { m_numIns++; }
	void ResetInstanceCount() { m_numIns = 0; }

	/**
	* Añade la información de un objeto al conjunto de clases base.
	*/
	void AddBaseClass(shared_ptr<ObjectInfo> base) {  m_base.push_back(base); }

	/**
	* Comprueba si el tipo es derivado del que se pasa como parámetro.
	*/
	bool IsSubclassOf(string classname);

	shared_ptr<ObjectInfo> GetBaseClass(unsigned int idx);
	unsigned int GetBaseClassCount();


	//
	void SetIconFile(wxBitmap icon) { m_icon = icon; };
	wxBitmap GetIconFile() { return m_icon; }

	void SetSmallIconFile(wxBitmap icon) { m_smallIcon = icon; };
	wxBitmap GetSmallIconFile() { return m_smallIcon; }

	void AddCodeInfo(string lang, shared_ptr<CodeInfo> codeinfo);
	shared_ptr<CodeInfo> GetCodeInfo(string lang);

	/**
	* Le asigna un componente a la clase.
	*/
	void SetComponent(IComponent *c) { m_component = c; };
	IComponent* GetComponent() { return m_component; };

private:
	typedef map< string, shared_ptr<CodeInfo> > CodeInfoMap;
	string m_class;         // nombre de la clase (tipo de objeto)

	PObjectType m_type;     // tipo del objeto

	wxBitmap m_icon;
	wxBitmap m_smallIcon; // The icon for the property grid toolbar

	CodeInfoMap m_codeTemp;  // plantillas de codigo K=language_name T=shared_ptr<CodeInfo>

	unsigned int m_numIns;  // número de instancias del objeto

	map< string, shared_ptr< PropertyInfo > > m_properties;
	vector< shared_ptr< ObjectInfo > > m_base; // clases base
	IComponent* m_component;  // componente asociado a la clase los objetos del
	// designer
};

#endif
