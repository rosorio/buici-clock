# buici-clock

Buici-Clock is a project created by Marc Singer <elf@netcom.com>

This repo integrate the following Debian patches:

    * Remove rename and use res_y.hxx
    * Implement showSecondHand
    * Enable cross building
    * Update configure.in for autoconf 2.71
    * Add missing dependencies
    * Remove generated files
    * Additional fix for symbolic link creation in Makefile
    * Do not embed build date in README, preventing reproducible build
    * Use timeout instead of signal to make the clock tick

It also integrate the following evolutions:

    * New switch to set clock timezone

## New switch to set clock timezone

  -tz [<timezone_name>](https://en.wikipedia.org/wiki/List_of_tz_database_time_zones#List)

Eg: buici-clock -tz Etc/GMT+9
