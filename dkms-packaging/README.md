The `dkms-packaging` directory includes all the files required by DKMS in order
to create Jool packages.

# Building RPM Packages (Using DKMS)

1. Install all the requirements for Jool (including DKMS). Debian also requires
   `debhelper` in order to build the debian package.

2. Download the official release .ZIP file (http://jool.mx/en/download.html) and
   extract it. (The git repository release is not as straightforward.)

		wget https://github.com/NICMx/releases/raw/master/Jool/Jool-<version>.zip
		unzip Jool-<version>.zip

   (You might want to `sudo su` at this point. TODO: Don't do that; use a
   chroot environment)

3. Rename the resulting directory to match DKMS's naming requirements.
   (`/usr/src/<name>-<version>`, all lowercase.)

		mv Jool-<version> /usr/src/jool-<version>
		cd /usr/src/jool-<version>

4. Move the RPM spec to the root:

		mv dkms-packaging/rpm/jool-dkms-mkrpm.spec .

   The rest of the `dkms-packaging` directory can (and probably should) be
   deleted. (Do be aware that this README is part of it.)

		rm -r dkms-packaging

5. Tweak the package metadata according to your needs.

		nano jool-dkms-mkrpm.spec

6. Add Jool to the DKMS tree:

		dkms add -m jool -v <jool_version>

7. Create the rpm/deb/dsc package:

		dkms mkrpm -m jool -v <jool_version> --source-only

   You can find the resulting files in `/var/lib/dkms/jool`.

# Building official .deb Packages

The packages that result from these instructions are supposed to conform to the Debian Policy Manual. If you don't care about that and just want convenient .deb packages to install Jool in a few computers, jump to the next section.

Download the official ("upstream") tarball, rename it according to Debian's requirements (ie. add ".orig") and extract it:

		wget https://github.com/NICMx/releases/raw/master/Jool/jool_<version>.tar.gz
		mv jool_<version>.tar.gz jool_<version>.orig.tar.gz
		tar -xzf jool_<version>.orig.tar.gz

Copy `debian/` (next to this README) to the directory you just created:

		cd jool-<version>
		cp -r ~/git/Jool/dkms-packaging/debian .

Update the package metadata. At a minimum,

- `debian/changelog`: Add an entry to the log.
- `debian/control`: If you haven't done so already, add yourself to `Uploaders`.

Build:

		# TODO -us and -uc are wrong
		dpkg-buildpackage -us -uc
		ls ..

> TODOs
> 
> 1. Sign the kernel modules?
> 2. Sign the upstream tarball

# Building simple .deb packages

		cd ..
		git clean -dfX # Hopefully this will clean everything.

		mv dkms-packaging/debian .
		# Prevent dpkg-buildpackage from requiring the upstream tarball.
		echo "1.0" > debian/source/format

		cd usr
		# Ready the userspace app build.
		# Recall that you need autotools to be able to run this.
		./autogen.sh
		cd ..

		# Create the packages.
		# (Ignore the source directory name warning.)
		dpkg-buildpackage -us -uc
		ls ..

`jool-dkms*.deb` is the package that installs the kernel modules, and `jool-utils*.deb` is the one that contains the userspace apps.

Notice that the `jool-utils` package will be generated for your current architecture only.

