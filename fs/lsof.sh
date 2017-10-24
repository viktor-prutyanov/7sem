#!/bin/bash

echo "  PID                     COMM        NODE NAME/TYPE"

for pid in `ls /proc/ | egrep -o "[0-9]+" | sort -k1n`; do
    if [ -d "/proc/"$pid ]; then
        cd "/proc/"$pid
        cd "fd" 2>/dev/null 
        if [ $? -ne 0 ]; then
            cd "/proc"
            continue
        fi
        
        for fd in `ls`; do
            desc=$(readlink $fd)
            node="-"
            
            printf "%5s " $pid
            printf "%24s " $(cat "/proc/"$pid"/comm" | tr '\n' ' ')
            if [[ $desc =~ .*\:\[[0-9]+\]$ ]]; then
                node=`echo "$desc" | egrep -o '[0-9]+'`
                desc=`echo "$desc" | egrep -o '[a-z]+'`
            elif [[ $desc =~ ^anon\_inode\:.*$ ]]; then
                node="a_inode"
                desc=`echo "$desc" | awk -F":" '{ print $2 }'`
            elif [[ $desc =~ .*\:\[.*\]$ ]]; then
                :
            else
                node=`stat -c '%i' $desc 2>/dev/null`
                if [ $? -ne 0 ]; then
                    node="-"
                fi
            fi
            printf " %10s %s\n" $node $desc
        done
        
        cd ..
    fi
done
