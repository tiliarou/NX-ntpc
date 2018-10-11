# NX-ntpc
A crude NTP client for the Switch.

# What's it do?
It contacts a time server at http://ntp.org and sets the Nintendo Switch's clock appropriately. Note however, that it will not set the clock that you can see on the Home Screen, unless "Synchronise Clock via Internet" (in System Settings -> System -> Date & Time) is already turned on.

# How do I compile it?
Install [devkitA64 (along with libnx)](https://devkitpro.org/wiki/Getting_Started), browse to the directory containing this project's `Makefile`, and type `make`.

# How do I use it?
Run the NRO from the hbmenu, or send it over nxlink. An internet connection is required for this program to work, but it does not contact Nintendo's servers, so it should be friendly in 90DNS and other Nintendo-blocked environments.


*Sample output of a successful use.*
![image](https://i.imgur.com/fu7bE87.png)
