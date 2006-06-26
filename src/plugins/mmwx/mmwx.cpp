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
// Written by José Antonio Hurtado - joseantonio.hurtado@hispalinux.es
//
///////////////////////////////////////////////////////////////////////////////

#include "../component.h"
#include "../plugin.h"
#include "wx/grid.h"
#include "mmwx/mmMultiButton.h"
#include "mmwx/mmMonthCtrl.h"

///////////////////////////////////////////////////////////////////////////////
// Desgraciadamente el componente MultiButton necesita crear dinámicamente
// los Bitmaps, sin embargo no se preocupa de liberar los recursos.
// Dado que no tenemos un mecanismo para saber cuando se destruye la instancia
// vamos a manejar la memoria desde la propia DLL poniendo un límite máximo
// de bitmaps en memoria y liberando los más viejos.
///////////////////////////////////////////////////////////////////////////////
#define MAX_BITMAPS 50
#include <list>
class BitmapManager
{
 private:
  static BitmapManager *s_instance;
  typedef list<wxBitmap *> BitmapList;
  BitmapList m_bitmapList;
  BitmapManager() {};
 public:
  static BitmapManager * GetInstance();
  void AddBitmap (wxBitmap *bitmap);    
};

BitmapManager * BitmapManager::s_instance = NULL;
BitmapManager * BitmapManager::GetInstance()
{
  if (s_instance == NULL)
    s_instance = new BitmapManager();
  
  return s_instance;
}

void BitmapManager::AddBitmap (wxBitmap *bitmap)
{
  m_bitmapList.push_back(bitmap);
  if (m_bitmapList.size() >= MAX_BITMAPS)
  {
    wxBitmap *bitmap = m_bitmapList.front();
    delete bitmap;
    m_bitmapList.pop_front();
  }
}

///////////////////////////////////////////////////////////////////////////////


class MultiButtonComponent : public ComponentBase
{
  wxObject* Create(IObject *obj, wxObject *parent)
  {  
    BitmapManager *mgr = BitmapManager::GetInstance();
    
    wxBitmap *default_bitmap;
    wxBitmap *focus_bitmap;
    wxBitmap *toggle_bitmap;
    wxBitmap *sel_bitmap;
    
    default_bitmap = new wxBitmap(obj->GetPropertyAsBitmap(_("default_bp")));
    focus_bitmap = new wxBitmap(obj->GetPropertyAsBitmap(_("focus_bp")));      
    sel_bitmap = new wxBitmap(obj->GetPropertyAsBitmap(_("sel_bp")));
    toggle_bitmap = new wxBitmap(obj->GetPropertyAsBitmap(_("toggle_bp")));
    
    mgr->AddBitmap(default_bitmap);
    mgr->AddBitmap(focus_bitmap);
    mgr->AddBitmap(toggle_bitmap);
    mgr->AddBitmap(sel_bitmap);
          
    mmMultiButton *button = new mmMultiButton((wxWindow*)parent, -1,
      obj->GetPropertyAsString(_("label")),
      *default_bitmap,
      obj->GetPropertyAsPoint(_("pos")),
      obj->GetPropertyAsSize(_("size")),
      obj->GetPropertyAsInteger(_("style")));


    button->SetFocusBitmap(*focus_bitmap);
    button->SetSelectedBitmap(*sel_bitmap);
    button->SetToggleBitmap(*toggle_bitmap);
    
    return button;
  }
};

class MonthCtrlComponent : public ComponentBase
{
  wxObject* Create(IObject *obj, wxObject *parent)
  {  
    return new mmMonthCtrl((wxWindow*)parent,-1, wxDateTime::Today(),
      obj->GetPropertyAsPoint(_("pos")),
      obj->GetPropertyAsSize(_("size")),
      obj->GetPropertyAsInteger(_("style")));
  }
};

///////////////////////////////////////////////////////////////////////////////

BEGIN_LIBRARY()
  COMPONENT("mmMultiButton",MultiButtonComponent)
  MACRO(mmMB_FOCUS)
  MACRO(mmMB_SELECT)
  MACRO(mmMB_TOGGLE)
  MACRO(mmMB_DROPDOWN)
  MACRO(mmMB_WHOLEDROPDOWN)
  MACRO(mmMB_STATIC)
  MACRO(mmMB_NO_AUTOSIZE)
  MACRO(mmMB_AUTODRAW)

  COMPONENT("mmMonthCtrl",MonthCtrlComponent)
  MACRO(mmSHOW_TODAY)
  MACRO(mmSHOW_SELECT)
  MACRO(mmSHOW_MOVE)
  MACRO(mmSHOW_BORDER_DAYS)
  MACRO(mmSHOW_ALL)
  MACRO(mmPREV_BTN)
  MACRO(mmNEXT_BTN)
  MACRO(mmRESCALE_FONTS)
END_LIBRARY()

