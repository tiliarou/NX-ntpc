# NX-ntpc
A crude NTP client for the Switch.

# What's it do?
It contacts a time server at http://ntp.org and sets the Nintendo Switch's clock appropriately.

# How do I compile it?
Install [devkitA64 (along with libnx)](https://devkitpro.org/wiki/Getting_Started), browse to the directory containing this project's `Makefile`, and type `make`.

# How do I use it?
Run the NRO from the hbmenu, or send it over nxlink. An internet connection is required for this application to work, but it does not contact Nintendo's servers, so it should be friendly in 90DNS and other Nintendo-blocked environments.  If "Synchronise Clock via Internet" (in System Settings -> System -> Date & Time) is enabled before launching this application, the time will automatically update correctly on the console's Home Screen. If it is not enabled, the application will ask if you wish to enable it, and then reboot afterward to correct the time, or offer a way to exit without enabling internet time sync.


*Sample output of a successful use.*
![image](https://i.imgur.com/fu7bE87.png)
