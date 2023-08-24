

FLAMEGRAPH=/home/akkamath/FlameGraph
REDIS=/home/akkamath/redis
#/home/akkamath/zIO/benchmarks/redis

record_perf () {
    perf record -F 200 -g ./redis-server ../redis.conf 
    perf script > out.perf
    ${FLAMEGRAPH}/stackcollapse-perf.pl out.perf > out.folded
    ${FLAMEGRAPH}/flamegraph.pl out.folded > redis_${1}.svg
}

REDIS_SIZES="16384 32768 65536"

cd ${REDIS}/src;

for size in ${REDIS_SIZES}; do
    rm -rf ../pmem/*
    record_perf ${size} &
    sleep 5
    echo "Redis running"
    ./redis-benchmark -p 6379 -d ${size} -t set -c 64 -n 51200
    sleep 5
    pkill redis-server
    sleep 5
done
