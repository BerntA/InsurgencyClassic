#!/usr/bin/python
#
# $Header: $
#
# Insurgency Mod - Insatller Generator
# 
# Generates NSIS-based installers, .zip server packages, and .tar.bz2 
# server packages for Insurgency Mod.

import optparse; # Easy parsing of command-line options
import os.path;  # Path manipulation, allows for canonical path-names
import pysvn;    # Allow direct manipulation of Subversion repositories
import tarfile;  # Needed for .tar{,.bz2,.gz} output
import zipfile;  # Needed for .zip output
import re;       # Regular expression library used for argument checking 
import string;   # Needed for string constants

# ---------------------------------------
# Common utils
def decodeVersion(modVersion):
    if re.match("\A\d*\.\d*\.\d*\Z", modVersion):
        splitVer = modVersion.split(".");
        letter = ""
        if int(splitVer[2]) > 0:
            letter = chr(ord("a") + int(splitVer[2]) - 1);
        return (unicode(modVersion), 
                 unicode(splitVer[0] + "." + splitVer[1] + letter))
    elif re.match("\A\d*\.\d*\w?\Z", modVersion):
        splitVer = modVersion.split(".");
        ver1 = splitVer[1].rstrip(string.letters);
        ver2 = "0"
        if splitVer[1].lstrip(string.digits):
            ver2 = str(ord(splitVer[1].lstrip(string.digits)) - ord('a') + 1);
        return (unicode(splitVer[0] + "." + ver1 + "." + ver2), 
                 unicode(modVersion))
        return splitVer
    return None

# ---------------------------------------
# Win32 Client Package
class Win32ClientPacker:
    def __init__(self, modVersion, isPatch):
        self.versionName = decodeVersion(modVersion)[1];
        self.versionInfo = decodeVersion(modVersion)[0];
        self.fileName = u"Insurgency-" + self.versionName;
        self.isPatch = isPatch; 
        if isPatch:
            self.fileName += u"-Patch.exe";
        else:
            self.fileName += u"-Full.exe";        
        self.outFile = file(u"InstallConfig.nsh", "w");
        self.outFile.write(u"!macro CORE_FILES\n");

    def __del__(self):
        self.outFile.write(u"!macroend\n\n");
        if self.isPatch:
            self.outFile.write(u"!define PATCH\n\n");        
        self.outFile.write(u"OutFile \"" + self.fileName + "\"\n");
        self.outFile.write(u"!macro VERSION_INFO\n")
        self.outFile.write(u"VIProductVersion " + self.versionInfo + u".0\n");
        self.outFile.write(u"VIAddVersionKey /LANG=${LANG_ENGLISH} \"ProductVersion\" \"" + self.versionName + u"\"\n");
        self.outFile.write(u"!macroend\n")
        self.outFile.close();
    
    def filecallback(self, path, type, props):
        if len(props) > 0:
            if props[0][1].has_key( "installer:exclude" ):
                return False;
            if ( props[0][1].has_key( "installer:os" )
                 and props[0][1]["installer:os"] == "linux" ):
                return False;
        path = os.path.normpath( path );        
        if type == "d":
            path = path.replace( "insurgency", "$INSTDIR", 1 );
            self.outFile.write( "    SetOutPath \"" + path + "\"\n" );
        else:
            self.outFile.write( "    File \"" + path + "\"\n" );
        return True;

# ---------------------------------------
# Win32 Server Package
class Win32ServerPacker:
    def __init__(self, modVersion, isPatch):
        fileName = u"Insurgency-Win32-Dedicated-" + decodeVersion(modVersion)[1];
        if isPatch:
            fileName += u"-Patch.zip";
        else:
            fileName += u"-Full.zip";  
        self.outFile = zipfile.ZipFile( fileName, "w", allowZip64=True,
                                        compression=zipfile.ZIP_DEFLATED );        
    
    def __del__(self):
        # Close .zip
        self.outFile.close()
        return;
    
    def filecallback(self, path, type, props):
        if len(props) > 0:
            if props[0][1].has_key( "installer:exclude" ):
                return False;
            if ( props[0][1].has_key( "installer:os" ) 
                 and ( props[0][1]["installer:os"] == "linux"
                 or props[0][1]["installer:os"] == "client" ) ):
                return False;
        # Write file to .zip
        if type == "f":
            self.outFile.write( str( path ) );
        return True;

# ---------------------------------------
# Linux Server Package
class LinuxServerPacker:
    def __init__(self, modVersion, isPatch):
        fileName = u"Insurgency-Linux-Dedicated-" + decodeVersion(modVersion)[1];
        if isPatch:
            fileName += u"-Patch.tar.bz2";
        else:
            fileName += u"-Full.tar.bz2";  
        self.outFile = tarfile.open( fileName, 'w:bz2' );   
    
    def __del__(self):
        # Close .tar.bz2
        self.outFile.close()
        return;

    def filecallback(self, path, type, props):
        if len(props) > 0:
            if props[0][1].has_key( "installer:exclude" ):
                return False;
            if ( props[0][1].has_key( "installer:os" ) 
                 and ( props[0][1]["installer:os"] == "win32"
                 or props[0][1]["installer:os"] == "client" ) ):
                return False;
        # Write file to .tar.bz2
        if type == "f":
            self.outFile.add( str( path ) );
        return True;

# ---------------------------------------
# SVN Updater Callback
def svnCallback(event_dict):
    if (event_dict["action"] == pysvn.wc_notify_action.add or
        event_dict["action"] == pysvn.wc_notify_action.update_add):
        print "Added: " + event_dict["path"];
    elif event_dict["action"] == pysvn.wc_notify_action.update_update:
        print "Updated: " + event_dict["path"];
    elif event_dict["action"] == pysvn.wc_notify_action.delete:
        print "Deleted: " + event_dict["path"];
    return;

# ---------------------------------------
# SVN File Scanner
class SVNScanner:
    
    def __init__(self, srcRevision, dstRevision):
        self.srcRevision = None;
        if srcRevision and srcRevision.isdigit():
            self.srcRevision = pysvn.Revision(pysvn.opt_revision_kind.number, int(srcRevision))
        self.dstRevision = pysvn.Revision(pysvn.opt_revision_kind.head)
        if dstRevision and dstRevision.isdigit():
            self.dstRevision = pysvn.Revision(pysvn.opt_revision_kind.number, int(dstRevision))
        elif dstRevision and dstRevision != "HEAD":
            print u"Target revision " + dstRevision + u" invaid.  Please enter an integer, or \"HEAD\"";
            print u"Defaulting to head revision.";
        self.svnClient = pysvn.Client();
        self.svnClient.callback_notify = svnCallback;
        self.callbacks = [];
        print "Checking out SVN tree...";
        self.svnClient.checkout("https://svn.insmod.net/svn/game/Internal", 
                                 "insurgency", revision=self.dstRevision);
        self.svnClient.cleanup( "insurgency" )      
        return;
    
    def __del__(self):
        return;
    
    def addCallback(self, callback):
        self.callbacks.append(callback);

    def scanDiffs(self):
        diff = self.svnClient.diff_summarize( "insurgency",
                revision1=self.srcRevision,
                revision2=self.dstRevision,
                recurse=True, ignore_ancestry=False );
        diff.reverse();
        print diff;
        for f in diff:
            F = self.svnClient.info( "insurgency/" + f.path );
            for c in self.callbacks:
                path = "insurgency/" + f.path
                if F.kind == pysvn.node_kind.file:
                    c( path, "f", self.svnClient.proplist( path ) )
                if F.kind == pysvn.node_kind.dir:
                    c( path, "d", self.svnClient.proplist( path ) )               
    
    def scanDir(self,base,callbacks):
        subvList = self.svnClient.list( base );
        dirList = [];
        # Iterate through files list.
        for f in subvList:
            if f[0].kind == pysvn.node_kind.file:
                for c in callbacks:
                    c( f[0].path, "f", self.svnClient.proplist( f[0].path ) )
            elif f[0].kind == pysvn.node_kind.dir and f[0].path != base:
                dirList.append( f[0].path )
            elif f[0].kind == pysvn.node_kind.dir and f[0].path:
                for c in callbacks:
                    c( f[0].path, "d", self.svnClient.proplist( f[0].path ) )
        for f in dirList:            
            newCallbacks = [];
            for c in callbacks:
                if c( f, "d", self.svnClient.proplist( f ) ):
                    newCallbacks.append( c );
            if len(newCallbacks) > 0:
                self.scanDir( f, newCallbacks );
        return;
        
    
    def scanFiles(self):
        if self.srcRevision:
            self.scanDiffs()
        else:
            self.scanDir("insurgency", self.callbacks);
        return;
    
    def isPatch(self):
        return self.srcRevision;

# ---------------------------------------
# Program Entry Point
def main():
    parser = optparse.OptionParser(usage="%prog [options] <version>", version="%prog 1.0");
    parser.add_option("-r", "--revision", dest="dstRevision", 
                       help="Target SVN Revision for patch/installer", 
                       metavar="rev");
    parser.add_option("-p", "--patch", dest="srcRevision", 
                       help="Source SVN Revision for patch. "
                            " Omit for a full installer.", 
                       metavar="rev");
    parser.add_option("-w", "--win32", dest="buildWin32DS", action="store_true", 
                      help="Build Win32 dedicated server package");
    parser.add_option("-l", "--linux", dest="buildLinuxDS", action="store_true", 
                      help="Build Linux dedicated server package");
    parser.add_option("-c", "--client", dest="buildClient", action="store_true", 
                      help="Build Client installer package");
    (options, args) = parser.parse_args();
    
    if len(args) != 1 or decodeVersion(args[0]) == None:
        parser.error("Please specify exactly one version string\nof the format \"x.y.z\" or \"x.yz\".")

    svnIterator = SVNScanner(options.srcRevision, options.dstRevision)
    
    if options.buildClient:
        clientPacker = Win32ClientPacker(args[0], svnIterator.isPatch())
        svnIterator.addCallback(clientPacker.filecallback)
    if options.buildWin32DS:
        winDSPacker = Win32ServerPacker(args[0], svnIterator.isPatch())
        svnIterator.addCallback(winDSPacker.filecallback)
    if options.buildLinuxDS:
        linuxPacker = LinuxServerPacker(args[0], svnIterator.isPatch())
        svnIterator.addCallback(linuxPacker.filecallback)
    
    svnIterator.scanFiles();    
    del svnIterator;
    
    if options.buildClient:
        del clientPacker;
    if options.buildWin32DS:
        del winDSPacker
    if options.buildLinuxDS:
        del linuxPacker
    
    return;

# Make sure this isn't being called by an external script.
if __name__ == "__main__":    
    main();
