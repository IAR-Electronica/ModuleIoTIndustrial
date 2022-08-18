#!/bin/bash

command_open_idf_dev0='. $HOME/esp/esp-idf/export.sh; /bin/bash' 
command_open_idf_dev1='. $HOME/esp/esp-idf/export.sh; /bin/bash' 
command_open_idf_dev2='. $HOME/esp/esp-idf/export.sh; /bin/bash' 
command_open_idf_dev3='. $HOME/esp/esp-idf/export.sh; /bin/bash' 

xfce4-terminal --title="terminalUSB0" --geometry 70x20+0+0  -x /bin/bash  -c "$command_open_idf_dev0" & 
xfce4-terminal --title="terminalUSB1" --geometry 70x20-0+0 -x /bin/bash   -c "$command_open_idf_dev1" & 
xfce4-terminal --title="terminalUSB2" --geometry 70x20-0-0 -x /bin/bash   -c "$command_open_idf_dev2" & 
xfce4-terminal --title="terminalUSB3" --geometry 70x20+0-0 -x /bin/bash   -c "$command_open_idf_dev3" & 
#capture sigint and close terminals 
exit