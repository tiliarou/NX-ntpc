# SwitchTime
Change NetworkSystemClock

## Credit
- [@thedax](https://github.com/thedax) for NX-ntpc, from which this project is forked.

## Functionality
- Change time by day/hour
- Contact a time server at http://ntp.org to set the time back to normal

## Building
Install [devkitA64 (along with libnx)](https://devkitpro.org/wiki/Getting_Started), and run `make` under the project directory.

## Disclaimer
This program changes NetworkSystemClock, which may cause a desync between console and servers. Use at your own risk! It is recommended that you only use the changed clock while offline, and change it back as soon as you are connected (either manually or using ntp.org server.)
