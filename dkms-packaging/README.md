The `dkms-packaging` directory includes all the files required by DKMS in order
to create Jool packages.

# Building DSC/DEB/RPM Packages Using DKMS

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

4. If you want to build an RPM package, move the RPM spec to the root:

		mv dkms-packaging/rpm/jool-dkms-mkrpm.spec .

   If you want to build a deb package, rename the deb spec as `jool-dkms-mkdeb`:

		mv dkms-packaging/debian jool-dkms-mkdeb

   If you want to build a dsc package, rename the deb spec as `jool-dkms-mkdsc`:

		mv dkms-packaging/debian jool-dkms-mkdsc

   Note that, if you want to build both a deb and a dsc file, you need to resort
   to copying instead of `mv`. (TODO that's bloody awkward. Why is it that way?)

   The rest of the `dkms-packaging` directory can (and probably should) be
   deleted. (Do be aware that this README is part of it.)

		rm -r dkms-packaging

5. Tweak the package metadata according to your needs. At a minimum,

	- `jool-dkms-mk<package>/changelog`: Update the version number, the
	  distribution field ("UNRELEASED" at time of writing) and add an entry
	  to the log.
	- `jool-dkms-mk<package>/control`: Add yourself to `Uploaders`.

   (TODO add RPM tweaks)

6. Add Jool to the DKMS tree:

		dkms add -m jool -v <jool_version>

7. Create the rpm/deb/dsc package:

		RPM:    dkms mkrpm -m jool -v <jool_version> --source-only
		DEB:    dkms mkdeb -m jool -v <jool_version> --source-only
		DSC:    dkms mkdsc -m jool -v <jool_version> --source-only

   You can find the resulting files in `/var/lib/dkms/jool`.

