# archive-logs

Iteratively archive newline separated log files.

## Status

I actively use this software for archiving [hii][hii github] chat logs.
For this purpose it currently works entirely fine as is.

## Usage Example

Consider the file `/var/log/messages` with the following content:

	Jun  2 10:27:05 somehost syslog.info syslogd exiting
	Jun  2 10:28:05 somehost syslog.info syslogd started

Running `archive-logs -k 50 /var/log /mnt/nfs/archive/logs` will
cause 50% of the data in `/var/log/messages` to be retained and the
other 50% to be archived in `/mnt/nfs/archive`. The utility will only
archive old data. As such, a file called `/mnt/nfs/archive/messages`
will exist after the invocation with the following content:

	Jun  2 10:27:05 somehost syslog.info syslogd exiting

The existing log file `/var/log/messages` will be truncated to only
store recent data, i.e. its content will be the following:

	Jun  2 10:28:05 somehost syslog.info syslogd started

## Installation

The software can be compiled using:

	make

The software makes heavy use of the Linux `sendfile(2)` system call. If
this system call is available it is highly desirable to compile with
`HAVE_SENDFILE=1`. By default a compatibility function is used which
emulates the sendfile system call in userland.

After compilation, tests can be run using:

	make check

The software can be installed globally using:

	make install

## License

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <http://www.gnu.org/licenses/>.

[hii github]: https://github.com/nmeum/hii
