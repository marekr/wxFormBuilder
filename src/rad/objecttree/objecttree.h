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

#ifndef __OBJECT_TREE__
#define __OBJECT_TREE__

#include "rad/appobserver.h"
#include "model/objectbase.h"
//#include <hash_map>
#include <map>
#include "rad/customkeys.h"

#include <wx/treectrl.h>

class ObjectTree : public wxPanel, public DataObserver
{
private:
   typedef map<shared_ptr<ObjectBase>, wxTreeItemId> ObjectItemMap;
   typedef map<unistring, int> IconIndexMap;
   typedef map<shared_ptr<ObjectBase>, bool> ItemExpandedMap;
   
   ObjectItemMap m_map;
   ItemExpandedMap m_expandedMap;
   
   wxImageList *m_iconList;
   IconIndexMap m_iconIdx;

   wxTextCtrl* m_txtSelected;
   wxTreeCtrl* m_tcObjects;
   
   /**
    * Crea el arbol completamente.
    */
   void RebuildTree();
   void AddChildren(shared_ptr<ObjectBase> child, wxTreeItemId &parent, bool is_root = false);
   int GetImageIndex (unistring type);
   void UpdateItem(wxTreeItemId id, shared_ptr<ObjectBase> obj);
   
   void SaveItemStatus(shared_ptr<ObjectBase> obj);
   void RestoreItemStatus(shared_ptr<ObjectBase> obj);
   
   DECLARE_EVENT_TABLE()
   
public:
  ObjectTree(wxWindow *parent, int id);
  void Create();
  
  void OnSelChanged(wxTreeEvent &event);
  void OnRightClick(wxTreeEvent &event);
   
  void ProjectLoaded();
  void ProjectSaved();
  void ObjectSelected(shared_ptr<ObjectBase> obj);
  void ObjectCreated(shared_ptr<ObjectBase> obj);
  void ObjectRemoved(shared_ptr<ObjectBase> obj);
  void PropertyModified(shared_ptr<Property> prop);
  void ProjectRefresh();
  
  void AddCustomKeysHandler(CustomKeysEvtHandler *h) { m_tcObjects->PushEventHandler(h); };
};

/**
 * Gracias a que podemos asociar un objeto a cada item, esta clase nos va
 * a facilitar obtener el objeto (ObjectBase) asociado a un item para 
 * seleccionarlo pinchando en el item.
 */
class ObjectTreeItemData : public wxTreeItemData
{
 private:
  shared_ptr<ObjectBase> m_object;   
 public:
  ObjectTreeItemData(shared_ptr<ObjectBase> obj);
  shared_ptr<ObjectBase> GetObject() { return m_object; }
};

/**
 * Menu popup asociado a cada item del arbol.
 *
 * Este objeto ejecuta los comandos incluidos en el menu referentes al objeto
 * seleccionado.
 */
class ItemPopupMenu : public wxMenu
{
 private:
  DataObservable *m_data;
  shared_ptr<ObjectBase> m_object;
  
  DECLARE_EVENT_TABLE()
     
 public:
  void OnUpdateEvent(wxUpdateUIEvent& e);
  ItemPopupMenu(DataObservable *data, shared_ptr<ObjectBase> obj);
  void OnMenuEvent (wxCommandEvent & event);
};
 
#endif //__OBJECT_TREE__
