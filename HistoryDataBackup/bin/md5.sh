read -s abc
#����
psw=`echo $abc | openssl aes-128-cbc -k 123 -base64`
echo $psw
#����
#echo U2FsdGVkX19pp2N4NL40pw1mIuQe2tcPCidlofUAxTI= | openssl aes-128-cbc -d -k 123 -base64
