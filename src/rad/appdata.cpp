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

#include "appdata.h"
#include "utils/typeconv.h"
#include "utils/debug.h"
#include "codegen/codegen.h"
#include "rad/global.h"
#include "bitmaps.h"
#include <wx/ffile.h>

using namespace TypeConv;


///////////////////////////////////////////////////////////////////////////////
// Comandos
///////////////////////////////////////////////////////////////////////////////

/**
* Comando para insertar un objeto en el árbol.
*/
class InsertObjectCmd : public Command
{
private:
	ApplicationData *m_data;
	shared_ptr<ObjectBase> m_parent;
	shared_ptr<ObjectBase> m_object;
	int m_pos;
	shared_ptr<ObjectBase> m_oldSelected;


protected:
	void DoExecute();
	void DoRestore();

public:
	InsertObjectCmd(ApplicationData *data, shared_ptr<ObjectBase> object, shared_ptr<ObjectBase> parent, int pos = -1);
};

/**
* Comando para borrar un objeto.
*/
class RemoveObjectCmd : public Command
{
private:
	ApplicationData *m_data;
	shared_ptr<ObjectBase> m_parent;
	shared_ptr<ObjectBase> m_object;
	int m_oldPos;
	shared_ptr<ObjectBase> m_oldSelected;

protected:
	void DoExecute();
	void DoRestore();

public:
	RemoveObjectCmd(ApplicationData *data,shared_ptr<ObjectBase> object);
};

/**
* Comando para modificar una propiedad.
*/
class ModifyPropertyCmd : public Command
{
private:
	shared_ptr<Property> m_property;
	unistring m_oldValue, m_newValue;

protected:
	void DoExecute();
	void DoRestore();

public:
	ModifyPropertyCmd(shared_ptr<Property> prop, unistring value);
};

/**
* Comando para mover de posición un objeto.
*/
class ShiftChildCmd : public Command
{
private:
	shared_ptr<ObjectBase> m_object;
	int m_oldPos, m_newPos;

protected:
	void DoExecute();
	void DoRestore();

public:
	ShiftChildCmd(shared_ptr<ObjectBase> object, int pos);

};

/**
* CutObjectCmd ademas de eliminar el objeto del árbol se asegura
* de eliminar la referencia "clipboard" deshacer el cambio.
*/
class CutObjectCmd : public Command
{
private:
	// necesario para consultar/modificar el objeto "clipboard"
	ApplicationData *m_data;
	//shared_ptr<ObjectBase> m_clipboard;
	shared_ptr<ObjectBase> m_parent;
	shared_ptr<ObjectBase> m_object;
	int m_oldPos;
	shared_ptr<ObjectBase> m_oldSelected;

protected:
	void DoExecute();
	void DoRestore();

public:
	CutObjectCmd(ApplicationData *data, shared_ptr<ObjectBase> object);
};

/**
* Cambia el padre.
*/
class ReparentObjectCmd : public Command
{
private:
	shared_ptr<ObjectBase> m_sizeritem;
	shared_ptr<ObjectBase> m_sizer;
	shared_ptr<ObjectBase> m_oldSizer;
protected:
	void DoExecute();
	void DoRestore();

public:
	ReparentObjectCmd (shared_ptr<ObjectBase> sizeritem, shared_ptr<ObjectBase> sizer);
};

///////////////////////////////////////////////////////////////////////////////
// Implementación de los Comandos
///////////////////////////////////////////////////////////////////////////////

InsertObjectCmd::InsertObjectCmd(ApplicationData *data, shared_ptr<ObjectBase> object,
								 shared_ptr<ObjectBase> parent, int pos)
								 : m_data(data), m_parent(parent), m_object(object), m_pos(pos)
{
	m_oldSelected = data->GetSelectedObject();
}

void InsertObjectCmd::DoExecute()
{
	m_parent->AddChild(m_object);
	m_object->SetParent(m_parent);

	if (m_pos >= 0)
		m_parent->ChangeChildPosition(m_object,m_pos);
}

void InsertObjectCmd::DoRestore()
{
	m_parent->RemoveChild(m_object);
	m_object->SetParent(shared_ptr<ObjectBase>());
	m_data->SelectObject(m_oldSelected);
}

//-----------------------------------------------------------------------------

RemoveObjectCmd::RemoveObjectCmd(ApplicationData *data, shared_ptr<ObjectBase> object)
{
	m_data = data;
	m_object = object;
	m_parent = object->GetParent();
	m_oldPos = m_parent->GetChildPosition(object);
	m_oldSelected = data->GetSelectedObject();
}

void RemoveObjectCmd::DoExecute()
{
	m_parent->RemoveChild(m_object);
	m_object->SetParent(shared_ptr<ObjectBase>());
}

void RemoveObjectCmd::DoRestore()
{
	m_parent->AddChild(m_object);
	m_object->SetParent(m_parent);

	// restauramos la posición
	m_parent->ChangeChildPosition(m_object,m_oldPos);
	m_data->SelectObject(m_oldSelected);
}

//-----------------------------------------------------------------------------

ModifyPropertyCmd::ModifyPropertyCmd(shared_ptr<Property> prop, unistring value)
: m_property(prop), m_newValue(value)
{
	m_oldValue = prop->GetValue();
}

void ModifyPropertyCmd::DoExecute()
{
	m_property->SetValue(m_newValue);
}

void ModifyPropertyCmd::DoRestore()
{
	m_property->SetValue(m_oldValue);
}

//-----------------------------------------------------------------------------

ShiftChildCmd::ShiftChildCmd(shared_ptr<ObjectBase> object, int pos)
{
	m_object = object;
	shared_ptr<ObjectBase> parent = object->GetParent();

	assert(parent);

	m_oldPos = parent->GetChildPosition(object);
	m_newPos = pos;
}

void ShiftChildCmd::DoExecute()
{
	if (m_oldPos != m_newPos)
	{
		shared_ptr<ObjectBase> parent (m_object->GetParent());
		parent->ChangeChildPosition(m_object,m_newPos);
	}
}

void ShiftChildCmd::DoRestore()
{
	if (m_oldPos != m_newPos)
	{
		shared_ptr<ObjectBase> parent (m_object->GetParent());
		parent->ChangeChildPosition(m_object,m_oldPos);
	}
}

//-----------------------------------------------------------------------------

CutObjectCmd::CutObjectCmd(ApplicationData *data, shared_ptr<ObjectBase> object)
{
	m_data = data;
	m_object = object;
	m_parent = object->GetParent();
	m_oldPos = m_parent->GetChildPosition(object);
	m_oldSelected = data->GetSelectedObject();
}

void CutObjectCmd::DoExecute()
{
	// guardamos el clipboard ???
	//m_clipboard = m_data->GetClipboardObject();

	m_data->SetClipboardObject(m_object);
	m_parent->RemoveChild(m_object);
	m_object->SetParent(shared_ptr<ObjectBase>());
}

void CutObjectCmd::DoRestore()
{
	// reubicamos el objeto donde estaba
	m_parent->AddChild(m_object);
	m_object->SetParent(m_parent);
	m_parent->ChangeChildPosition(m_object,m_oldPos);

	// restauramos el clipboard
	//m_data->SetClipboardObject(m_clipboard);
	m_data->SetClipboardObject(shared_ptr<ObjectBase>());
	m_data->SelectObject(m_oldSelected);
}

//-----------------------------------------------------------------------------

ReparentObjectCmd ::ReparentObjectCmd (shared_ptr<ObjectBase> sizeritem, shared_ptr<ObjectBase> sizer)
{
	m_sizeritem = sizeritem;
	m_sizer = sizer;
	m_oldSizer = m_sizeritem->GetParent();
}

void ReparentObjectCmd::DoExecute()
{
	m_oldSizer->RemoveChild(m_sizeritem);
	m_sizeritem->SetParent(m_sizer);
	m_sizer->AddChild(m_sizeritem);
}

void ReparentObjectCmd::DoRestore()
{
	m_sizer->RemoveChild(m_sizeritem);
	m_sizeritem->SetParent(m_oldSizer);
	m_oldSizer->AddChild(m_sizeritem);
}

///////////////////////////////////////////////////////////////////////////////
// ApplicationData
///////////////////////////////////////////////////////////////////////////////

ApplicationData::ApplicationData(const unistring &rootdir)
{
	m_rootDir = rootdir;
	AppBitmaps::LoadBitmaps( _WXSTR(m_rootDir + _T("/xml/icons.xml") ), _WXSTR(m_rootDir + _T("/resources/icons/") ) );
	m_objDb = PObjectDatabase(new ObjectDatabase());
	m_objDb->SetXmlPath(_STDSTR( (m_rootDir + _T("/xml/")).c_str() ) ) ;
	m_objDb->SetIconPath( _STDSTR( (m_rootDir + _T("/resources/icons/")).c_str() ) );
	m_objDb->LoadObjectTypes();
	m_objDb->LoadFile();
}

shared_ptr<ObjectBase> ApplicationData::GetSelectedObject()
{
	return m_selObj;
}

shared_ptr<ObjectBase> ApplicationData::GetSelectedForm()
{
	if (m_selObj->GetObjectTypeName() == _T("form") )
		return m_selObj;
	else
		return m_selObj->FindNearAncestor( _T("form") );
}


shared_ptr<ObjectBase> ApplicationData::GetProjectData()
{
	return m_project;
}

void ApplicationData::BuildNameSet(shared_ptr<ObjectBase> obj, shared_ptr<ObjectBase> top, set<unistring> &name_set)
{
	if (obj != top)
	{
		shared_ptr<Property> nameProp = top->GetProperty( _T("name") );
		if (nameProp)
			name_set.insert(nameProp->GetValue());
	}

	for (unsigned int i=0; i< top->GetChildCount(); i++)
		BuildNameSet(obj, top->GetChild(i), name_set);
}

void ApplicationData::ResolveNameConflict(shared_ptr<ObjectBase> obj)
{
	while (obj && obj->GetObjectInfo()->GetObjectType()->IsItem())
	{
		if (obj->GetChildCount() > 0)
			obj = obj->GetChild(0);
		else
			return;
	}

	shared_ptr<Property> nameProp = obj->GetProperty( _T("name") );
	if (!nameProp)
		return;

	unistring name = nameProp->GetValue();

	// el nombre no puede estar repetido dentro del mismo form
	shared_ptr<ObjectBase> top = obj->FindNearAncestor( _T("form") );
	if (!top)
		top = m_project; // el objeto es un form.

	// construimos el conjunto de nombres
	set<unistring> name_set;
	BuildNameSet(obj, top, name_set);

	// comprobamos si hay conflicto
	set<unistring>::iterator it = name_set.find(name);
	while (it != name_set.end())
	{
		name = name + _T("_");
		it = name_set.find(name);
	}

	nameProp->SetValue(name);
}

void ApplicationData::ResolveSubtreeNameConflicts(shared_ptr<ObjectBase> obj, shared_ptr<ObjectBase> topObj)
{
	if (!topObj)
	{
		topObj = obj->FindNearAncestor( _T("form") );
		if (!topObj)
			topObj = m_project; // object is the project
	}

	// Ignore item objects
	while (obj && obj->GetObjectInfo()->GetObjectType()->IsItem())
	{
		if (obj->GetChildCount() > 0)
			obj = obj->GetChild(0);
		else
			return; // error
	}

	// Resolve a possible name conflict
	ResolveNameConflict(obj);

	// Recurse through all children
	for (unsigned int i=0 ; i < obj->GetChildCount() ; i++)
		ResolveSubtreeNameConflicts(obj->GetChild(i), topObj);
}

int ApplicationData::CalcPositionOfInsertion(shared_ptr<ObjectBase> selected,shared_ptr<ObjectBase> parent)
{
	int pos = -1;

	if (parent && selected)
	{
		shared_ptr<ObjectBase> parentSelected = selected->GetParent();
		while (parentSelected && parentSelected != parent)
		{
			selected = parentSelected;
			parentSelected = selected->GetParent();
		}

		if (parentSelected && parentSelected == parent)
			pos = parent->GetChildPosition(selected) + 1;
	}

	return pos;
}

void ApplicationData::RemoveEmptyItems(shared_ptr<ObjectBase> obj)
{
	if (!obj->GetObjectInfo()->GetObjectType()->IsItem())
	{
		bool emptyItem = true;

		// esto es un algoritmo ineficiente pero "seguro" con los índices
		while (emptyItem)
		{
			emptyItem = false;
			for (unsigned int i=0; !emptyItem && i<obj->GetChildCount(); i++)
			{
				shared_ptr<ObjectBase> child = obj->GetChild(i);
				if (child->GetObjectInfo()->GetObjectType()->IsItem() &&
					child->GetChildCount() == 0)
				{
					obj->RemoveChild(child); // borramos el item
					child->SetParent(shared_ptr<ObjectBase>());

					emptyItem = true;        // volvemos a recorrer
					wxString msg;
					msg.Printf(wxT("Empty item removed under %s"),obj->GetPropertyAsString(wxT("name")).c_str());
					wxLogWarning(msg);
				}
			}
		}
	}

	for (unsigned int i=0; i<obj->GetChildCount() ; i++)
		RemoveEmptyItems(obj->GetChild(i));
}

shared_ptr< ObjectBase >  ApplicationData::SearchSizerInto(shared_ptr<ObjectBase> obj)
{
	shared_ptr<ObjectBase> theSizer;

	if (obj->GetObjectTypeName() == _T("sizer") )
		theSizer = obj;
	else
	{
		for (unsigned int i = 0; !theSizer && i < obj->GetChildCount(); i++)
			theSizer = SearchSizerInto(obj->GetChild(i));
	}

	return theSizer;
}

///////////////////////////////////////////////////////////////////////////////

void ApplicationData::SelectObject(shared_ptr<ObjectBase> obj)
{
	Debug::Print( _T("Object Selected!") );
	m_selObj = obj;
	/*
	if (obj->GetObjectType() != T_FORM)
	{
	m_selForm = shared_dynamic_cast<FormObject>(obj->FindNearAncestor(T_FORM));
	}
	else
	m_selForm = shared_dynamic_cast<FormObject>(obj);*/

	DataObservable::NotifyObjectSelected(obj);
}

void ApplicationData::CreateObject(wxString name)
{
	Debug::Print( _T("ApplicationData::CreateObject] New %s"),name.c_str());

	shared_ptr<ObjectBase> parent = GetSelectedObject();
	shared_ptr<ObjectBase> obj;
	if (parent)
	{
		bool created = false;

		// Para que sea más práctico, si el objeto no se puede crear debajo
		// del objeto seleccionado vamos a intentarlo en el padre del seleccionado
		// y seguiremos subiendo hasta que ya no podamos crear el objeto.
		while (parent && !created)
		{
			// además, el objeto se insertará a continuación del objeto seleccionado
			obj = m_objDb->CreateObject( _STDSTR(name.c_str()),parent);

			if (obj)
			{
				int pos = CalcPositionOfInsertion(GetSelectedObject(),parent);

				PCommand command(new InsertObjectCmd(this,obj,parent,pos));
				Execute(command); //m_cmdProc.Execute(command);
				created = true;
				ResolveNameConflict(obj);
			}
			else
			{
				// lo vamos a seguir intentando con el padre, pero cuidado, el padre
				// no puede ser un item!
				parent = parent->GetParent();
				while (parent && parent->GetObjectInfo()->GetObjectType()->IsItem())
					parent = parent->GetParent();
			}
		}
	}

	DataObservable::NotifyObjectCreated(obj);

	// Seleccionamos el objeto, si este es un item entonces se selecciona
	// el objeto contenido. ¿Tiene sentido tener un item debajo de un item?
	while (obj && obj->GetObjectInfo()->GetObjectType()->IsItem())
		obj = ( obj->GetChildCount() > 0 ? obj->GetChild(0) : shared_ptr<ObjectBase>());

	if (obj)
		SelectObject(obj);
}

void ApplicationData::RemoveObject(shared_ptr<ObjectBase> obj)
{
	DoRemoveObject(obj,false);
}

void ApplicationData::CutObject(shared_ptr<ObjectBase> obj)
{
	DoRemoveObject(obj,true);
}

void ApplicationData::DoRemoveObject(shared_ptr<ObjectBase> obj, bool cutObject)
{
	// Nota:
	//  cuando se va a eliminar un objeto es importante que no se dejen
	//  nodos ficticios ("items") en las hojas del árbol.
	shared_ptr<ObjectBase> parent = obj->GetParent();
	if (parent)
	{
		while (parent && parent->GetObjectInfo()->GetObjectType()->IsItem())
		{
			obj = parent;
			parent = obj->GetParent();
		}

		if (cutObject)
		{
			m_copyOnPaste = false;
			PCommand command(new CutObjectCmd(this, obj));
			Execute(command); //m_cmdProc.Execute(command);
		}
		else
		{
			PCommand command(new RemoveObjectCmd(this,obj));
			Execute(command); //m_cmdProc.Execute(command);
		}

		DataObservable::NotifyObjectRemoved(obj);

		// "parent" será el nuevo objeto seleccionado tras eliminar obj.
		SelectObject(parent);
	}
	else
	{
		if (obj->GetObjectTypeName()!=_T("project") )
			assert(false);
	}

	CheckProjectTree(m_project);
}

void ApplicationData::CopyObject(shared_ptr<ObjectBase> obj)
{
	m_copyOnPaste = true;

	// Hacemos una primera copia del objeto, ya que si despues de copiar
	// el objeto se modificasen las propiedades, dichas modificaciones se verian
	// reflejadas en la copia.
	m_clipboard = m_objDb->CopyObject(obj);

	CheckProjectTree(m_project);
}

void ApplicationData::PasteObject(shared_ptr<ObjectBase> parent)
{
	if (m_clipboard)
	{
		// Vamos a hacer un pequeño truco, intentaremos crear un objeto nuevo
		// del mismo tipo que el guardado en m_clipboard debajo de parent.
		// El objeto devuelto quizá no sea de la misma clase que m_clipboard debido
		// a que esté incluido dentro de un "item".
		// Por tanto, si el objeto devuelto es no-nulo, entonces vamos a descender
		// en el arbol hasta que el objeto sea de la misma clase que m_clipboard,
		// momento en que cambiaremos dicho objeto por m_clipboard.
		//
		// Ejemplo:
		//
		//  m_clipboard :: wxButton
		//  parent      :: wxBoxSizer
		//
		//  obj = CreateObject(m_clipboard->GetObjectInfo()->GetClassName(), parent)
		//
		//  obj :: sizeritem
		//              /
		//           wxButton   <- Cambiamos este por m_clipboard
		shared_ptr<ObjectBase> old_parent = parent;

		shared_ptr<ObjectBase> obj =
			m_objDb->CreateObject(_STDSTR(m_clipboard->GetObjectInfo()->GetClassName().c_str() ), parent);

		int pos = -1;

		if (!obj)
		{
			// si no se ha podido crear el objeto vamos a intentar crearlo colgado
			// del padre de "parent" y además vamos a insertarlo en la posición
			// siguiente a "parent"
			shared_ptr<ObjectBase> selected = parent;
			parent = selected->GetParent();
			while (parent && parent->GetObjectInfo()->GetObjectType()->IsItem())
			{
				selected = parent;
				parent = selected->GetParent();
			}

			if (parent)
			{
				obj = m_objDb->CreateObject( _STDSTR( m_clipboard->GetObjectInfo()->GetClassName().c_str() ), parent);

				if (obj)
					pos = CalcPositionOfInsertion(selected,parent);
			}
		}

		if (obj)
		{
			shared_ptr<ObjectBase> clipboard(m_clipboard);
			if (m_copyOnPaste)
				clipboard = m_objDb->CopyObject(m_clipboard);

			shared_ptr<ObjectBase> aux = obj;
			while (aux && aux->GetObjectInfo() != clipboard->GetObjectInfo())
				aux = ( aux->GetChildCount() > 0 ? aux->GetChild(0) : shared_ptr<ObjectBase>());

			if (aux && aux != obj)
			{
				// sustituimos aux por clipboard
				shared_ptr<ObjectBase> auxParent = aux->GetParent();
				auxParent->RemoveChild(aux);
				aux->SetParent(shared_ptr<ObjectBase>());

				auxParent->AddChild(clipboard);
				clipboard->SetParent(auxParent);
			}
			else
				obj = clipboard;

			// y finalmente insertamos en el arbol
			PCommand command(new InsertObjectCmd(this,obj,parent,pos));
			Execute(command); //m_cmdProc.Execute(command);

			if (!m_copyOnPaste)
				m_clipboard.reset();

			ResolveSubtreeNameConflicts(obj);

			DataObservable::NotifyProjectRefresh();

			// vamos a mantener seleccionado el nuevo objeto creado
			// pero hay que tener en cuenta que es muy probable que el objeto creado
			// sea un "item"
			while (obj && obj->GetObjectInfo()->GetObjectType()->IsItem())
			{
				assert(obj->GetChildCount() > 0);
				obj = obj->GetChild(0);
			}

			SelectObject(obj);
		}
	}

	CheckProjectTree(m_project);
}

void ApplicationData::InsertObject(shared_ptr<ObjectBase> obj, shared_ptr<ObjectBase> parent)
{
	// FIXME! comprobar obj se puede colgar de parent
	//  if (parent->GetObjectInfo()->GetObjectType()->FindChildType(
	//    obj->GetObjectInfo()->GetObjectType()))
	//  {
	PCommand command(new InsertObjectCmd(this,obj,parent));
	Execute(command); //m_cmdProc.Execute(command);
	DataObservable::NotifyProjectRefresh();
	//  }
}

void ApplicationData::MergeProject(shared_ptr<ObjectBase> project)
{
	// FIXME! comprobar obj se puede colgar de parent
	for (unsigned int i=0; i<project->GetChildCount(); i++)
	{
		//m_project->AddChild(project->GetChild(i));
		//project->GetChild(i)->SetParent(m_project);

		shared_ptr<ObjectBase> child = project->GetChild(i);
		RemoveEmptyItems(child);

		InsertObject(child,m_project);
	}
	DataObservable::NotifyProjectRefresh();
}

void ApplicationData::ModifyProperty(shared_ptr<Property> prop, wxString str)
{
	shared_ptr<ObjectBase> object = prop->GetObject();

	if ( str.c_str() != prop->GetValue())
	{
		PCommand command(new ModifyPropertyCmd(prop,str.c_str()));
		Execute(command); //m_cmdProc.Execute(command);

		DataObservable::NotifyPropertyModified(prop);
	}
}

void ApplicationData::SaveProject(const wxString &filename)
{
	TiXmlDocument *doc = m_project->Serialize();
	m_modFlag = false;
	doc->SaveFile(filename.mb_str());
	m_projectFile = filename.c_str();
	GlobalData()->SetProjectPath(::wxPathOnly(filename));
	delete doc;

	DataObservable::NotifyProjectSaved();
}

bool ApplicationData::LoadProject(const wxString &file)
{
	Debug::Print( _T("LOADING") );

	bool result = false;

	TiXmlDocument *doc = new TiXmlDocument();
	if (doc->LoadFile(file.mb_str()))
	{
		m_objDb->ResetObjectCounters();

		wxString fbpVersion;
		TiXmlNode* firstChild = doc->FirstChild();
		TiXmlComment* version = firstChild->ToComment();
		if ( NULL != version )
		{
			fbpVersion = _WXSTR( version->ValueStr() );
		}

		if ( fbpVersion != GlobalData()->m_fbpVersion )
		{
			if ( wxYES == wxMessageBox( _("This project file is not of the current version.\nWould you to attempt automatic conversion?"), _("Wrong Version"), wxYES_NO ) )
			{
				ConvertProject( file );
				if ( !doc->LoadFile( file.mb_str() ) )
				{
					delete doc;
					return false;
				}
			}
		}

		TiXmlElement *root = doc->RootElement();
		shared_ptr<ObjectBase> proj = m_objDb->CreateObject(root);
		if (proj && proj->GetObjectTypeName() == _T("project") )
		{
			shared_ptr<ObjectBase> old_proj = m_project;
			//m_project = shared_dynamic_cast<ProjectObject>(proj);
			m_project = proj;
			m_selObj = m_project;
			result = true;
			m_modFlag = false;
			m_cmdProc.Reset();
			m_projectFile = file.c_str();
			GlobalData()->SetProjectPath(::wxPathOnly(file));
			DataObservable::NotifyProjectLoaded();
			DataObservable::NotifyProjectRefresh();
		}
	}
	delete doc;

	return result;
}

void ApplicationData::ConvertProject( const wxString& path )
{
	wxFFile proj( path );
	if ( !proj.IsOpened() )
	{
		wxLogError( wxT("Could not open project file for conversion\n%s"), path.c_str() );
	}

	wxString contents;
	proj.ReadAll( &contents );

	contents.Replace( wxT("<property name=\"option\">"), wxT("<property name=\"proportion\">") );

	proj.Close();

	wxFFile projw( path, wxT("w") );
	if ( !projw.IsOpened() )
	{
		wxLogError( wxT("Could not open project file for writing\n%s"), path.c_str() );
	}

	projw.Write( contents );
	projw.Close();

}

void ApplicationData::NewProject()
{
	m_project = m_objDb->CreateObject("Project");
	m_selObj = m_project;
	m_modFlag = false;
	m_cmdProc.Reset();
	m_projectFile = _T("");
	GlobalData()->SetProjectPath(wxT(""));
	DataObservable::NotifyProjectRefresh();
}

void ApplicationData::GenerateCode( bool panelOnly )
{
	DataObservable::NotifyCodeGeneration( panelOnly );
}

void ApplicationData::MovePosition(shared_ptr<ObjectBase> obj, bool right, unsigned int num)
{
	shared_ptr<ObjectBase> noItemObj = obj;

	shared_ptr<ObjectBase> parent = obj->GetParent();
	if (parent)
	{
		// Si el objeto está incluido dentro de un item hay que desplazar
		// el item
		while (parent && parent->GetObjectInfo()->GetObjectType()->IsItem())
		{
			obj = parent;
			parent = obj->GetParent();
		}

		unsigned int pos = parent->GetChildPosition(obj);

		// nos aseguramos de que los límites son correctos

		unsigned int children_count = parent->GetChildCount();

		if ((right && num + pos < children_count) ||
			!right  && (num <= pos))
		{
			pos = (right ? pos+num : pos-num);

			PCommand command(new ShiftChildCmd(obj,pos));
			Execute(command); //m_cmdProc.Execute(command);
			DataObservable::NotifyProjectRefresh();
			SelectObject(noItemObj);

		}
	}
}

void ApplicationData::MoveHierarchy(shared_ptr<ObjectBase> obj, bool up)
{
	shared_ptr<ObjectBase> sizeritem = obj->GetParent();

	// object must be inside a sizer
	if (sizeritem && sizeritem->GetObjectTypeName() == _T("sizeritem") )
	{
		shared_ptr<ObjectBase> nextSizer = sizeritem->GetParent(); // points to the object's sizer
		if (nextSizer)
		{
			if (up)
			{
				do
				{
					nextSizer = nextSizer->GetParent();
				}
				while (nextSizer && nextSizer->GetObjectTypeName() != _T("sizer") );

				if (nextSizer && nextSizer->GetObjectTypeName() == _T("sizer") )
				{
					PCommand cmdReparent(new ReparentObjectCmd(sizeritem,nextSizer));
					Execute(cmdReparent);
					DataObservable::NotifyProjectRefresh();
					SelectObject(obj);
				}
			}
			else
			{
				// object will be move to the top sizer of the next sibling object
				// subtree.
				unsigned int pos = nextSizer->GetChildPosition(sizeritem) + 1;

				if (pos < nextSizer->GetChildCount())
				{
					nextSizer = SearchSizerInto(nextSizer->GetChild(pos));
					if (nextSizer)
					{
						PCommand cmdReparent(new ReparentObjectCmd(sizeritem,nextSizer));
						Execute(cmdReparent);
						DataObservable::NotifyProjectRefresh();
						SelectObject(obj);
					}
				}
			}
		}
	}
}


void ApplicationData::Undo()
{
	m_cmdProc.Undo();
	DataObservable::NotifyProjectRefresh();
	CheckProjectTree(m_project);
	DataObservable::NotifyObjectSelected(GetSelectedObject());
}

void ApplicationData::Redo()
{
	m_cmdProc.Redo();
	DataObservable::NotifyProjectRefresh();
	CheckProjectTree(m_project);
	DataObservable::NotifyObjectSelected(GetSelectedObject());
}


void ApplicationData::ToggleExpandLayout(shared_ptr<ObjectBase> obj)
{
	if (!obj)
	{
		return;
	}

	shared_ptr<ObjectBase> object;
	shared_ptr<ObjectBase> parent = obj->GetParent();
	if ( obj->GetObjectTypeName() == _T("spacer") )
	{
		object = obj;
	}
	else if ( parent && parent->GetObjectTypeName() == _T("sizeritem") )
	{
		object = parent;
	}
	else
	{
		return;
	}

	shared_ptr<Property> propFlag = object->GetProperty( _T("flag") );
	assert(propFlag);

	wxString value;
	wxString currentValue = propFlag->GetValueAsString();

	value =
		(TypeConv::FlagSet(wxT("wxEXPAND"),currentValue) ?
		TypeConv::ClearFlag(wxT("wxEXPAND"),currentValue) :
	TypeConv::SetFlag(wxT("wxEXPAND"),currentValue));

	ModifyProperty(propFlag,value);
}

void ApplicationData::ToggleStretchLayout(shared_ptr<ObjectBase> obj)
{
	if (!obj)
	{
		return;
	}

	shared_ptr<ObjectBase> object;
	shared_ptr<ObjectBase> parent = obj->GetParent();
	if ( obj->GetObjectTypeName() == _T("spacer") )
	{
		object = obj;
	}
	else if ( parent && parent->GetObjectTypeName() == _T("sizeritem") )
	{
		object = parent;
	}
	else
	{
		return;
	}

	shared_ptr<Property> propOption = object->GetProperty( _T("proportion") );
	assert(propOption);

	unistring value = ( propOption->GetValue() == _T("1") ? _T("0") : _T("1") );

	ModifyProperty(propOption, _WXSTR(value));
}

void ApplicationData::CheckProjectTree(shared_ptr<ObjectBase> obj)
{
	assert(obj);
	for (unsigned int i=0; i< obj->GetChildCount(); i++)
	{
		shared_ptr<ObjectBase> child = obj->GetChild(i);
		if (child->GetParent() != obj)
			wxLogError(wxString::Format(wxT("Parent of object \'%s\' is wrong!"),child->GetPropertyAsString(wxT("name")).c_str()));

		CheckProjectTree(child);
	}
}

bool ApplicationData::GetLayoutSettings(shared_ptr<ObjectBase> obj, int *flag, int *option,int *border)
{
	if (obj)
	{
		shared_ptr<ObjectBase> parent = obj->GetParent();
		if ( parent && parent->GetObjectTypeName() == _T("sizeritem") )
		{
			shared_ptr<Property> propOption = parent->GetProperty( _T("proportion") );
			shared_ptr<Property> propFlag   = parent->GetProperty( _T("flag") );
			shared_ptr<Property> propBorder = parent->GetProperty( _T("border") );
			assert(propOption && propFlag && propBorder);

			*option = propOption->GetValueAsInteger();
			*flag   = propFlag->GetValueAsInteger();
			*border = propBorder->GetValueAsInteger();

			return true;
		}
		else if ( obj->GetObjectTypeName() == _T("spacer") )
		{
			shared_ptr<Property> propOption = obj->GetProperty( _T("proportion") );
			shared_ptr<Property> propFlag   = obj->GetProperty( _T("flag") );
			shared_ptr<Property> propBorder = obj->GetProperty( _T("border") );
			assert(propOption && propFlag && propBorder);

			*option = propOption->GetValueAsInteger();
			*flag   = propFlag->GetValueAsInteger();
			*border = propBorder->GetValueAsInteger();

			return true;
		}
	}
	return false;
}

void ApplicationData::ChangeAlignment (shared_ptr<ObjectBase> obj, int align, bool vertical)
{
	if (!obj)
	{
		return;
	}

	shared_ptr<ObjectBase> object;
	shared_ptr<ObjectBase> parent = obj->GetParent();
	if ( obj->GetObjectTypeName() == _T("spacer") )
	{
		object = obj;
	}
	else if ( parent && parent->GetObjectTypeName() == _T("sizeritem") )
	{
		object = parent;
	}
	else
	{
		return;
	}

	shared_ptr<Property> propFlag = object->GetProperty( _T("flag") );
	assert(propFlag);

	wxString value = propFlag->GetValueAsString();

	// Primero borramos los flags de la configuración previa, para así
	// evitar conflictos de alineaciones.
	if (vertical)
	{
		value = TypeConv::ClearFlag(wxT("wxALIGN_TOP"), value);
		value = TypeConv::ClearFlag(wxT("wxALIGN_BOTTOM"), value);
		value = TypeConv::ClearFlag(wxT("wxALIGN_CENTER_VERTICAL"), value);
	}
	else
	{
		value = TypeConv::ClearFlag(wxT("wxALIGN_LEFT"), value);
		value = TypeConv::ClearFlag(wxT("wxALIGN_RIGHT"), value);
		value = TypeConv::ClearFlag(wxT("wxALIGN_CENTER_HORIZONTAL"), value);
	}

	wxString alignStr;
	switch (align)
	{
	case wxALIGN_RIGHT:
		alignStr = wxT("wxALIGN_RIGHT");
		break;
	case wxALIGN_CENTER_HORIZONTAL:
		alignStr = wxT("wxALIGN_CENTER_HORIZONTAL");
		break;
	case wxALIGN_BOTTOM:
		alignStr = wxT("wxALIGN_BOTTOM");
		break;
	case wxALIGN_CENTER_VERTICAL:
		alignStr = wxT("wxALIGN_CENTER_VERTICAL");
		break;
	}

	value = TypeConv::SetFlag(alignStr, value);
	ModifyProperty(propFlag,value);
}

void ApplicationData::ToggleBorderFlag(shared_ptr<ObjectBase> obj, int border)
{
	if (!obj)
	{
		return;
	}

	shared_ptr<ObjectBase> borderObject;
	shared_ptr<ObjectBase> parent = obj->GetParent();
	if ( obj->GetObjectTypeName() == _T("spacer") )
	{
		borderObject = obj;
	}
	else if ( parent && parent->GetObjectTypeName() == _T("sizeritem") )
	{
		borderObject = parent;
	}
	else
	{
		return;
	}

	shared_ptr<Property> propFlag = borderObject->GetProperty( _T("flag") );
	assert(propFlag);

	wxString value = propFlag->GetValueAsString();

	value = TypeConv::ClearFlag(wxT("wxALL"), value);
	value = TypeConv::ClearFlag(wxT("wxTOP"), value);
	value = TypeConv::ClearFlag(wxT("wxBOTTOM"), value);
	value = TypeConv::ClearFlag(wxT("wxRIGHT"), value);
	value = TypeConv::ClearFlag(wxT("wxLEFT"), value);

	int intVal = propFlag->GetValueAsInteger();
	intVal ^= border;

	if ((intVal & wxALL) == wxALL)
		value = TypeConv::SetFlag(wxT("wxALL"), value);
	else
	{
		if ((intVal & wxTOP) != 0) value = TypeConv::SetFlag(wxT("wxTOP"), value);
		if ((intVal & wxBOTTOM) != 0) value = TypeConv::SetFlag(wxT("wxBOTTOM"), value);
		if ((intVal & wxRIGHT) != 0) value = TypeConv::SetFlag(wxT("wxRIGHT"), value);
		if ((intVal & wxLEFT) != 0) value = TypeConv::SetFlag(wxT("wxLEFT"), value);
	}

	ModifyProperty(propFlag, value);
}

void ApplicationData::CreateBoxSizerWithObject(shared_ptr<ObjectBase> obj)
{
	shared_ptr<ObjectBase> sizer, sizeritem;

	sizeritem = obj->GetParent();
	if (sizeritem && sizeritem->GetObjectTypeName() == _T("sizeritem") )
	{
		sizer = sizeritem->GetParent();
		unsigned int childPos = sizer->GetChildPosition(sizeritem);

		// creamos un wxBoxSizer
		shared_ptr<ObjectBase> newSizer = m_objDb->CreateObject("wxBoxSizer",sizer);
		if (newSizer)
		{
			PCommand cmd(new InsertObjectCmd(this,newSizer,sizer,childPos));
			Execute(cmd);

			if (newSizer->GetObjectTypeName() == _T("sizeritem") )
				newSizer = newSizer->GetChild(0);

			PCommand cmdReparent(new ReparentObjectCmd(sizeritem,newSizer));
			Execute(cmdReparent);
			DataObservable::NotifyProjectRefresh();
		}
	}
}

bool ApplicationData::CanPasteObject()
{
	shared_ptr<ObjectBase> obj = GetSelectedObject();
	if (obj && obj->GetObjectTypeName() != _T("project") )
		return (m_clipboard != NULL);

	return false;
}

bool ApplicationData::CanCopyObject()
{
	shared_ptr<ObjectBase> obj = GetSelectedObject();
	if (obj && obj->GetObjectTypeName() != _T("project") )
		return true;

	return false;
}

bool ApplicationData::IsModified()
{
	return m_modFlag;
}

void ApplicationData::Execute(PCommand cmd)
{
	m_modFlag = true;
	m_cmdProc.Execute(cmd);
}

//////////////////////////////////////////////////////////////////////////////
