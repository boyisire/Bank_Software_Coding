echo "CPU	USER	COMM	PID	VSIZE"
ps -e -T -o "pcpu=" -o "user="  -o "comm=" -o "pid=" -o "vsize=" |grep -v root | sort
