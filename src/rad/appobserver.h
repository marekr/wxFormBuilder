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

#ifndef __APP_OBSERVER__
#define __APP_OBSERVER__

#include "model/objectbase.h"
#include "model/database.h"
#include "wx/wx.h"
#include <vector>

class DataObserver;
//class ItemPopupMenu;

/**
 * Interfaz para manejar el modelo de datos de la aplicación.
 */
class DataObservable
{
 private:
   typedef vector<DataObserver *> ObserverVector;
   ObserverVector m_observers;
   bool m_lock;
   //friend class ItemPopupMenu;

 protected:
   // Notifican a cada observador el evento correspondiente
   void NotifyProjectLoaded();
   void NotifyProjectSaved();
   void NotifyObjectSelected(shared_ptr<ObjectBase> obj);
   void NotifyObjectCreated(shared_ptr<ObjectBase> obj);
   void NotifyObjectRemoved(shared_ptr<ObjectBase> obj);
   void NotifyPropertyModified(shared_ptr<Property> prop);
   void NotifyProjectRefresh();
   void NotifyCodeGeneration( bool panelOnly = false );

 public:
  DataObservable() { m_lock = false; }
  virtual ~DataObservable() {};

  // notar que los observadores se pasan como punteros normales, el motivo
  // es que, generalmente, las implementaciones de DataObserver serán objetos
  // de la GUI, por tanto es el propio framework quien gestiona la vida del
  // objeto.
  void AddDataObserver(DataObserver *o);
  void RemoveDataObserver(DataObserver *o);

  // Realizan cambios en el modelo de datos.
  virtual bool LoadProject(const wxString &filename) = 0;
  virtual void SaveProject(const wxString &filename) = 0;
  virtual void NewProject() = 0;
  virtual void SelectObject(shared_ptr<ObjectBase> obj) = 0;
  virtual void CreateObject(wxString name) = 0;
  virtual void RemoveObject(shared_ptr<ObjectBase> obj) = 0;
  virtual void CutObject(shared_ptr<ObjectBase> obj) = 0;
  virtual void CopyObject(shared_ptr<ObjectBase> obj) = 0;
  virtual void PasteObject(shared_ptr<ObjectBase> parent) = 0;
  virtual void InsertObject(shared_ptr<ObjectBase> obj, shared_ptr<ObjectBase> parent) = 0;
  virtual void MergeProject(shared_ptr<ObjectBase> project) = 0;
  virtual void ModifyProperty(shared_ptr<Property> prop, wxString value) = 0;
  virtual void GenerateCode( bool panelOnly = false ) = 0;
  virtual void MovePosition(shared_ptr<ObjectBase>, bool right, unsigned int num = 1) = 0;
  virtual void MoveHierarchy(shared_ptr<ObjectBase> obj, bool up) = 0;
  virtual void Undo() = 0;
  virtual void Redo() = 0;
  virtual void ToggleExpandLayout(shared_ptr<ObjectBase> obj) = 0;
  virtual void ToggleStretchLayout(shared_ptr<ObjectBase> obj) = 0;
  virtual void ChangeAlignment (shared_ptr<ObjectBase> obj, int align, bool vertical) = 0;
  virtual void ToggleBorderFlag(shared_ptr<ObjectBase> obj, int border) = 0;
  virtual void CreateBoxSizerWithObject(shared_ptr<ObjectBase> obj) = 0;

  // Obtienen información del modelo de datos
  virtual PObjectPackage GetPackage(unsigned int idx) = 0;
  virtual unsigned int GetPackageCount() = 0;
  virtual PObjectDatabase GetObjectDatabase() = 0;

  virtual shared_ptr<ObjectBase> GetSelectedObject() = 0;
  virtual shared_ptr<ObjectBase> GetSelectedForm() = 0;
  virtual shared_ptr<ObjectBase> GetProjectData() = 0;

  virtual bool CanUndo() = 0;
  virtual bool CanRedo() = 0;

  virtual bool GetLayoutSettings(shared_ptr<ObjectBase> obj, int *flag, int *option,int *border) = 0;
  virtual bool CanPasteObject() = 0;
  virtual bool CanCopyObject() = 0;

  virtual bool IsModified() = 0;

  virtual string GetProjectFileName() = 0;
};

/**
 * Interfaz para manejar los eventos de notificación del modelo de datos
 */
class DataObserver
{
 private:
  DataObservable *m_data;

 public:

  DataObserver() { m_data = NULL; }
  virtual ~DataObserver() {};

  // Establece el objeto observable
  void SetData(DataObservable *data) { m_data = data; };
  DataObservable *GetData() { return m_data; }

  virtual void ProjectLoaded() {}
  virtual void ProjectSaved() {}
  virtual void ObjectSelected(shared_ptr<ObjectBase> obj) {}
  virtual void ObjectCreated(shared_ptr<ObjectBase> obj) {}
  virtual void ObjectRemoved(shared_ptr<ObjectBase> obj) {}
  virtual void PropertyModified(shared_ptr<Property> prop) {}
  virtual void ProjectRefresh() {}
  virtual void CodeGeneration( bool panelOnly = false ) {}
};


#endif //__APP_OBSERVER__
