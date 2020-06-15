# archive-logs

Iteratively archive newline separated log files.

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
store recent data, i.e. its content will looks as follows:

	Jun  2 10:28:05 somehost syslog.info syslogd started

## Installation

The program makes use of the linux `sendfile(2)` system call, if this
system call is available compile the software using:

	make HAVE_SENDFILE=1

If the system call is not available, a compact function will be used.
This function can be activated by compiling with `HAVE_SENDFILE=0`
(the default). Disclaimer: The compat function is less well tested.

Afterwards, the software can be installed globally using:

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
