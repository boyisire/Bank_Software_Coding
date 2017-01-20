#!/bin/bash
echo "start...."
kpn bmq
./getmsg 20 100000 >getmsg1.log &
#./getmsg 21 100000 >getmsg2.log &
#./getmsg 22 100000 >getmsg3.log &
#./getmsg 23 100000 >getmsg4.log &
#./getmsg 24 100000 >getmsg5.log &
#./getmsg 25 100000 >getmsg6.log &
#./getmsg 26 100000 >getmsg7.log &
#./getmsg 27 100000 >getmsg8.log &

./sendmsg 10 20 100000 >sendmsg1.log &
#./sendmsg 11 21 100000 >sendmsg2.log &
#./sendmsg 12 22 100000 >sendmsg3.log &
#./sendmsg 13 23 100000 >sendmsg4.log &
#./sendmsg 14 24 100000 >sendmsg5.log &
#./sendmsg 15 25 100000 >sendmsg6.log &
#./sendmsg 16 26 100000 >sendmsg7.log &
#./sendmsg 17 27 100000 >sendmsg8.log &
