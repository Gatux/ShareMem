#!/bin/bash
> machine_file
if [[ $1 -lt 1 ]]
then
    echo "Error: $1 must be > 0"
    exit
fi

c=0
for line in $(nmap -sn 10.7.1.* | sed -n '1~2p' | cut -d ' ' -f 5)
do
    ssh -oStrictHostKeyChecking=no -oPasswordAuthentication=no $line exit
    if [[ $? == 0 ]]
    then
        echo $line >> machine_file
	c=$c+1
	if [[ $c -ge $1 ]]
	then
	    echo "Nombre de machines : $(wc -l machine_file)"
	    exit
	fi
    fi
done

echo "Nombre de machines : $(wc -l machine_file)"
