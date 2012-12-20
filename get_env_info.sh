#!/bin/sh
init_mm=`cat /proc/kallsyms | grep "init_mm$" | awk '{print $1}'`
sys_call_table=`cat /proc/kallsyms | grep " sys_call_table" | awk '{print $1}'`

echo "#define INIT_MM 0x$init_mm"
echo "#define SYS_CALL_TABLE 0x$sys_call_table"
