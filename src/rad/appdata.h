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

#ifndef __APP_DATA__
#define __APP_DATA__

#include "rad/appobserver.h"
#include "model/database.h"
#include "rad/cmdproc.h"
#include <set>

class ApplicationData : public DataObservable
{
 private:

  string m_rootDir;       // directorio raíz (mismo que el ejecutable)
  bool m_modFlag;           // flag de proyecto modificado
  PObjectDatabase m_objDb;  // Base de datos de objetos
  shared_ptr<ObjectBase> m_project;    // Proyecto
  shared_ptr<ObjectBase> m_selObj;     // Objeto seleccionado

  shared_ptr<ObjectBase> m_clipboard;
  bool m_copyOnPaste; // flag que indica si hay que copiar el objeto al pegar

  // Procesador de comandos Undo/Redo
  CommandProcessor m_cmdProc;
  string m_projectFile;


  /**
   * Comprueba las referencias cruzadas de todos los nodos del árbol
   */
  void CheckProjectTree(shared_ptr<ObjectBase> obj);

  /**
   * Resuelve un posible conflicto de nombres.
   * @note el objeto a comprobar debe estar insertado en proyecto, por tanto
   *       no es válida para arboles "flotantes".
   */
  void ResolveNameConflict(shared_ptr<ObjectBase> obj);

  /**
   * Rename all objects that have the same name than any object of a subtree.
   */
  void ResolveSubtreeNameConflicts(shared_ptr<ObjectBase> obj, shared_ptr<ObjectBase> topObj = shared_ptr<ObjectBase>());

  /**
   * Rutina auxiliar de ResolveNameConflict
   */
  void BuildNameSet(shared_ptr<ObjectBase> obj, shared_ptr<ObjectBase> top, set<string> &name_set);

  /**
   * Calcula la posición donde deberá ser insertado el objeto.
   *
   * Dado un objeto "padre" y un objeto "seleccionado", esta rutina calcula la
   * posición de inserción de un objeto debajo de "parent" de forma que el objeto
   * quede a continuación del objeto "seleccionado".
   *
   * El algoritmo consiste ir subiendo en el arbol desde el objeto "selected"
   * hasta encontrar un objeto cuyo padre sea el mismo que "parent" en cuyo
   * caso se toma la posición siguiente a ese objeto.
   *
   * @param parent objeto "padre"
   * @param selected objeto "seleccionado".
   * @return posición de insercción (-1 si no se puede insertar).
   */
  int CalcPositionOfInsertion(shared_ptr<ObjectBase> selected,shared_ptr<ObjectBase> parent);


  /**
   * Elimina aquellos items que no contengan hijos.
   *
   * Esta rutina se utiliza cuando el árbol que se carga de un fichero
   * no está bien formado, o la importación no ha sido correcta.
   */
   void RemoveEmptyItems(shared_ptr<ObjectBase> obj);

   /**
    * Eliminar un objeto.
    */
   void DoRemoveObject(shared_ptr<ObjectBase> object, bool cutObject);

   void Execute(PCommand cmd);

   /**
    * Search a size in the hierarchy of an object
    */
   shared_ptr<ObjectBase> SearchSizerInto(shared_ptr<ObjectBase> obj);

 public:
  ApplicationData(const string &rootdir = ".");

  // Operaciones sobre los datos
  bool LoadProject(const wxString &filename);
  void SaveProject(const wxString &filename);
  void NewProject();
  void ConvertProject( const wxString& path );
  void SelectObject(shared_ptr<ObjectBase> obj);
  void CreateObject(wxString name);
  void RemoveObject(shared_ptr<ObjectBase> obj);
  void CutObject(shared_ptr<ObjectBase> obj);
  void CopyObject(shared_ptr<ObjectBase> obj);
  void PasteObject(shared_ptr<ObjectBase> parent);
  void InsertObject(shared_ptr<ObjectBase> obj, shared_ptr<ObjectBase> parent);
  void MergeProject(shared_ptr<ObjectBase> project);
  void ModifyProperty(shared_ptr<Property> prop, wxString value);
  void GenerateCode( bool projectOnly = false );
  void MovePosition(shared_ptr<ObjectBase>, bool right, unsigned int num = 1);
  void MoveHierarchy( shared_ptr<ObjectBase> obj, bool up);
  void Undo();
  void Redo();
  void ToggleExpandLayout(shared_ptr<ObjectBase> obj);
  void ToggleStretchLayout(shared_ptr<ObjectBase> obj);
  void ChangeAlignment (shared_ptr<ObjectBase> obj, int align, bool vertical);
  void ToggleBorderFlag(shared_ptr<ObjectBase> obj, int border);
  void CreateBoxSizerWithObject(shared_ptr<ObjectBase> obj);

  // Servicios para los observadores
  shared_ptr<ObjectBase> GetSelectedObject();
  shared_ptr<ObjectBase> GetProjectData();
  shared_ptr<ObjectBase> GetSelectedForm();
  bool CanUndo() { return m_cmdProc.CanUndo(); }
  bool CanRedo() { return m_cmdProc.CanRedo(); }
  bool GetLayoutSettings(shared_ptr<ObjectBase> obj, int *flag, int *option,int *border);
  bool CanPasteObject();
  bool CanCopyObject();
  bool IsModified();

  PObjectPackage GetPackage(unsigned int idx)
    { return m_objDb->GetPackage(idx);}

  unsigned int GetPackageCount()
    { return m_objDb->GetPackageCount(); }

  PObjectDatabase GetObjectDatabase()
    { return m_objDb; }


  // Servicios específicos, no definidos en DataObservable
  void        SetClipboardObject(shared_ptr<ObjectBase> obj) { m_clipboard = obj; }
  shared_ptr<ObjectBase> GetClipboardObject()                { return m_clipboard; }

  string GetProjectFileName() { return m_projectFile; }
};



#endif //__APP_DATA__
