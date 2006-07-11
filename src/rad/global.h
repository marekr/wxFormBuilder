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

#ifndef __GLOBAL_DATA__
#define __GLOBAL_DATA__

#include <wx/wx.h>

// Esta clase encapsula toda la información global de wxFormBuilder
// TO-DO : incluir aquí el diccionario de macros que estaba implementado
//         como un singleton.

#define GlobalData()         (GlobalApplicationData::Get())
#define GlobalDataInit()     (GlobalApplicationData::Get())
#define GlobalDataDestroy()  (GlobalApplicationData::Destroy())


// Al inicio de la aplicación llamar a GlobalDataInit();
// A la salida de la aplicación llamar a GlobalDataDestroy();
class GlobalApplicationData
{
 private:
	GlobalApplicationData()
	:
	m_fbpVerMajor( 1 ),
	m_fbpVerMinor( 3 )
	{}

  static GlobalApplicationData* s_instance;

  wxString m_projectPath;
  wxString m_exePath;

 public:
	const int m_fbpVerMajor;
	const int m_fbpVerMinor;

   static GlobalApplicationData* Get();
   static void Destroy();

   const wxString &GetProjectPath() { return m_projectPath; };
   void SetProjectPath(const wxString &path) { m_projectPath = path; };

   const wxString &GetApplicationPath() { return m_exePath; };
   void SetApplicationPath(const wxString &path) { m_exePath = path; };

};


#endif //__GLOBAL_DATA__
