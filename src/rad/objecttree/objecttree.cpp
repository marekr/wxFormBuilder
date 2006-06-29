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

#include "objecttree.h"
#include "rad/bitmaps.h"
#include "utils/debug.h"
#include <wx/imaglist.h>
#include <wx/image.h>
#include "rad/menueditor.h"
#include "utils/typeconv.h"

BEGIN_EVENT_TABLE( ObjectTree, wxPanel )
	EVT_TREE_SEL_CHANGED( -1, ObjectTree::OnSelChanged )
	EVT_TREE_ITEM_RIGHT_CLICK( -1, ObjectTree::OnRightClick )	
END_EVENT_TABLE()

ObjectTree::ObjectTree( wxWindow *parent, int id )
: wxPanel( parent, id )
{
	m_tcObjects = new wxTreeCtrl(this, -1, wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS|wxTR_LINES_AT_ROOT|wxTR_DEFAULT_STYLE|wxSIMPLE_BORDER);

	wxBoxSizer* sizer_1 = new wxBoxSizer(wxVERTICAL);
    sizer_1->Add(m_tcObjects, 1, wxEXPAND, 0);
    SetAutoLayout(true);
    SetSizer(sizer_1);
    sizer_1->Fit(this);
    sizer_1->SetSizeHints(this);
}

void ObjectTree::RebuildTree()
{
	m_tcObjects->Freeze();

	shared_ptr<ObjectBase> project = GetData()->GetProjectData();

	// guardamos el valor del atributo "IsExpanded"
	// para regenerar correctamente el árbol
	// (ojo! hacerlo antes de m_map.clear())
	if (project)
	{
		assert(m_expandedMap.empty());
		SaveItemStatus(project);
	}

	// borramos toda la información previa
	m_tcObjects->DeleteAllItems();
	m_map.clear();

	if (project)
	{  
		wxTreeItemId dummy;
		AddChildren(project, dummy, true );

		// restauramos el valor del atributo "IsExpanded"
		RestoreItemStatus(project);
	}  

	m_expandedMap.clear();

	m_tcObjects->Thaw();
}  

void ObjectTree::OnSelChanged(wxTreeEvent &event)
{
	wxTreeItemId id = event.GetItem();
	if (!id.IsOk()) return;
	wxTreeItemData *item_data = m_tcObjects->GetItemData(id);

	if (item_data)
	{
		shared_ptr<ObjectBase> obj(((ObjectTreeItemData *)item_data)->GetObject());
		assert(obj);
		GetData()->SelectObject(obj);
	}
}

void ObjectTree::OnRightClick(wxTreeEvent &event)
{
	wxTreeItemId id = event.GetItem();
	wxTreeItemData *item_data = m_tcObjects->GetItemData(id);
	if (item_data)
	{  
		shared_ptr<ObjectBase> obj(((ObjectTreeItemData *)item_data)->GetObject());
		assert(obj);
		wxMenu * menu = new ItemPopupMenu(GetData(),obj);
		wxPoint pos = event.GetPoint();
		menu->UpdateUI(menu);
		PopupMenu(menu,pos.x, pos.y);
	}
}

void ObjectTree::ProjectLoaded()
{
	RebuildTree();
}

void ObjectTree::ProjectRefresh()
{
	RebuildTree();
}

void ObjectTree::ProjectSaved()
{
}

void ObjectTree::ObjectSelected(shared_ptr<ObjectBase> obj)
{
	// buscamos el item asociado al objeto lo marcamos
	// como seleccionado
	ObjectItemMap::iterator it = m_map.find(obj);
	if (it != m_map.end()) //&& m_tcObjects->GetSelection() != it->second)
	{
		m_tcObjects->SelectItem(it->second);
		m_tcObjects->SetFocus();
	}
	else
	{
#ifdef __WXFB_DEBUG__
		wxLogError(wxT("Algo pasa porque no se encuentra el item asociado al objeto"));
#endif
	}
}

void ObjectTree::ObjectCreated(shared_ptr<ObjectBase> obj)
{
	// seguro que se puede optimizar
	RebuildTree();
}

void ObjectTree::ObjectRemoved(shared_ptr<ObjectBase> obj)
{
	// seguro que se puede optimizar
	RebuildTree();
}

void ObjectTree::PropertyModified(shared_ptr<Property> prop)
{
	if (prop->GetName() == _T("name") )
	{
		ObjectItemMap::iterator it = m_map.find(prop->GetObject());
		if (it != m_map.end())
		{
			UpdateItem(it->second,it->first);
		}
	};
}

void ObjectTree::AddChildren(shared_ptr<ObjectBase> obj, wxTreeItemId &parent, bool is_root)
{
	// los sizeritems son objetos "ficticios", y no se deben mostrar en el árbol
	//if (obj->GetObjectTypeName() == "sizeritem" ||
	//    obj->GetObjectTypeName() == "notebookpage")

	if (obj->GetObjectInfo()->GetObjectType()->IsItem())
	{
		if (obj->GetChildCount() > 0)
			AddChildren(obj->GetChild(0),parent);
		else
		{
			// Si hemos llegado aquí ha sido porque el arbol no está bien formado
			// y habrá que revisar cómo se ha creado.
			wxString msg;
			shared_ptr<ObjectBase> itemParent = obj->GetParent();
			assert(parent);

			msg = wxString::Format(wxT("Item without object as child of \'%s:%s\'"),
				itemParent->GetPropertyAsString(wxT("name")).c_str(),
				itemParent->GetClassName().c_str());

			wxLogError(msg);
		}
	}
	else
	{
		wxTreeItemId new_parent;

		ObjectTreeItemData *item_data = new ObjectTreeItemData(obj);

		if (is_root)
			new_parent = m_tcObjects->AddRoot(wxT(""),-1,-1,item_data);
		else
			new_parent = m_tcObjects->AppendItem(parent,wxT(""),-1,-1,item_data);


		m_tcObjects->Expand(parent); // por defecto expandido


		// registramos el objeto
		m_map.insert(ObjectItemMap::value_type(obj,new_parent));

		// configuramos su imagen
		//int image_idx = GetImageIndex(obj->GetObjectTypeName());
		int image_idx = GetImageIndex(obj->GetObjectInfo()->GetClassName());

		m_tcObjects->SetItemImage(new_parent,image_idx);
		//m_tcObjects->SetItemSelectedImage(new_parent,image_idx);

		// y su contenido
		UpdateItem(new_parent,obj);


		// y de forma recursiva generamos el resto de hijos    
		unsigned int count = obj->GetChildCount();
		unsigned int i;
		for (i = 0; i < count ; i++)
		{
			shared_ptr<ObjectBase> child = obj->GetChild(i);
			AddChildren(child, new_parent);
		}
	}  
}


int ObjectTree::GetImageIndex (unistring name)
{
	int index = 0; //default icon

	IconIndexMap::iterator it = m_iconIdx.find(name);
	if (it != m_iconIdx.end())
		index = it->second;

	return index;
}

void ObjectTree::UpdateItem(wxTreeItemId id, shared_ptr<ObjectBase> obj)
{
	// mostramos el nombre 
	wxString class_name( _WXSTR(obj->GetClassName()) );
	shared_ptr<Property> prop = obj->GetProperty( _T("name") );
	wxString obj_name;
	if (prop)
	{
		obj_name = wxString( _WXSTR(prop->GetValue()) );
	}

	wxString text = obj_name + wxT(" : ") + class_name;

	// actualizamos el item
	m_tcObjects->SetItemText(id,text);
}

void ObjectTree::Create()
{
	// Cramos la lista de iconos obteniendo los iconos de los paquetes.
	unsigned int index = 0;
	m_iconList = new wxImageList(ICON_SIZE,ICON_SIZE);

	{
		wxBitmap icon = AppBitmaps::GetBitmap(wxT("project"), ICON_SIZE);
		m_iconList->Add(icon);
		m_iconIdx.insert(IconIndexMap::value_type( _T("_default_"),index++));
	}

	unsigned int pkg_count = GetData()->GetPackageCount();
	for (unsigned int i = 0; i< pkg_count;i++)
	{
		PObjectPackage pkg = GetData()->GetPackage(i);

		unsigned int j;
		for (j=0;j<pkg->GetObjectCount();j++)
		{
			unistring comp_name(pkg->GetObjectInfo(j)->GetClassName());
			m_iconList->Add( pkg->GetObjectInfo(j)->GetIconFile() );
			m_iconIdx.insert(IconIndexMap::value_type(comp_name,index++));
		}
	}

	m_tcObjects->AssignImageList(m_iconList);
}

void ObjectTree::SaveItemStatus(shared_ptr<ObjectBase> obj)
{
	ObjectItemMap::iterator it = m_map.find(obj);
	if (it != m_map.end()) 
	{
		wxTreeItemId id = it->second; // obtenemos el item
		m_expandedMap.insert(ItemExpandedMap::value_type(obj,m_tcObjects->IsExpanded(id)));

		unsigned int i,count = obj->GetChildCount();
		for (i = 0; i<count ; i++)
			SaveItemStatus(obj->GetChild(i));
	}
}

void ObjectTree::RestoreItemStatus(shared_ptr<ObjectBase> obj)
{
	bool isExpanded;

	ObjectItemMap::iterator item_it = m_map.find(obj);
	if (item_it != m_map.end())
	{
		wxTreeItemId id = item_it->second;

		ItemExpandedMap::iterator expand_it = m_expandedMap.find(obj);
		isExpanded = (expand_it != m_expandedMap.end()? expand_it->second : true );

		if (isExpanded)
			m_tcObjects->Expand(id);      
		else
			m_tcObjects->Collapse(id); 
	}

	unsigned int i,count = obj->GetChildCount();  
	for (i = 0; i<count ; i++)
		RestoreItemStatus(obj->GetChild(i));
}

///////////////////////////////////////////////////////////////////////////////

ObjectTreeItemData::ObjectTreeItemData(shared_ptr<ObjectBase> obj) : m_object(obj)
{}

///////////////////////////////////////////////////////////////////////////////

#define MENU_MOVE_UP    100
#define MENU_MOVE_DOWN  101
#define MENU_CUT        102
#define MENU_PASTE      103
#define MENU_EDIT_MENUS 104
#define MENU_COPY       105
#define MENU_MOVE_NEW_BOXSIZER   106
#define MENU_DELETE 107

BEGIN_EVENT_TABLE(ItemPopupMenu,wxMenu)
EVT_MENU(-1, ItemPopupMenu::OnMenuEvent)
EVT_UPDATE_UI(-1, ItemPopupMenu::OnUpdateEvent)
END_EVENT_TABLE()

ItemPopupMenu::ItemPopupMenu(DataObservable *data, shared_ptr<ObjectBase> obj)
: wxMenu(), m_data(data), m_object(obj)
{
	Append(MENU_CUT,       wxT("Cut\tCtrl+X"));
	Append(MENU_COPY,      wxT("Copy\tCtrl+C"));
	Append(MENU_PASTE,     wxT("Paste\tCtrl+V"));
	AppendSeparator();
	Append(MENU_DELETE,    wxT("Delete\tCtrl+D"));
	AppendSeparator();
	Append(MENU_MOVE_UP,   wxT("Move Up\tAlt+Up"));
	Append(MENU_MOVE_DOWN, wxT("Move Down\tAlt+Down"));
	Append(MENU_MOVE_NEW_BOXSIZER, wxT("Move into a new wxBoxSizer"));
	AppendSeparator();
	Append(MENU_EDIT_MENUS, wxT("Menu Editor..."));
}

void ItemPopupMenu::OnMenuEvent (wxCommandEvent & event)
{
	int id = event.GetId();

	switch (id)
	{
	case MENU_CUT:
		m_data->CutObject(m_object);
		break;
	case MENU_PASTE:
		m_data->PasteObject(m_object);
		break;
	case MENU_DELETE:
		m_data->RemoveObject(m_object);
		break;
	case MENU_MOVE_UP:
		m_data->MovePosition(m_object,false);
		break;
	case MENU_MOVE_DOWN:
		m_data->MovePosition(m_object,true);
		break;
	case MENU_MOVE_NEW_BOXSIZER:
		m_data->CreateBoxSizerWithObject(m_object);
		break;
	case MENU_EDIT_MENUS:
		{
			shared_ptr<ObjectBase> obj = m_data->GetSelectedObject();
			if (obj && (obj->GetClassName() == _T("wxMenuBar") || obj->GetClassName() == _T("Frame") ) )
			{
				MenuEditor me(NULL);
				if (obj->GetClassName() == _T("Frame") )
				{
					bool found = false;
					shared_ptr<ObjectBase> menubar;
					for (unsigned int i = 0; i < obj->GetChildCount() && !found; i++)
					{
						menubar = obj->GetChild(i);  
						found = menubar->GetClassName() == _T("wxMenuBar"); 
					}
					if (found) obj = menubar;
				}

				if (obj->GetClassName() == _T("wxMenuBar")) me.Populate(obj);
				if (me.ShowModal() == wxID_OK)
				{ 
					if (obj->GetClassName() == _T("wxMenuBar"))
					{
						shared_ptr<ObjectBase> menubar = me.GetMenubar(m_data->GetObjectDatabase());
						while (obj->GetChildCount() > 0)
						{
							shared_ptr<ObjectBase> child = obj->GetChild(0);
							obj->RemoveChild(0);
							child->SetParent(shared_ptr<ObjectBase>());
						}
						for (unsigned int i = 0; i < menubar->GetChildCount(); i++)
						{
							shared_ptr<ObjectBase> child = menubar->GetChild(i);
							m_data->InsertObject(child,obj);
						}
					}
					else
						m_data->InsertObject(me.GetMenubar(m_data->GetObjectDatabase()),m_data->GetSelectedForm());
				}
			}
		}
		break;
	default:
		break;
	}
}

void ItemPopupMenu::OnUpdateEvent(wxUpdateUIEvent& e)
{
	switch (e.GetId())
	{
	case MENU_EDIT_MENUS:
		e.Enable(m_object && (m_object->GetClassName() == _T("wxMenuBar" )
			|| m_object->GetClassName() == _T("Frame")));
		break;
	case MENU_CUT:
	case MENU_COPY:
		e.Enable(m_data->CanCopyObject());
		break;
	case MENU_PASTE:
		e.Enable(m_data->CanPasteObject());
		break;

	case MENU_MOVE_UP: case MENU_MOVE_DOWN:
		e.Enable(m_object && m_object->GetObjectTypeName() != _T("project"));
		break;
	}
}


