./es -t &
export ES_PID=$!
sleep 1
./at 3 hello
sleep 2
./at 7 hello

sleep 50
echo "(B) Sending Ctrl+C to es..."
kill -SIGINT $ES_PID
