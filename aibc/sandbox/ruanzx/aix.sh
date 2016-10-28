#!/bin/sh

AWK=${AWK:-awk}

#### for OSInfo ####
echo [OSInfo]

echo SysName=`uname -s`
echo HostName=`hostname`
echo Release=`uname -r`
echo Version=`uname -v`
echo Platform=`uname -p`
echo Machine=`uname -m`
echo Uptime=`uptime | cut -b 16- | cut -d, -f 1,2`
echo IpAddress=`host \`hostname\`| cut -d " " -f 3`

#### for MemInfo ####
echo [MemInfo]

vmstat -v | $AWK '
    /memory pages$/ {r = $1*4/1024;} 

    /free pages$/ {f = $1/256;} 

    END {
        print "Real=" r;
        print "Free=" f;
        print "Active=" r - f;
    }'

SWAP_INFO=`lsps -s | tail -1 | tr -s " " | cut -d " " -f 2,3`
SWAP=`echo $SWAP_INFO | cut -d " " -f 1 | cut -d M -f 1`
PSWAPUSED=`echo $SWAP_INFO | cut -d " " -f 2 | cut -d "%" -f 1`

UsedSwap=`expr $SWAP \* $PSWAPUSED / 100`
FreeSwap=`expr $SWAP - $UsedSwap`

echo UsedSwap=$UsedSwap
echo FreeSwap=$FreeSwap

#### for disk info ####
echo [DiskInfo]

FSTMPFILE1=/tmp/DISKINFO_fs1.txt.$$
FSTMPFILE2=/tmp/DISKINFO_fs2.txt.$$

df -m | tail +2 | egrep -v proc | sort | tr -s " " "," | cut -d "," -f 1,2,3,4,7 > $FSTMPFILE1
mount | egrep -v proc | tail +3 | tr -s " " "," | cut -d "," -f 4 > $FSTMPFILE2
paste -d "," $FSTMPFILE1 $FSTMPFILE2 | $AWK -F, '
    {
        printf("Capacity_%d=%d\n", NR - 1, $2);
        printf("Available_%d=%d\n", NR - 1, $3);
        printf("UsedRate_%d=%f\n", NR - 1, $4/100);
        printf("MntName_%d=%s\n", NR - 1, $1);
        printf("MntPoint_%d=%s\n", NR - 1, $5);
        printf("MntFSType_%d=%s\n", NR - 1, $6);
    }'

/bin/rm -rf $FSTMPFILE1 $FSTMPFILE2


#### for TcpInfo ####
echo [TcpInfo]

netstat -na | $AWK '
    BEGIN {t=0;}

    /^tcp4/ {++v[$6];++t;}

    END {
        print "TotalCount=" t;
        print "TimeWaitCount=" v["TIME_WAIT"];
        print "EstablishedCount=" v["ESTABLISHED"];
    }'

#### for ProcInfo ####
echo [ProcInfo]

ps -e -o thcount,pid,ppid,gid,uid,flag,cputime,spgsz,tpgsz,dpgsz,pcpu,pmem,user,command,args|tail +2|$AWK '
    {   printf("Lwps_%d=%s\n", NR - 1, $1);
        printf("Pid_%d=%s\n", NR - 1, $2);
        printf("Ppid_%d=%s\n", NR - 1, $3);
        printf("Gid_%d=%s\n", NR - 1, $4);
        printf("Uid_%d=%s\n", NR - 1, $5);
        printf("Flag_%d=%s\n", NR - 1, $6);
        printf("CPUTime_%d=%s\n", NR - 1, $7);
        printf("MemSize_%d=%s\n", NR - 1, $8 + $9 + $10);
        printf("CPUUsed_%d=%s\n", NR - 1, $11);
        printf("MemUsed_%d=%s\n", NR - 1, $12);
        printf("UserName_%d=%s\n", NR - 1, $13);
        printf("ProcName_%d=%s\n", NR - 1, $14);
        printf("ProcArgs_%d=", NR - 1); for (x=15;x <= NF; ++x) printf("%s ",$x); print "";
        printf("RunPath_%d=", NR - 1); system("procwdx -F " $2 " | cut -d : -f 2 "); print "";
    }

    END {
        print "[CPUInfo]";
        print "ProcCounts=" NR;
    }'


#### for CPUInfo ####

iostat 1 1 | head -n 5 | $AWK '
    /lcpu=/ {
        split($3,v,"=");
        print "Counts=" v[2];
    } 

    {
        if (NR == 5) {
            printf("User=%f\n", $3);
            printf("Kernel=%f\n", $4);
            printf("Idle=%f\n", $5);
            printf("IOWait=%f\n", $6);
        }
    }'

LOADS=`uptime | cut -d ":" -f 4`

echo Swap=
echo Run1Min=`echo $LOADS | cut -d "," -f 1`
echo Run5Min=`echo $LOADS | cut -d "," -f 2 | cut -b 2-`
echo Run15Min=`echo $LOADS | cut -d "," -f 3 | cut -b 2-`


