#!/bin/bash
kpn bmq
./getmsg 20 100000 >getmsg1.log &
./sendmsg 10 20 100000 >sendmsg1.log &


