
If someone is interested to help out, we can replace PhysX by Bullet, to create
a 3ds max plugin for Bullet, exporting to the .bullet format.

Introduction
============
This is the current development version of the PhysX plugin for Autodesk 3ds Max.

The original code for the plugin was developed by AGEIA Technologies (www.ageia.com) 
and please note that this plugin is not endorsed by Autodesk. You will need a license
of Autodesk 3ds Max in order to compile and use the plugin and a copy of the 
AGEIA PhysX SDK.


Getting Started / Installation
==============================
There are project- and solution files provided for Microsoft Visual Studio 2003 
and 2005, but they need to be modified in order to fit into your installation paths.

The aim is to use XPJ (xpj.sourceforge.net) as a simple means of creating the 
project files together with a few environment variables. Currently this is not 
possible though, since XPJ can't handle environment variables. Steps to be taken
for installation are currently:
1. Download the PhysX plugin code.
2. Install XPJ (download from xpj.sourceforge.net).
3. Set the two environment variables (and adjust to fit your installations):
     a. STUDIOMAX_ROOT = d:\3dsmax9
     b. STUDIOMAX_SDK = d:\3ds Max 9 SDK
     c. PHYSXSDK_ROOT  = C:\Program Files\AGEIA Technologies\AGEIA PhysX SDK\v2.7.0
4. Edit the compiler\xpj\pxplugin.xpj file and change the source path of the NxuStream2 and Softbody
   files to match your installation [this step will be redundant once XPJ supports
   environment variables].
5. Run compiler\xpj\create_projects.bat
6. Open the project file in compiler\vc71win32 (or vc8win32).
7. Compile the project in either debug or release (make sure that the output 
   path is correct, or copy the resulting DLM file to 3dsmax\plugins).
8. Copy the scripts to 3dsmax\scripts (both the startup and physx folders).


Binary Availability
===================
The plugin will also be published in a binary version (with an installer), where
it is compiled against the current version(s) of AGEIA PhysX, for those who don't
want to download and compile it themselves. Please go to physxplugin.sourceforge.net
for more downloading the binaries.


Documentation
=============
1. User documentation
The plugin has recently had its graphical user interface altered a lot, so the
user documentation still needs to be updated. It will be uploaded to the project
as soon as there is a first version good enough.

2. Developer documentation
Information about how the plugin code is structured also needs to be written before
it is committed to the project. Basically the code should be somewhat self-explanatory
for someone used to writing plugins for Autodesk 3ds Max, but a light-weight introduction
will be provided.


Folder Overview
===============
compiler    -   XPJ file and project files for Visual Studio
scripts     -   MaxScript files for the plugin
src         -   C++ source code for the plugin

