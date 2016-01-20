#!/bin/sh
#*****************************************************************************
#* Copyright (C) 2007 Auria
#*
#* This program is free software; you can redistribute it and/or
#* modify it under the terms of the GNU General Public License
#* as published by the Free Software Foundation; either version 2
#* of the License, or (at your option) any later version.
#*
#* This program is distributed in the hope that it will be useful,
#* but WITHOUT ANY WARRANTY; without even the implied warranty of
#* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#* GNU General Public License for more details.
#*
#* You should have received a copy of the GNU General Public License
#* along with this program; if not, write to the Free Software
#* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#*
#*****************************************************************************

# uses tool macdylibbundler.sf.net to integrate wxWidgets libraries
# inside the app bundle. Not necessary if you require the user to install
# them on his side, or if you use the ones that come with the system on Leopard

pushd `dirname $0` > /dev/null
cd ../../
PROJECT_ROOT=`pwd`
popd > /dev/null

PLUG_IN_PATH="$PROJECT_ROOT/output/wxFormBuilder.app/Contents/PlugIns"

FILES="-x $PROJECT_ROOT/output/wxFormBuilder.app/Contents/MacOS/wxformbuilder \
-x $PLUG_IN_PATH/libadditional.dylib \
-x $PLUG_IN_PATH/libcommon.dylib \
-x $PLUG_IN_PATH/libcontainers.dylib \
-x $PLUG_IN_PATH/liblayout.dylib"

# these are 'old' contributions that were merged into the main 
# wxWidgets repo after 2.9
if [ -f "$PLUG_IN_PATH/libwx_macu_flatnotebook-2.8_wxfb.dylib" ]; then
	FILES = "$FILES -x $PLUG_IN_PATH/libwx_macu_flatnotebook-2.8_wxfb.dylib "
fi
if [ -f "$PLUG_IN_PATH/libwx_macu_propgrid-2.8_wxfb.dylib" ]; then
	FILES = "$FILES -x $PLUG_IN_PATH/libwx_macu_propgrid-2.8_wxfb.dylib "
fi
if [ -f "$PLUG_IN_PATH/libwx_macu_scintilla-2.8_wxfb.dylib" ]; then
	FILES = "$FILES -x $PLUG_IN_PATH/libwx_macu_scintilla-2.8_wxfb.dylib "
fi
if [ -f "$PLUG_IN_PATH/libwxadditions-mini.dylib" ]; then
	FILES = "$FILES -x $PLUG_IN_PATH/libwxadditions-mini.dylib "
fi

dylibbundler -od -b -d $PROJECT_ROOT/output/wxFormBuilder.app/Contents/libs/ $FILES
