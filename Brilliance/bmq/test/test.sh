#!/bin/bash
echo "start...."
./getmsg 20 >getmsg1.log &
./getmsg 40 >getmsg2.log &

./sendmsg 10 20  >sendmsg1.log &
./sendmsg 11 40 >sendmsg2.log &

echo "end...."

