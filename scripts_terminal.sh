#!/bin/bash

command_dev0=$1 #
command_dev1=$2 #
command_dev2=$3 #
command_dev3=$4 #

dev_0=' idf.py monitor -p '
dev_0="$dev_0$command_dev0"
# --before default_reset --after hard_reset --chip esp32 write_flash --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 bootloader/bootloader.bin 0x10000 new_prh.bin 0x8000 partition_table/partition-table.bin 
dev_1=' idf.py monitor -p '
dev_1="$dev_1$command_dev1"
dev_2=' idf.py monitor -p '
dev_2="$dev_2$command_dev2"
dev_3=' idf.py monitor -p '
dev_3="$dev_3$command_dev3"

command_open_idf_dev0='. $HOME/esp/esp-idf/export.sh; ' #
command_open_idf_dev1='. $HOME/esp/esp-idf/export.sh; ' 
command_open_idf_dev2='. $HOME/esp/esp-idf/export.sh; ' 
command_open_idf_dev3='. $HOME/esp/esp-idf/export.sh; ' 

command_open_idf_dev0="$command_open_idf_dev0$dev_0"
command_open_idf_dev1="$command_open_idf_dev1$dev_1"
command_open_idf_dev2="$command_open_idf_dev2$dev_2"
command_open_idf_dev3="$command_open_idf_dev3$dev_3"



echo "$command_open_idf_dev0"
echo "$command_open_idf_dev1"
echo "$command_open_idf_dev2"
echo "$command_open_idf_dev3"


xfce4-terminal --title="terminalUSB0" --geometry 70x20+0+0 -x /bin/bash  -c "$command_open_idf_dev0" & 
xfce4-terminal --title="terminalUSB1" --geometry 70x20-0+0 -x /bin/bash   -c "$command_open_idf_dev1" & 
xfce4-terminal --title="terminalUSB2" --geometry 70x20-0-0 -x /bin/bash   -c "$command_open_idf_dev2" & 
xfce4-terminal --title="terminalUSB3" --geometry 70x20+0-0 -x /bin/bash   -c "$command_open_idf_dev3" & 
#capture sigint and close terminals 
