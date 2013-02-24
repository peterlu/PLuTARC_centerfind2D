Peter J. Lu
23 Feb 2012
http://www.peterlu.org
plu@fas.harvard.edu
Copyright 2013 Peter J. Lu

This document provides instructions on how to build

plu_centerfind2D 

**If you use this code, please cite in your publications:
Peter J. Lu, Peter A. Sims, Hidekazu Oki, James B. Macarthur, and David A. Weitz, 
"Target-locking acquisition with real-time confocal (TARC) microscopy,"
Optics Express Vol. 15, pp. 8702-8712 (2007).

Information on the details of the algorithm are given in Chapter 6 of: 
Peter J. Lu, "Gelation and Phase Separation of Attractive Colloids,"
Harvard University PhD Thesis (2008).

The code was originally developed for Windows, originally using Microsoft Visual Studio 6, with the Intel Compiler and Intel Performance Primitives (IPP) Library. Since 2008, the code has been recompiled and used primarily on Linux, primarily because all of the tools required are free and/or open-source, and the code runs significantly faster on the same hardware. In particular, the Intel Compiler and libraries are free for Linux, but must be purchased for Windows, and IPP is required for this code to run. Therefore, while the code should compile and run with the appropriate Intel compiler and libraries, no effort has been made to maintain that implementation of the code since around 2009. In addition, the code should build on a Macintosh system, but this is so far generally untested. If anyone is interested in testing the code on these platforms, I'd be happy to work with you on this.

The build instructions in this document therefore are a current description, starting from a standard default installation of Ubuntu Linux. I recommend Ubuntu 12.04 LTS, as it is fairly modern, quite stable, and generally well supported.

1. To download the code, install the "git" package (if not otherwise installed) in Ubuntu with Synaptic or apt-get. Create the directory structure for the project, and change to that directory in a terminal window. Then, clone the git repository to copy the code to your local machine:

git clone git@github.com:peterlu/PLuTARC_centerfind2D.git

Note that I prefer using SSH certificates as the form of security here. To generate this, go to the home directory, make a directory labeled ".ssh" as a subdirectory of that home directory, and give permission value of 700. Then run ssh-keygen -t rsa, to create a public and private key pair. The public key, id_rsa.pub, can then be copied to the github site, if necessary (those these steps should not be needed for download only, just if you want to make changes to the code).

2. Install also the following Ubuntu packages (compiler, HDF5 for file output): 
g++ 
gcc-multilib
h5utils 
hdf5-tools 
hdfview 
libhdf5-serial-1.8.4 
libhdf5-serial-dev

3. The Intel Compiler and IPP must be installed for the program to work. Register for and download the Intel C++ Composer XE 2013 for Linux (64-bit, tested on version 2013.2). Run the "install.sh" as a regular user, and follow the prompts and instructions. Make sure to install both the compiler and IPP; the other tools are not necessary.

4. The code requires the FreeImage library to load and save images. Download the source distribution from:

http://freeimage.sourceforge.net/

Decompress the distribution, and run "make", and then "make install" from the command line (tested using version 3.15.4).

5. Eclipse is the preferred IDE for the code; however, Ubuntu typically has older versions in the package manager. Best to download the latest version of Eclipse:

http://www.eclipse.org/downloads/

Choose the "Eclipse IDE for C/C++ Developers," and download for the proper platform, e.g. Linux 64-bit (tested using Eclipse Juno 4.2 SR1). Follow the install instructions, which basically entail decompressing a tar archive, and copying the entire folder to "/opt". 

6. Once Eclipse is installed, add the "EGit" plugin within Eclipse. After starting Eclipse, go to Eclipse -> Help -> Install New Software. In the field labeled "Work with," enter the address for EGit:

http://download.eclipse.org/egit/updates

Download the plugin and let it install; restart Eclipse.

7. The Intel Compiler has Eclipse integration, to expose all of the features of the Intel Compiler and libraries to the GUI within Eclipse. Follow the install instructions, which are very straightforward. [**Note that EGit might have to be uninstalled afterwards, from Eclipse -> Help -> About Eclipse SDK. Select "Installation Details", and then "Eclipse EGit", and click Uninstall at the bottom. For some reason, this removes a conflict with the Intel Compiler Eclipse integrations, and the git server functionality is still there, even after removing the plugin, strangely.**]

8. To build plu_centerfind2D using Eclipse and the Intel Compilers, create a new project from File -> New -> C++ Project, then select "Intel Toolchain for Executable on Intel 64 (v. 13.1.0)". Typically, the project should be created in the same directory as the main directory as the git repository. Note that the current version of the repository has the .cproject and .project files, so creating a new project in Eclipse may not be necessary.

A number of build options are needed to compile and link the project. Under Project -> Properties -> C/C++ Build -> Settings, in the tool settings tab:

a. Intel 64 C++ Compiler (v13.1.0) -> Performance Library Build Components: under "Use Intel Integrated Performance Primitives Libraries", select Use main libraries set (-ipp=common).

b. Intel 64 C++ Linker (v13.1.0) -> Libraries: select "Link with Dynamic Libraries (-shared-intel), and under "Additional Libraries (-l)", add an entries with the lowercase text:
freeimage
hdf5
hdf5_hl

The project should build correctly now. There may be warnings about deprecation of functions in IPP, but these can be ignored.

9. To enable EGit and the integration with Eclipse, select the project in the Project Explorer, and right click, going to the menu near the bottom "Team -> Share Project." Make sure to choose "git", and "use or create project in parent folder of project." This enables Eclipse to interact with git, with menu items, in lieu of command-prompt operation (though this is obviously still enabled). Individual files can be committed by right-clicking on the file name in the Project Explorer, and selecting Team->Commit. Other git operations can be conducted from the same submenus.

After this program builds successfully, build plu_link3dt and plu_struct3dt, so that the verification tests from https://github.com/peterlu/PLuTARC_testbed can be performed (see instructions).

This is a preliminary draft of these instructions. If things are unclear or do not function as described, please contact me so we can resolve the issues.



