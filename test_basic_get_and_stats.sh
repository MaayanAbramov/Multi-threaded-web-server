#!/bin/bash

 PORT=8003
THREADS=5
QUEUE_SIZE=10
 NUM_CLIENTS=100
SERVER_EXEC=./server
 CLIENT_EXEC=./client
 FILENAME=/home.html

# # Compile
# make clean > /dev/null
# make server client > /dev/null
# if [ $? -ne 0 ]; then
#   echo "❌ Compilation failed"
#   exit 1
# fi

# # Create test HTML file
 mkdir -p public
 echo "<html><body><h1>Concurrent Test</h1></body></html>" > public/home.html

# # Start server
$SERVER_EXEC $PORT $THREADS $QUEUE_SIZE &
SERVER_PID=$!
sleep 1

# # Prepare output dir
 mkdir -p outputs
 rm -f outputs/output_*.txt

# Launch clients in parallel
# SERVER_PID  = 7412
echo "🚀 Launching $NUM_CLIENTS GET requests in parallel..."
for i in $(seq 1 $NUM_CLIENTS); do
  $CLIENT_EXEC localhost $PORT $FILENAME GET > outputs/output_$i.txt &
done

# Wait for all clients to finish
wait
kill $SERVER_PID
wait $SERVER_PID 2>/dev/null

# Validate all responses
echo
echo "🔍 Checking client responses..."
FAILED=0
for i in $(seq 1 $NUM_CLIENTS); do
  if ! grep -q "Concurrent Test" outputs/output_$i.txt; then
    echo "❌ Client $i failed to get proper HTML"
    FAILED=1
  fi
done

if [ $FAILED -eq 0 ]; then
  echo "✅ All clients received the correct response"
else
  echo "⚠️ Some responses failed"
fi

# Count threads usage
echo
echo "🔍 Thread usage summary:"
grep Stat-Thread-Id outputs/output_*.txt | cut -d':' -f3 | sort | uniq -c | sort -nr | while read count tid; do
  echo "Thread ID $tid handled $count requests"
done

# Cleanup
rm -rf outputs
