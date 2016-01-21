WXFORMBUILDER
=============
This is a "fork" of wxFormBuilder is normally found here http://sourceforge.net/projects/wxformbuilder/ . The original uses a very unfriendly luamake system and development is also slowed. This "fork" attempts to rectify those issues and add new features where possible. This was primarily started to support its usage in [KiCad](http://kicad-pcb.org/)


Branches
--------
- master - Development branch, latest code, not necessarily compatible with the "official" sourceforge releases due to new controls,etc.
- wxFB3.5-RC1 - Branch that is file format compatible with the wxFB 3.5 RC1 release
- wxFB3.5-RC2 - Branch that is file format compatible with the "in progress" wxFB 3.5 RC2 Release


Building on Ubuntu
-----------------

0. If using Ubuntu 12 or older, add PPA for wxWidgets v3.0
    sudo add-apt-repository -y ppa:wxformbuilder/wxwidgets

1. Pre-requisites
    sudo apt-get install libwxgtk3.0-0 libwxgtk3.0-dev libwxgtk-media3.0-dev

2. Download source code
    git clone https://github.com/marekr/wxFormBuilder.git

3. Prepare build files
    mkdir build
    cd build
    cmake ..

4. Build

    make install

5. Test

    wxformbuilder