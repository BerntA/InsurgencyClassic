Since I started out, I have improved the shader build system significantly.

Initially, it was an external installation of the SDK, but then I incorporated it into the INSmod trunk.  Finally, I made it so you could build it _without_ re-configuring all the .bat files, much as I did with the mod install files. 

Still, there are some things you need that I didn't, and don't think we should include, since not everyone needs to be able to build the shaders.

Here's a list:

- The Microsoft DirectX 9.0 SDK (August 2006 or later), which can be found at Microsoft's website.
		  I'd give you the URL, but it changes every few months, with the newest release.
- ActivePerl 5.8, which is conveniently located at http://www.activestate.com/Products/ActivePerl/
	
Once you have installed those, you need to make some directories, and copy some files.

First, you need the DirectX tools found in the "Utilities" directory of your DirectX SDK, specifically, fxc.exe, psa.exe, and vsa.exe.

Place those in a new directory under the INS tree called "dx9sdk\utilities"

Next, you need two more files from that SDK.  They're the libraries, d3d9.lib, and d3dx9.lib.

Put those in a directory called "dx9sdk\libraries"

Finally, you need perl.  The perl interpreter consists of perl.exe and perl58.dll, which must be placed
in the already present "devtools\bin" directory.

After that, the shaders are ready to be built.
