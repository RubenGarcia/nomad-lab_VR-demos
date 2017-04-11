This demo is based on 
https://svn.lrz.de/repos/v2t/projects/demos/demoBunge/
Revision 1538

For use in LRZ CAVE and Powerwall.

Use in suse:
Run once:
/sw/bin/remove_cursor
On CAVE Master 1 (linux)
/sw/DTrack2/2.8.1/bin/DTrack2.bin
	(connect, activate, close, do not stop measurement)
/sw/equalizer/1.8.0/bin/vrpn_server -v -f /sw/config/vrpn-lrz-cave.cfg
On CAVE Master 1 (windows)
Dtrack2 icon on the desktop
	(connect, activate, close, do not stop measurement)
vrpn: startvrpn in the desktop complains that vrpn_server.exe does not exist. Check this.

use in ubuntu:
/sw/vrpn/latest/bin/vrpn_server -v -f  /sw/config/vrpn/lrz_cave.cfg 


Run demo (on cavemaster 2): 
./demoBunge_server /sw/config/mlib/cave_2.conf
Run demo (on cavemaster 1):
./demoBunge_server /sw/config/mlib/cave_1.conf

WAIT 5 minutes
