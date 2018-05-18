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

# Building DSC/DEB Packages

Prepare:

		mv debian/ ..
		cd ..
		rm -r dkms-packaging/

Update the package metadata. At a minimum,

- `debian/changelog`: Update the version number, the distribution field
  ("UNRELEASED" at time of writing) and add an entry to the log.
- `debian/control`: Add yourself to `Uploaders`.

Build:

		# Probably remove -us and -uc.
		dpkg-buildpackage -us -uc
		cd ..
		ls

