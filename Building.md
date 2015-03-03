# Windows #

The easiest way to build Dynamica under Windows is using cmake-gui. All the external dependencies are already included in the Dynamica source tree, except for the Maya plug-in SDK.

When running cmake-gui, type configure and make sure the Maya location under MAYA\_BASE\_DIR is properly set, so that the plug-in SDK can be found. It can be adjusted manually if necessary.

Once you build the project, it will create a Maya plugin under libs/Release/plug-ins

# Linux #

Open a terminal window

```
cd /dynamica/Extras/MayaPlugin
export MAYA_LOCATION=/usr/autodesk/maya2012-x64
make
csh
setenv MAYA_PLUG_IN_PATH $cwd
setenv XBMLANGPATH $cwd/icons/%B
setenv MAYA_SCRIPT_PATH $cwd/scripts
/usr/autodesk/maya2012-x64/bin/maya
```

# Mac OSX #

Open a terminal window

```
cd /dynamica/Extras/MayaPlugin
make -f Makefile.mac
make -f Makefile.mac install
```

It might be necessary to adjust the location of the Maya base directory.
Either set the environment variable MAYA\_LOCATION using
```
export MAYA_LOCATION=/Applications/Autodesk/maya2010/devkit
```

or you can set it manually at the top of Makefile.mac file, using a text editor.