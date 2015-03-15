## ioping ##

This tool lets you monitor I/O latency in real time.
It shows disk latency in the same way as ping shows network latency.

```
Usage: ioping [-LABCDWRq] [-c count] [-w deadline] [-pP period] [-i interval]
               [-s size] [-S wsize] [-o offset] directory|file|device
        ioping -h | -v

      -c <count>      stop after <count> requests
      -w <deadline>   stop after <deadline>
      -p <period>     print raw statistics for every <period> requests
      -P <period>     print raw statistics for every <period> in time
      -i <interval>   interval between requests (1s)
      -s <size>       request size (4k)
      -S <wsize>      working set size (1m)
      -o <offset>     working set offset (0)
      -k              keep and reuse temporary working file
      -L              use sequential operations (includes -s 256k)
      -A              use asynchronous I/O
      -C              use cached I/O
      -D              use direct I/O
      -W              use write I/O *DANGEROUS*
      -R              seek rate test (same as -q -i 0 -w 3 -S 64m)
      -B              print final statistics in raw format
      -q              suppress human-readable output
      -h              display this message and exit
      -v              display version and exit
```

See also: [man](man.md)


---


### Supported OS ###
  * GNU/Linux
  * GNU/HURD
  * Windows
  * OS X
  * FreeBSD
  * DragonFlyBSD
  * OpenBSD

### Examples ###

Show disk I/O latency using the default values and the current directory, until interrupted

```
$ ioping .
4096 bytes from . (ext4 /dev/sda3): request=1 time=0.2 ms
4096 bytes from . (ext4 /dev/sda3): request=2 time=0.2 ms
4096 bytes from . (ext4 /dev/sda3): request=3 time=0.3 ms
4096 bytes from . (ext4 /dev/sda3): request=4 time=12.7 ms
4096 bytes from . (ext4 /dev/sda3): request=5 time=0.3 ms
^C
--- . (ext4 /dev/sda3) ioping statistics ---
5 requests completed in 4794.0 ms, 364 iops, 1.4 MiB/s
min/avg/max/mdev = 0.2/2.8/12.7/5.0 ms
```

Measure disk seek rate (iops, avg)

```
$ ioping -R /dev/sda

--- /dev/sda (device 465.8 GiB) ioping statistics ---
186 requests completed in 3004.6 ms, 62 iops, 0.2 MiB/s
min/avg/max/mdev = 6.4/16.0/26.8/4.7 ms
```

Measure disk sequential speed (MiB/s)

```
$ ioping -RL /dev/sda

--- /dev/sda (device 465.8 GiB) ioping statistics ---
837 requests completed in 3004.1 ms, 292 iops, 72.9 MiB/s
min/avg/max/mdev = 2.0/3.4/28.9/2.0 ms
```