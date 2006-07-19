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

#include "database.h"
#include "rad/bitmaps.h"
#include "utils/stringutils.h"
#include "utils/typeconv.h"
#include "utils/debug.h"
#include <wx/filename.h>
#include <wx/image.h>

//#define DEBUG_PRINT(x) cout << x

#define OBJINFO_TAG "objectinfo"
#define CODEGEN_TAG "codegen"
#define TEMPLATE_TAG "template"
#define NAME_TAG "name"
#define DESCRIPTION_TAG "help"
#define PROPERTY_TAG "property"
#define OBJECT_TAG "object"
#define CLASS_TAG "class"
#define PACKAGE_TAG "package"
#define PKGDESC_TAG "desc"
#define PRGLANG_TAG "language"
#define HIDDEN_TAG "hidden"     //Juan
#define ICON_TAG "icon"
#define SMALL_ICON_TAG "smallIcon"


ObjectPackage::ObjectPackage(wxString name, wxString desc, wxBitmap icon)
{
	m_name = name;
	m_desc = desc;
	m_icon = icon;
}

shared_ptr<ObjectInfo> ObjectPackage::GetObjectInfo(unsigned int idx)
{
	assert (idx < m_objs.size());
	return m_objs[idx];
}


///////////////////////////////////////////////////////////////////////////////

ObjectDatabase::ObjectDatabase()
{
	//InitObjectTypes();
	//  InitWidgetTypes();
	InitPropertyTypes();
}

ObjectDatabase::~ObjectDatabase()
{
	// destruir todos los objetos
}

shared_ptr< ObjectInfo > ObjectDatabase::GetObjectInfo(wxString class_name)
{
	shared_ptr< ObjectInfo > info;
	map< wxString, shared_ptr< ObjectInfo > >::iterator it = m_objs.find( class_name );

	if ( it != m_objs.end() )
	{
		info = it->second;
	}

	return info;
}

PObjectPackage ObjectDatabase::GetPackage(unsigned int idx)
{
	assert (idx < m_pkgs.size());

	return m_pkgs[idx];
}


/**
* @todo La herencia de propiedades ha de ser de forma recursiva.
*/

shared_ptr<ObjectBase> ObjectDatabase::NewObject(shared_ptr<ObjectInfo> obj_info)
{
	shared_ptr<ObjectBase> object;

	// Llagados aquí el objeto se crea seguro...
	object = shared_ptr<ObjectBase>(new ObjectBase(obj_info->GetClassName()));
	object->SetObjectTypeName(obj_info->GetObjectTypeName()); // *FIXME*

	object->SetObjectInfo(obj_info);

	shared_ptr<PropertyInfo> prop_info;
	shared_ptr<ObjectInfo> class_info = obj_info;

	unsigned int base = 0;
	while (class_info)
	{
		unsigned int i;
		for (i = 0; i < class_info->GetPropertyCount(); i++)
		{
			prop_info = class_info->GetPropertyInfo(i);

			shared_ptr<Property> property(new Property(prop_info, object));

			// Set the default value, either from the property info, or an override from this class
			wxString defaultValue = prop_info->GetDefaultValue();
			if ( base > 0 )
			{
				wxString defaultValueTemp = obj_info->GetBaseClassDefaultPropertyValue( base - 1, prop_info->GetName() );
				if ( !defaultValueTemp.empty() )
				{
					defaultValue = defaultValueTemp;
				}
			}
			property->SetValue( defaultValue );

			// Las propiedades están implementadas con una estructura "map",
			// ello implica que no habrá propiedades duplicadas.
			// En otro caso habrá que asegurarse de que dicha propiedad
			// no existe.
			// Otra cosa importante, es que el orden en que se insertan
			// las propiedades, de abajo-arriba, esto permite que se pueda redefir
			// alguna propiedad.
			object->AddProperty (prop_info->GetName(), property);
		}

		class_info = ( base < obj_info->GetBaseClassCount() ?
			obj_info->GetBaseClass(base++) : shared_ptr<ObjectInfo>());
	}

	// si el objeto tiene la propiedad name (reservada para el nombre del
	// objeto) le añadimos el contador para no repetir nombres.

	obj_info->IncrementInstanceCount();

	unsigned int ins = obj_info->GetInstanceCount();
	shared_ptr<Property> pname = object->GetProperty( wxT(NAME_TAG) );
	if (pname)
		pname->SetValue(pname->GetValue() + StringUtils::IntToStr(ins));

	return object;
}


int ObjectDatabase::CountChildrenWithSameType(shared_ptr<ObjectBase> parent,PObjectType type)
{
	unsigned int count = 0;
	unsigned int numChildren = parent->GetChildCount();
	for (unsigned int i=0; i < numChildren ; i++)
	{
		if (type == parent->GetChild(i)->GetObjectInfo()->GetObjectType())
			count++;
	}

	return count;
}

/**
* Crea una instancia de classname por debajo de parent.
* La función realiza la comprobación de tipos para crear el objeto:
* - Comprueba si el tipo es un tipo-hijo válido de "parent", en cuyo caso
*   se comprobará también que el número de hijos del mismo tipo no sobrepase
el máximo definido. El objeto no se crea si supera el máximo permitido.
* - Si el tipo-hijo no se encuentra entre los definidos para el tipo de
*   "parent" se intentará crearlo como hijo de alguno de los tipos hijos con el
*   flag item a "1". Para ello va recorriendo todos los tipos con flag item,
*   si no puede crear el objeto, bien por que el tipo no es válido o porque
*   sobrepasa el máximo permitido si intenta con el siguiente hasta que no queden
*   más.
*
* Nota: quizá sea conveniente que el método cree el objeto sin enlazarlo
*       en el árbol, para facilitar el undo-redo.
*/
shared_ptr<ObjectBase> ObjectDatabase::CreateObject( std::string classname, shared_ptr<ObjectBase> parent)
{
	shared_ptr<ObjectBase> object;
	shared_ptr<ObjectInfo> objInfo = GetObjectInfo( _WXSTR(classname) );

	if (!objInfo)
	{
		wxLogError(wxT("Object not found (") + _WXSTR(classname) + wxT(")"));
		return shared_ptr<ObjectBase>();
	}

	PObjectType objType = objInfo->GetObjectType();

	if (parent)
	{
		// Comprobamos si el tipo es válido
		PObjectType parentType = parent->GetObjectInfo()->GetObjectType();
		int max = parentType->FindChildType(objType);

		// FIXME! Esto es un parche para evitar crear los tipos menubar,statusbar y
		// toolbar en un form que no sea wxFrame.
		// Hay que modificar el conjunto de tipos para permitir tener varios tipos
		// de forms (como childType de project), pero hay mucho código no válido
		// para forms que no sean de tipo "form". Dicho de otra manera, hay
		// código que dependen del nombre del tipo, cosa que hay que evitar.
		if (parentType->GetName() == wxT("form") && parent->GetClassName() != wxT("Frame") &&
			(objType->GetName() == wxT("statusbar") ||
			objType->GetName() == wxT("menubar") ||
			objType->GetName() == wxT("toolbar") ))
			return shared_ptr<ObjectBase>(); // tipo no válido

		if (max != 0) // tipo válido
		{
			bool create = true;

			// comprobamos el número de instancias
			if (max > 0 && CountChildrenWithSameType(parent, objType) >= max)
				create = false;

			if (create)
				object = NewObject(objInfo);
		}
		else // max == 0
		{
			// el tipo no es válido, vamos a comprobar si podemos insertarlo
			// como hijo de un "item"
			bool created = false;
			for (unsigned int i=0; !created && i < parentType->GetChildTypeCount(); i++)
			{
				PObjectType childType = parentType->GetChildType(i);
				int max = childType->FindChildType(objType);

				if (childType->IsItem() && max != 0)
				{
					max = parentType->FindChildType(childType);

					// si el tipo es un item y además el tipo del objeto a crear
					// puede ser hijo del tipo del item vamos a intentar crear la
					// instancia del item para crear el objeto como hijo de este
					if (max < 0 || CountChildrenWithSameType(parent, childType) < max)
					{
						// No hay problemas para crear el item debajo de parent
						shared_ptr<ObjectBase> item = NewObject(GetObjectInfo(childType->GetName()));

						//shared_ptr<ObjectBase> obj = CreateObject(classname,item);
						shared_ptr<ObjectBase> obj = NewObject(objInfo);

						// la siguiente condición debe cumplirse siempre
						// ya que un item debe siempre contener a otro objeto
						if (obj)
						{
							// enlazamos item y obj
							item->AddChild(obj);
							obj->SetParent(item);

							// sizeritem es un tipo de objeto reservado, para que el uso sea
							// más práctico se asignan unos valores por defecto en función
							// del tipo de objeto creado
							if (item->GetObjectTypeName() == wxT("sizeritem"))
								SetDefaultLayoutProperties(item);

							object = item;
							created = true;
						}
						else
							wxLogError(wxT("Review your definitions file (objtypes.xml)"));
					}
				}
			}
		}
		///////////////////////////////////////////////////////////////////////
		// Nota: provisionalmente vamos a enlazar el objeto al padre pero
		//       esto debería hacerse fuera para poder implementar el Undo-Redo
		///////////////////////////////////////////////////////////////////////
		//if (object)
		//{
		//  parent->AddChild(object);
		//  object->SetParent(parent);
		//}
	}
	else // parent == NULL;
	{
		object = NewObject(objInfo);
	}

	return object;
}

shared_ptr<ObjectBase> ObjectDatabase::CopyObject(shared_ptr<ObjectBase> obj)
{
	assert(obj);

	shared_ptr<ObjectInfo> objInfo = obj->GetObjectInfo();

	shared_ptr<ObjectBase> copyObj = NewObject(objInfo); // creamos la copia
	assert(copyObj);

	// copiamos las propiedades
	unsigned int i;
	unsigned int count = obj->GetPropertyCount();
	for (i = 0; i < count; i++)
	{
		shared_ptr<Property> objProp = obj->GetProperty(i);
		assert(objProp);

		shared_ptr<Property> copyProp = copyObj->GetProperty(objProp->GetName());
		assert(copyProp);

		wxString propValue = objProp->GetValue();
		copyProp->SetValue(propValue);
	}

	// creamos recursivamente los hijos
	count = obj->GetChildCount();
	for (i = 0; i<count; i++)
	{
		shared_ptr<ObjectBase> childCopy = CopyObject(obj->GetChild(i));
		copyObj->AddChild(childCopy);
		childCopy->SetParent(copyObj);
	}

	return copyObj;
}

void ObjectDatabase::SetDefaultLayoutProperties(shared_ptr<ObjectBase> sizeritem)
{
	assert(sizeritem->GetObjectTypeName() == wxT("sizeritem"));

	wxString obj_type = sizeritem->GetChild(0)->GetObjectTypeName();

	if (obj_type == wxT("notebook")			||
		obj_type == wxT("flatnotebook")		||
		obj_type == wxT("listbook")			||
		obj_type == wxT("choicebook")		||
		obj_type == wxT("expanded_widget")	||
		obj_type == wxT("container")
		)
	{
		sizeritem->GetProperty( wxT("proportion") )->SetValue( wxT("1") );
		sizeritem->GetProperty( wxT("flag") )->SetValue( wxT("wxEXPAND | wxALL") );
	}
	else if ( obj_type == wxT("widget") || obj_type == wxT("statusbar") )
	{
		sizeritem->GetProperty( wxT("proportion") )->SetValue( wxT("0") );
		sizeritem->GetProperty( wxT("flag") )->SetValue( wxT("wxALL") );
	}
	else if ( obj_type == wxT("sizer") || obj_type == wxT("splitter") )
	{
		sizeritem->GetProperty( wxT("proportion") )->SetValue( wxT("1") );
		sizeritem->GetProperty( wxT("flag") )->SetValue( wxT("wxEXPAND") );
	}
}

void ObjectDatabase::ResetObjectCounters()
{
	map< wxString, shared_ptr< ObjectInfo > >::iterator it;
	for (it = m_objs.begin() ; it != m_objs.end() ; it++)
	{
		it->second->ResetInstanceCount();
	}
}

///////////////////////////////////////////////////////////////////////

shared_ptr<ObjectBase>  ObjectDatabase::CreateObject(TiXmlElement *xml_obj, shared_ptr<ObjectBase> parent)
{
	if (!xml_obj->Attribute(CLASS_TAG))
		return shared_ptr<ObjectBase>();

	std::string class_name = xml_obj->Attribute(CLASS_TAG);
	shared_ptr<ObjectBase> object = CreateObject(class_name,parent);

	if (object)
	{
		// modificamos cada propiedad
		TiXmlElement *xml_prop = xml_obj->FirstChildElement(PROPERTY_TAG);
		while (xml_prop)
		{
			std::string prop_value;
			std::string prop_name = xml_prop->Attribute(NAME_TAG);
			shared_ptr<Property> prop = object->GetProperty( _WXSTR(prop_name) );
			if (prop) // ¿ existe la propiedad ?
			{
				// modificamos el valor
				TiXmlNode *xml_value = xml_prop->FirstChild();
				if (xml_value && xml_value->ToText())
					prop_value = xml_value->ToText()->Value();
				else
					prop_value = ""; // valor nulo

				prop->SetValue( _WXSTR(prop_value) );
			}

			xml_prop = xml_prop->NextSiblingElement(PROPERTY_TAG);
		}

		if (parent)
		{
			// enlazamos
			parent->AddChild(object);
			object->SetParent(parent);
		}

		// creamos los hijos
		TiXmlElement *child = xml_obj->FirstChildElement(OBJECT_TAG);
		while (child)
		{
			shared_ptr<ObjectBase> childObj = CreateObject(child,object);
			child = child->NextSiblingElement(OBJECT_TAG);
		}
	}
	return object;
}

//////////////////////////////

bool IncludeInPalette(wxString type)
{
	return true;
}

bool ObjectDatabase::LoadFile( std::string file)
{
	bool result = false;
	TiXmlDocument doc(m_xmlPath + '/' + file);
	result = doc.LoadFile();

	if (result)
	{
		// Cargamos el paquete por defecto
		LoadPackage("default.xml");
		LoadCodeGen("default.cppcode");

		TiXmlElement* root = doc.FirstChildElement("packages");
		if (root)
		{
			TiXmlElement* elem = root->FirstChildElement(PACKAGE_TAG);

			// En primer lugar se carga uno por uno todos los objetos
			// de los archivos xml en una primera pasada.
			while (elem)
			{
				std::string file = elem->Attribute("file");
				wxFileName fn(_WXSTR(file));
				fn.SetExt(wxT("cppcode"));

				PObjectPackage package = LoadPackage(file);
				if (package)
				{
					m_pkgs.push_back(package);
					LoadCodeGen(_STDSTR(fn.GetFullName()));
				}

				elem = elem->NextSiblingElement(PACKAGE_TAG);
			}

			// En una segunda pasada, añadimos a cada objeto
			// sus correspondientes "objetos base" para que puedan heredar
			// sus propiedades
			elem = root->FirstChildElement(PACKAGE_TAG);
			while (elem)
			{
				std::string file = elem->Attribute("file");
				SetupPackage(file);
				elem = elem->NextSiblingElement(PACKAGE_TAG);
			}
		}

		// imprimimos las macros declaradas pero no importadas.
#ifdef __WXFB_DEBUG__
		MacroSet::iterator it;
		//Debug::Print("Missing macros of component libraries");
		wxLogMessage(wxT("Missing macros of component libraries"));
		for (it = m_macroSet.begin(); it != m_macroSet.end(); it++)
		{
			wxLogMessage(_WXSTR(*it));
			//Debug::Print((wxChar*)(*it).c_str());
		}
#endif


	}
	//  else
	//  {
	//    cout << "Loading error on "<< file <<" file." << endl;
	//    system("pause");
	//  }

	return result;
}

void ObjectDatabase::SetupPackage( std::string file)
{
	//  DEBUG_PRINT("SETTING-UP " + file + "...");

	TiXmlDocument doc(m_xmlPath + '/' + file);
	if (doc.LoadFile())
	{
		//    DEBUG_PRINT("OK\n");
		TiXmlElement* root = doc.FirstChildElement(PACKAGE_TAG);
		if (root)
		{
			// comprobamos si hay libraría que importar
			if (root->Attribute("lib"))
				ImportComponentLibrary( _WXSTR(root->Attribute("lib")) );

			TiXmlElement* elem_obj = root->FirstChildElement(OBJINFO_TAG);
			while (elem_obj)
			{
				std::string class_name  = elem_obj->Attribute(CLASS_TAG);
				shared_ptr<ObjectInfo> class_info = GetObjectInfo( _WXSTR(class_name) );

				TiXmlElement* elem_base = elem_obj->FirstChildElement("inherits");
				while (elem_base)
				{
					std::string base_name = elem_base->Attribute(CLASS_TAG);
					// Añadimos la referencia a su clase base
					shared_ptr<ObjectInfo> base_info  = GetObjectInfo( _WXSTR(base_name) );
					if (class_info && base_info)
					{
						size_t baseIndex = class_info->AddBaseClass(base_info);
						TiXmlElement* inheritedProperty = elem_base->FirstChildElement("property");
						std::string prop_name, value;
						while( inheritedProperty )
						{
							prop_name = inheritedProperty->Attribute(NAME_TAG);
							value = inheritedProperty->GetText();
							class_info->AddBaseClassDefaultPropertyValue( baseIndex, _WXSTR(prop_name), _WXSTR(value) );
							inheritedProperty = inheritedProperty->NextSiblingElement("property");
						}
					}
					elem_base = elem_base->NextSiblingElement("inherits");
				}

				// vamos a añadir la interfaz "C++", predefinida para los componentes
				// y widgets
				if (HasCppProperties(class_info->GetObjectTypeName()))
				{
					shared_ptr<ObjectInfo> cpp_interface = GetObjectInfo( wxT("C++") );
					if (cpp_interface)
						class_info->AddBaseClass(cpp_interface);
				}

				elem_obj = elem_obj->NextSiblingElement(OBJINFO_TAG);
			}
		}
	}
}

bool ObjectDatabase::HasCppProperties(wxString type)
{
	return (type == wxT("notebook")			||
			type == wxT("flatnotebook")		||
			type == wxT("listbook")			||
			type == wxT("choicebook")		||
			type == wxT("widget")			||
			type == wxT("expanded_widget")	||
			type == wxT("statusbar")			||
			type == wxT("component")			||
			type == wxT("container")			||
			type == wxT("menubar")			||
			type == wxT("toolbar")			||
			type == wxT("splitter")
			);
}

void ObjectDatabase::LoadCodeGen( std::string file)
{
	TiXmlDocument doc(m_xmlPath + '/' + file);
	if (doc.LoadFile())
	{
		// leemos la descripcion de generacion de código
		TiXmlElement* elem_codegen = doc.FirstChildElement("codegen");
		if (elem_codegen)
		{
			std::string language = elem_codegen->Attribute("language");

			// leemos cada plantilla de código
			TiXmlElement* elem_templates = elem_codegen->FirstChildElement("templates");
			while (elem_templates)
			{
				std::string class_name = elem_templates->Attribute("class");
				TiXmlElement *elem_template = elem_templates->FirstChildElement("template");
				shared_ptr<CodeInfo> code_info(new CodeInfo());

				while (elem_template)
				{
					std::string template_name = elem_template->Attribute("name");
					std::string template_code;

					TiXmlNode * lastChild = elem_template->LastChild();
					if ( lastChild )
					{
						TiXmlText * elem_code = lastChild->ToText();
						if (elem_code)
							template_code = elem_code->Value();
					}

					code_info->AddTemplate( _WXSTR(template_name), _WXSTR(template_code) );
					elem_template = elem_template->NextSiblingElement("template");
				}

				shared_ptr<ObjectInfo> obj_info = GetObjectInfo( _WXSTR(class_name) );
				if (obj_info)
				{
					obj_info->AddCodeInfo( _WXSTR(language), code_info);
				}

				elem_templates = elem_templates->NextSiblingElement("templates");
			}

		}
	}
}

PObjectPackage ObjectDatabase::LoadPackage( std::string file)
{
	PObjectPackage package;

	//  DEBUG_PRINT("LOADING " + file + "...");

	TiXmlDocument doc(m_xmlPath + '/' + file);
	if (doc.LoadFile())
	{
		//    DEBUG_PRINT("OK\n");
		TiXmlElement* root = doc.FirstChildElement(PACKAGE_TAG);
		if (root)
		{
			const char* name = root->Attribute(NAME_TAG);
			if ( NULL == name )
			{
				name = "poopy";
			}
			std::string pkg_name = name;
			std::string pkg_desc = root->Attribute(PKGDESC_TAG);
			const char* icon_path = root->Attribute(ICON_TAG);

			wxBitmap pkg_icon;
			if ( 0 == icon_path )
			{
				pkg_icon = AppBitmaps::GetBitmap( wxT("unknown"), 16 );
			}
			else
			{
				wxImage image( _WXSTR( m_iconPath + '/' +  root->Attribute( ICON_TAG ) ), wxBITMAP_TYPE_ANY );
				pkg_icon = wxBitmap( image.Scale( 16, 16 ) );
			}

			package = PObjectPackage (new ObjectPackage( _WXSTR(pkg_name), _WXSTR(pkg_desc), pkg_icon));


			TiXmlElement* elem_obj = root->FirstChildElement(OBJINFO_TAG);

			while (elem_obj)
			{
				std::string class_name  = elem_obj->Attribute(CLASS_TAG);
				std::string type        = elem_obj->Attribute("type");
				std::string widget;
				if (elem_obj->Attribute("widget"))
					widget = elem_obj->Attribute("widget");
				std::string icon;
				if (elem_obj->Attribute("icon"))
					icon = elem_obj->Attribute("icon");
				std::string smallIcon;
				if (elem_obj->Attribute("smallIcon"))
					smallIcon = elem_obj->Attribute("smallIcon");

				shared_ptr<ObjectInfo> obj_info( new ObjectInfo( _WXSTR(class_name), GetObjectType( _WXSTR(type) ) ) );
				if ( !icon.empty() )
				{
					wxImage img( _WXSTR( m_iconPath + '/' + icon ), wxBITMAP_TYPE_ANY );
					obj_info->SetIconFile( wxBitmap( img.Scale( ICON_SIZE, ICON_SIZE ) ) );
				}
				else
				{
					obj_info->SetIconFile( AppBitmaps::GetBitmap( wxT("unknown"), ICON_SIZE ) );
				}

				if ( !smallIcon.empty() )
				{
					wxImage img( _WXSTR( m_iconPath + '/' + smallIcon ), wxBITMAP_TYPE_ANY );
					obj_info->SetSmallIconFile( wxBitmap( img.Scale( SMALL_ICON_SIZE, SMALL_ICON_SIZE ) ) );
				}
				else
				{
					wxImage img = obj_info->GetIconFile().ConvertToImage();
					obj_info->SetSmallIconFile( wxBitmap( img.Scale( SMALL_ICON_SIZE, SMALL_ICON_SIZE ) ) );
				}

				// parseamos las propiedades
				TiXmlElement* elem_prop = elem_obj->FirstChildElement(PROPERTY_TAG);
				while (elem_prop)
				{
					std::string pname = elem_prop->Attribute(NAME_TAG);
					bool hidden = false;    //Juan
					if (elem_prop->Attribute(HIDDEN_TAG)){
						int val;
						elem_prop->Attribute(HIDDEN_TAG, &val);
						hidden = val != 0;
					}
					std::string description;
					if ( elem_prop->Attribute(DESCRIPTION_TAG))
					{
						description = elem_prop->Attribute(DESCRIPTION_TAG);
					}
					PropertyType ptype = ParsePropertyType( _WXSTR( elem_prop->Attribute("type") ) );
					std::string def_value;
					shared_ptr<OptionList> opt_list;

					//          DEBUG_PRINT("    PROPERTY: '" + pname + "'\n");

					// si la propiedad es de tipo "bitlist" debemos parsear cada
					// una de las opciones
					if (ptype == PT_BITLIST || ptype == PT_OPTION)
					{
						opt_list = shared_ptr<OptionList>(new OptionList());
						TiXmlElement *elem_opt = elem_prop->FirstChildElement("option");
						while(elem_opt)
						{
							std::string macro_name = elem_opt->Attribute(NAME_TAG);
							std::string macro_description;
							if ( elem_opt->Attribute(DESCRIPTION_TAG))
							{
								macro_description = elem_opt->Attribute(DESCRIPTION_TAG);
							}
							opt_list->AddOption( _WXSTR(macro_name), _WXSTR(macro_description) );
							elem_opt = elem_opt->NextSiblingElement("option");

							m_macroSet.insert(_WXSTR(macro_name));
							// vamos a comprobar si la macro está registrada en la aplicación
							// de los contrario mostraremos un mensaje de advertencia.
							/*{
							int macro;
							PMacroDictionary dic = MacroDictionary::GetInstance();
							if (!dic->SearchMacro(macro_name,&macro))
							wxLogWarning(wxT("Macro '%s' not defined on component library!"),_WXSTR(macro_name).c_str());
							}*/
						}
					}

					TiXmlNode * lastChild = elem_prop->LastChild();
					if( lastChild )
					{
						TiXmlText * elem_text = lastChild->ToText();
						if (elem_text)
							def_value = elem_text->Value();
					}

					// creamos la instancia del descriptor de propiedad
					// Juan
					shared_ptr<PropertyInfo> prop_info(new PropertyInfo(_WXSTR(pname),ptype,_WXSTR(def_value),_WXSTR(description),hidden,opt_list));

					// añadimos el descriptor de propiedad
					obj_info->AddPropertyInfo(prop_info);


					elem_prop = elem_prop->NextSiblingElement(PROPERTY_TAG);
				}
				/*
				// leemos la descripcion de generacion de código
				TiXmlElement* elem_codegen = elem_obj->FirstChildElement(CODEGEN_TAG);
				while (elem_codegen)
				{
				wxString language = elem_codegen->Attribute(PRGLANG_TAG);
				shared_ptr<CodeInfo> code_info(new CodeInfo());

				// leemos cada plantilla de código
				TiXmlElement* elem_template = elem_codegen->FirstChildElement(TEMPLATE_TAG);
				while (elem_template)
				{
				wxString template_name = elem_template->Attribute(NAME_TAG);
				wxString template_code;

				TiXmlText * elem_code = elem_template->LastChild()->ToText();
				if (elem_code)
				template_code = elem_code->Value();

				code_info->AddTemplate(template_name,template_code);
				elem_template = elem_template->NextSiblingElement(TEMPLATE_TAG);
				}

				obj_info->AddCodeInfo(language,code_info);
				elem_codegen = elem_codegen->NextSiblingElement(CODEGEN_TAG);
				};
				*/

				// añadimos el descriptor de objeto al registro
				m_objs.insert( map< wxString, shared_ptr< ObjectInfo > >::value_type( _WXSTR(class_name), obj_info ) );

				// y al grupo
				if ( ShowInPalette( obj_info->GetObjectTypeName() ) )
				{
					package->Add( obj_info );
				}

				elem_obj = elem_obj->NextSiblingElement( OBJINFO_TAG );
			}
		}
	}

	return package;
}

bool ObjectDatabase::ShowInPalette(wxString type)
{
	return (type == wxT("form")				||
			type == wxT("sizer")				||
			type == wxT("spacer")			||
			type == wxT("menu")				||
			type == wxT("menuitem")			||
			type == wxT("submenu")			||
			type == wxT("tool")				||
			type == wxT("notebook")			||
			type == wxT("flatnotebook")		||
			type == wxT("listbook")			||
			type == wxT("choicebook")		||
			type == wxT("widget")			||
			type == wxT("expanded_widget")	||
			type == wxT("statusbar")			||
			type == wxT("component")			||
			type == wxT("container")			||
			type == wxT("menubar")			||
			type == wxT("toolbar")			||
			type == wxT("splitter")
			);
}


void ObjectDatabase::ImportComponentLibrary(wxString libfile)
{
	typedef IComponentLibrary* (*PFGetComponentLibrary)();
	wxString path = _WXSTR(m_xmlPath) + wxT('/') + libfile;

	// intentamos cargar la DLL
	wxDynamicLibrary *library = new wxDynamicLibrary(path);

	if (library->IsLoaded())
	{
		m_libs.push_back(library);
		PFGetComponentLibrary GetComponentLibrary =
			(PFGetComponentLibrary)library->GetSymbol(wxT("GetComponentLibrary"));

		if (GetComponentLibrary)
		{
			Debug::Print( wxT("[Database::ImportComponentLibrary] Importing %s library"),
				libfile.c_str());

			IComponentLibrary *comp_lib = GetComponentLibrary();

			// importamos todos los componentes
			for (unsigned int i=0; i<comp_lib->GetComponentCount(); i++)
			{
				wxString class_name = comp_lib->GetComponentName(i);
				IComponent *comp = comp_lib->GetComponent(i);

				// buscamos la clase
				shared_ptr<ObjectInfo> class_info = GetObjectInfo( class_name );
				if (class_info)
					class_info->SetComponent(comp);
				else
					Debug::Print( wxT("[Database::ImportComponentLibrary] ObjectInfo %s not found!"),
					_STDSTR(class_name).c_str());
			}

			// Añadimos al diccionario de macros todas las macros definidas en la
			// biblioteca
			for (unsigned int i=0; i<comp_lib->GetMacroCount(); i++)
			{
				PMacroDictionary dic = MacroDictionary::GetInstance();
				wxString name = comp_lib->GetMacroName(i);
				int value = comp_lib->GetMacroValue(i);
				dic->AddMacro( name, value );
				m_macroSet.erase( name );
			}

			/*for (unsigned int i = 0; i < comp_lib->GetSynonymousCount(); i++)
			{
			PMacroDictionary dic = MacroDictionary::GetInstance();
			wxString syn = comp_lib->GetMacroSynonymous(i);
			wxString name = comp_lib->GetSynonymousName(i);
			dic->AddSynonymous(_STDSTR(syn), _STDSTR(name));
			}*/
		}
		else
			Debug::Print( wxT("[Database::ImportComponentLibrary] %s is not a valid component library"),
			libfile.c_str());

	}
	else
		Debug::Print( wxT("[Database::ImportComponentLibrary] Error loading library %s."),
		libfile.c_str());

}

PropertyType ObjectDatabase::ParsePropertyType (wxString str)
{
	PropertyType result;
	PTMap::iterator it = m_propTypes.find(str);
	if (it != m_propTypes.end())
		result = it->second;
	else
	{
		result = PT_ERROR;
		assert(false);
	}

	return result;

}

wxString  ObjectDatabase::ParseObjectType   (wxString str)
{
	return str;
}


#define PT(x,y) m_propTypes.insert(PTMap::value_type(x,y))
void ObjectDatabase::InitPropertyTypes()
{
	PT( wxT("bool"),			PT_BOOL			);
	PT( wxT("text"),			PT_TEXT			);
	PT( wxT("bitlist"),		PT_BITLIST		);
	PT( wxT("intlist"),		PT_INTLIST		);
	PT( wxT("option"),		PT_OPTION		);
	PT( wxT("macro"),		PT_MACRO		);
	PT( wxT("path"),			PT_PATH			);
	PT( wxT("wxString"), 	PT_WXSTRING		);
	PT( wxT("wxPoint"),		PT_WXPOINT		);
	PT( wxT("wxSize"),		PT_WXSIZE		);
	PT( wxT("wxFont"),		PT_WXFONT		);
	PT( wxT("wxColour"),		PT_WXCOLOUR		);
	PT( wxT("bitmap"),		PT_BITMAP		);
	PT( wxT("wxString_i18n"),PT_WXSTRING_I18N);
	PT( wxT("stringlist"),	PT_STRINGLIST	);
	PT( wxT("float"),		PT_FLOAT		);
}

bool ObjectDatabase::LoadObjectTypes()
{
	TiXmlDocument doc( m_xmlPath + '/' + "objtypes.xml" );

	if ( doc.LoadFile() )
	{
		// se realizará en dos pasos, primero importamos cada uno de los ObjectTypes
		// definidos y a continuación se añadirán los posibles ObjectTypes hijos
		// de cada uno.
		TiXmlElement* root = doc.FirstChildElement("definitions");
		if (root)
		{
			TiXmlElement* elem = root->FirstChildElement("objtype");
			while (elem)
			{
				bool hidden = false, item = false;
				std::string name = elem->Attribute("name");
				if (elem->Attribute("hidden") && std::string(elem->Attribute("hidden"))=="1")
					hidden = true;
				if (elem->Attribute("item") && std::string(elem->Attribute("item"))=="1")
					item = true;

				int id = (int)m_types.size();
				PObjectType objType( new ObjectType( _WXSTR(name), id, hidden, item ) );
				m_types.insert(ObjectTypeMap::value_type( _WXSTR(name), objType ) );

				elem = elem->NextSiblingElement("objtype");
			}

			// ahora continuamos registrando añadiendo las referencias de cada
			// posible hijo
			elem = root->FirstChildElement("objtype");
			while (elem)
			{
				std::string name = elem->Attribute("name");

				// obtenemos el objType
				PObjectType objType = GetObjectType(_WXSTR(name));
				//wxLogMessage(wxString::Format(wxT("ObjectType %s can be parent of..."),name.c_str()));
				TiXmlElement *child = elem->FirstChildElement("childtype");
				while (child)
				{
					int nmax = -1; // sin límite
					std::string childname = child->Attribute("name");
					//wxLogMessage(wxString::Format(wxT("%s"),childname.c_str()));


					if (child->Attribute("nmax"))
						nmax = TypeConv::StringToInt(_WXSTR(child->Attribute("nmax")));
					//nmax = 1;

					PObjectType childType = GetObjectType(_WXSTR(childname));
					assert(childType);

					objType->AddChildType(childType, nmax);

					child = child->NextSiblingElement("childtype");
				}
				elem = elem->NextSiblingElement("objtype");
			}
		}
	}
	else
		wxLogError(wxT("Error loading object types"));
	return true;
}

PObjectType ObjectDatabase::GetObjectType(wxString name)
{
	PObjectType type;
	ObjectTypeMap::iterator it = m_types.find(name);
	if (it != m_types.end())
		type = it->second;

	return type;
}

