
```
IOPING(1)			 User Commands			     IOPING(1)



NAME
       ioping - simple disk I/O latency monitoring tool

SYNOPSYS
       ioping [-LABCDWRkq] [-c count] [-w deadline] [-p period] [-P period]
	      [-i interval] [-s size] [-S wsize] [-o offset]
	      directory|file|device
       ioping -h | -v

DESCRIPTION
       This tool lets you monitor I/O latency in real time.

OPTIONS
       -c count
	      Stop after count requests.

       -w deadline
	      Stop after deadline time passed.

       -p period
	      Print raw statistics for every period requests.

       -P period
	      Print raw statistics for every period in time.

       -i interval
	      Set time between requests to interval (1s).

       -s size
	      Request size (4k).

       -S size
	      Working set size (1m for directory, whole size for file or
	      device).

       -o offset
	      Starting offset in the file/device (0).

       -k     Keep (do not delete) working file "ioping.tmp". Works for
	      directory target.

       -L     Use sequential operations rather than random. This also sets
	      request size to 256k (as in -s 256k).

       -A     Use asynchronous I/O (syscalls io_submit(2), io_submit(2), etc).

       -C     Use cached I/O (suppress cache invalidation via
	      posix_fadvise(2)).

       -D     Use direct I/O (see O_DIRECT in open(2)).

       -W     Use writes rather than reads. Safe for directory target.
	      *DANGEROUS* for file/device, it will shred your data.  In this
	      case should be repeated tree times (-WWW).

       -R     Disk seek rate test (same as -q -i 0 -w 3 -S 64m).  If disk has
	      huge cache working set (-S) should be increased accordingly.

       -B     Batch mode. Be quiet and print final statistics in raw format.

       -q     Suppress periodical human-readable output.

       -h     Display help message and exit.

       -v     Display version and exit.

   Argument suffixes
       For options that expect time argument (-i, -P and -w), default is
       seconds, unless you specify one of the following suffixes (case-
       insensitive):

       us, usec
	      microseconds (a millionth of a second, 1 / 1 000 000)

       ms, msec
	      milliseconds (a thousandth of a second, 1 / 1 000)

       s, sec seconds

       m, min minutes

       h, hour
	      hours

       For options that expect "size" argument (-s, -S and -o), default is
       bytes, unless you specify one of the following suffixes (case-
       insensitive):

       sector disk sectors (a sector is always 512).

       KiB, k, kb
	      kilobytes (1 024 bytes)

       page   memory pages (a page is always 4KiB).

       MiB, m, mb
	      megabytes (1 048 576 bytes)

       GiB, g, gb
	      gigabytes (1 073 741 824 bytes)

       TiB, t, tb
	      terabytes (1 099 511 627 776 bytes)

       For options that expect "number" argument (-p and -c) you can
       optionally specify one of the following suffixes (case-insensitive):

       k      kilo (thousands, 1 000)

       m      mega (millions, 1 000 000)

       g      giga (billions, 1 000 000 000)

       t      tera (trillions, 1 000 000 000 000)

EXIT STATUS
       Returns 0 upon success. The following error codes are defined:

       1      Invalid usage (error in arguments).

       2      Error during preparation stage.

       3      Error during runtime.

RAW STATISTICS
       ioping -p 100 -c 200 -i 0 -q .
       100 26694 3746 15344272 188 267 1923 228
       100 24165 4138 16950134 190 242 2348 214
       (1) (2)	 (3)  (4)      (5) (6) (7)  (8)

       (1) number of requests
       (2) serving time 	(usec)
       (3) requests per second	(iops)
       (4) transfer speed	(bytes/sec)
       (5) minimal request time (usec)
       (6) average request time (usec)
       (7) maximum request time (usec)
       (8) request time standard deviation (usec)

EXAMPLES
       ioping .
	      Show disk I/O latency using the default values and the current
	      directory, until interrupted.

       ioping -c 10 -s 1M /tmp
	      Measure latency on /tmp using 10 requests of 1 megabyte each.

       ioping -R /dev/sda
	      Measure disk seek rate.

       ioping -RL /dev/sda
	      Measure disk sequential speed.

       ioping -RLB . | awk '{print $4}'
	      Get disk sequential speed in bytes per second.

SEE ALSO
       iostat(1), dd(1), fio(1), dbench(1), fsstress, xfstests, hdparm(8),
       badblocks(8),

HOMEPAGE
       ⟨http://code.google.com/p/ioping/⟩.

AUTHORS
       This program was written by Konstantin Khlebnikov ⟨koct9i@gmail.com⟩.
       Man-page was written by Kir Kolyshkin ⟨kir@openvz.org⟩.



				   Oct 2014			     IOPING(1)
```