1. 使用unordered flat map作为存储

- data bytes=1024
```bash
redis-benchmark -h 127.0.0.1 -p 26379 -n 1000000 -c 50 -P 100 -d 1024 -t set,get
WARNING: Could not fetch server CONFIG
====== SET ======
  1000000 requests completed in 0.39 seconds
  50 parallel clients
  1024 bytes payload
  keep alive: 1
  multi-thread: no

Latency by percentile distribution:
0.000% <= 0.167 milliseconds (cumulative count 100)
50.000% <= 0.799 milliseconds (cumulative count 506900)
75.000% <= 1.047 milliseconds (cumulative count 750500)
87.500% <= 1.215 milliseconds (cumulative count 877900)
93.750% <= 1.359 milliseconds (cumulative count 937500)
96.875% <= 1.487 milliseconds (cumulative count 969800)
98.438% <= 1.599 milliseconds (cumulative count 985000)
99.219% <= 1.823 milliseconds (cumulative count 992200)
99.609% <= 2.503 milliseconds (cumulative count 996100)
99.805% <= 3.871 milliseconds (cumulative count 998100)
99.902% <= 4.159 milliseconds (cumulative count 999100)
99.951% <= 4.463 milliseconds (cumulative count 999600)
99.976% <= 4.607 milliseconds (cumulative count 999800)
99.988% <= 4.631 milliseconds (cumulative count 999900)
99.994% <= 4.735 milliseconds (cumulative count 1000000)
100.000% <= 4.735 milliseconds (cumulative count 1000000)

Cumulative distribution of latencies:
0.000% <= 0.103 milliseconds (cumulative count 0)
0.020% <= 0.207 milliseconds (cumulative count 200)
1.310% <= 0.303 milliseconds (cumulative count 13100)
7.530% <= 0.407 milliseconds (cumulative count 75300)
16.470% <= 0.503 milliseconds (cumulative count 164700)
30.000% <= 0.607 milliseconds (cumulative count 300000)
40.650% <= 0.703 milliseconds (cumulative count 406500)
51.510% <= 0.807 milliseconds (cumulative count 515100)
61.080% <= 0.903 milliseconds (cumulative count 610800)
71.180% <= 1.007 milliseconds (cumulative count 711800)
80.240% <= 1.103 milliseconds (cumulative count 802400)
87.310% <= 1.207 milliseconds (cumulative count 873100)
91.680% <= 1.303 milliseconds (cumulative count 916800)
94.980% <= 1.407 milliseconds (cumulative count 949800)
97.230% <= 1.503 milliseconds (cumulative count 972300)
98.600% <= 1.607 milliseconds (cumulative count 986000)
99.040% <= 1.703 milliseconds (cumulative count 990400)
99.210% <= 1.807 milliseconds (cumulative count 992100)
99.270% <= 1.903 milliseconds (cumulative count 992700)
99.350% <= 2.007 milliseconds (cumulative count 993500)
99.440% <= 2.103 milliseconds (cumulative count 994400)
99.730% <= 3.103 milliseconds (cumulative count 997300)
99.900% <= 4.103 milliseconds (cumulative count 999000)
100.000% <= 5.103 milliseconds (cumulative count 1000000)

Summary:
  throughput summary: 2557544.75 requests per second
  latency summary (msec):
          avg       min       p50       p95       p99       max
        0.835     0.160     0.799     1.415     1.695     4.735
====== GET ======
  1000000 requests completed in 0.51 seconds
  50 parallel clients
  1024 bytes payload
  keep alive: 1
  multi-thread: no

Latency by percentile distribution:
0.000% <= 0.367 milliseconds (cumulative count 100)
50.000% <= 0.807 milliseconds (cumulative count 513000)
75.000% <= 0.983 milliseconds (cumulative count 754300)
87.500% <= 1.175 milliseconds (cumulative count 875900)
93.750% <= 1.383 milliseconds (cumulative count 938800)
96.875% <= 1.575 milliseconds (cumulative count 969300)
98.438% <= 1.743 milliseconds (cumulative count 984600)
99.219% <= 1.927 milliseconds (cumulative count 992200)
99.609% <= 2.303 milliseconds (cumulative count 996100)
99.805% <= 4.103 milliseconds (cumulative count 998100)
99.902% <= 4.527 milliseconds (cumulative count 999100)
99.951% <= 5.495 milliseconds (cumulative count 999600)
99.976% <= 5.567 milliseconds (cumulative count 999900)
99.994% <= 5.607 milliseconds (cumulative count 1000000)
100.000% <= 5.607 milliseconds (cumulative count 1000000)

Cumulative distribution of latencies:
0.000% <= 0.103 milliseconds (cumulative count 0)
0.050% <= 0.407 milliseconds (cumulative count 500)
3.280% <= 0.503 milliseconds (cumulative count 32800)
15.460% <= 0.607 milliseconds (cumulative count 154600)
33.930% <= 0.703 milliseconds (cumulative count 339300)
51.300% <= 0.807 milliseconds (cumulative count 513000)
65.830% <= 0.903 milliseconds (cumulative count 658300)
77.430% <= 1.007 milliseconds (cumulative count 774300)
84.140% <= 1.103 milliseconds (cumulative count 841400)
88.670% <= 1.207 milliseconds (cumulative count 886700)
92.170% <= 1.303 milliseconds (cumulative count 921700)
94.300% <= 1.407 milliseconds (cumulative count 943000)
95.990% <= 1.503 milliseconds (cumulative count 959900)
97.340% <= 1.607 milliseconds (cumulative count 973400)
98.190% <= 1.703 milliseconds (cumulative count 981900)
98.740% <= 1.807 milliseconds (cumulative count 987400)
99.160% <= 1.903 milliseconds (cumulative count 991600)
99.380% <= 2.007 milliseconds (cumulative count 993800)
99.550% <= 2.103 milliseconds (cumulative count 995500)
99.670% <= 3.103 milliseconds (cumulative count 996700)
99.810% <= 4.103 milliseconds (cumulative count 998100)
99.930% <= 5.103 milliseconds (cumulative count 999300)
100.000% <= 6.103 milliseconds (cumulative count 1000000)

Summary:
  throughput summary: 1968503.88 requests per second
  latency summary (msec):
          avg       min       p50       p95       p99       max
        0.868     0.360     0.807     1.447     1.847     5.607
```

- 小数据
```bash
redis-benchmark -h 127.0.0.1 -p 26379 -n 1000000 -c 50 -P 100 -t set,get
WARNING: Could not fetch server CONFIG
====== SET ======
  1000000 requests completed in 0.23 seconds
  50 parallel clients
  3 bytes payload
  keep alive: 1
  multi-thread: no

Latency by percentile distribution:
0.000% <= 0.303 milliseconds (cumulative count 100)
50.000% <= 0.935 milliseconds (cumulative count 503300)
75.000% <= 1.135 milliseconds (cumulative count 755300)
87.500% <= 1.391 milliseconds (cumulative count 875900)
93.750% <= 1.591 milliseconds (cumulative count 937800)
96.875% <= 1.775 milliseconds (cumulative count 969100)
98.438% <= 2.103 milliseconds (cumulative count 984500)
99.219% <= 2.423 milliseconds (cumulative count 992200)
99.609% <= 2.943 milliseconds (cumulative count 996100)
99.805% <= 3.263 milliseconds (cumulative count 998100)
99.902% <= 4.335 milliseconds (cumulative count 999100)
99.951% <= 4.383 milliseconds (cumulative count 999900)
99.994% <= 4.423 milliseconds (cumulative count 1000000)
100.000% <= 4.423 milliseconds (cumulative count 1000000)

Cumulative distribution of latencies:
0.000% <= 0.103 milliseconds (cumulative count 0)
0.010% <= 0.303 milliseconds (cumulative count 100)
1.380% <= 0.407 milliseconds (cumulative count 13800)
6.630% <= 0.503 milliseconds (cumulative count 66300)
12.890% <= 0.607 milliseconds (cumulative count 128900)
21.980% <= 0.703 milliseconds (cumulative count 219800)
34.180% <= 0.807 milliseconds (cumulative count 341800)
46.450% <= 0.903 milliseconds (cumulative count 464500)
59.220% <= 1.007 milliseconds (cumulative count 592200)
72.270% <= 1.103 milliseconds (cumulative count 722700)
80.630% <= 1.207 milliseconds (cumulative count 806300)
84.400% <= 1.303 milliseconds (cumulative count 844000)
88.070% <= 1.407 milliseconds (cumulative count 880700)
91.260% <= 1.503 milliseconds (cumulative count 912600)
94.180% <= 1.607 milliseconds (cumulative count 941800)
95.840% <= 1.703 milliseconds (cumulative count 958400)
97.140% <= 1.807 milliseconds (cumulative count 971400)
97.630% <= 1.903 milliseconds (cumulative count 976300)
98.190% <= 2.007 milliseconds (cumulative count 981900)
98.450% <= 2.103 milliseconds (cumulative count 984500)
99.720% <= 3.103 milliseconds (cumulative count 997200)
99.890% <= 4.103 milliseconds (cumulative count 998900)
100.000% <= 5.103 milliseconds (cumulative count 1000000)

Summary:
  throughput summary: 4255319.00 requests per second
  latency summary (msec):
          avg       min       p50       p95       p99       max
        0.984     0.296     0.935     1.647     2.295     4.423
====== GET ======
  1000000 requests completed in 0.23 seconds
  50 parallel clients
  3 bytes payload
  keep alive: 1
  multi-thread: no

Latency by percentile distribution:
0.000% <= 0.223 milliseconds (cumulative count 100)
50.000% <= 0.943 milliseconds (cumulative count 511900)
75.000% <= 1.207 milliseconds (cumulative count 750500)
87.500% <= 1.519 milliseconds (cumulative count 877000)
93.750% <= 1.679 milliseconds (cumulative count 938000)
96.875% <= 1.799 milliseconds (cumulative count 969500)
98.438% <= 1.895 milliseconds (cumulative count 985000)
99.219% <= 2.023 milliseconds (cumulative count 992800)
99.609% <= 2.167 milliseconds (cumulative count 996100)
99.805% <= 2.607 milliseconds (cumulative count 998100)
99.902% <= 2.791 milliseconds (cumulative count 999100)
99.951% <= 3.295 milliseconds (cumulative count 999600)
99.976% <= 3.487 milliseconds (cumulative count 999800)
99.988% <= 3.495 milliseconds (cumulative count 999900)
99.994% <= 3.727 milliseconds (cumulative count 1000000)
100.000% <= 3.727 milliseconds (cumulative count 1000000)

Cumulative distribution of latencies:
0.000% <= 0.103 milliseconds (cumulative count 0)
0.070% <= 0.303 milliseconds (cumulative count 700)
1.130% <= 0.407 milliseconds (cumulative count 11300)
3.620% <= 0.503 milliseconds (cumulative count 36200)
8.020% <= 0.607 milliseconds (cumulative count 80200)
16.570% <= 0.703 milliseconds (cumulative count 165700)
30.540% <= 0.807 milliseconds (cumulative count 305400)
44.830% <= 0.903 milliseconds (cumulative count 448300)
59.790% <= 1.007 milliseconds (cumulative count 597900)
69.310% <= 1.103 milliseconds (cumulative count 693100)
75.050% <= 1.207 milliseconds (cumulative count 750500)
79.540% <= 1.303 milliseconds (cumulative count 795400)
83.500% <= 1.407 milliseconds (cumulative count 835000)
87.170% <= 1.503 milliseconds (cumulative count 871700)
91.110% <= 1.607 milliseconds (cumulative count 911100)
94.450% <= 1.703 milliseconds (cumulative count 944500)
97.160% <= 1.807 milliseconds (cumulative count 971600)
98.620% <= 1.903 milliseconds (cumulative count 986200)
99.180% <= 2.007 milliseconds (cumulative count 991800)
99.500% <= 2.103 milliseconds (cumulative count 995000)
99.940% <= 3.103 milliseconds (cumulative count 999400)
100.000% <= 4.103 milliseconds (cumulative count 1000000)

Summary:
  throughput summary: 4424779.00 requests per second
  latency summary (msec):
          avg       min       p50       p95       p99       max
        1.019     0.216     0.943     1.727     1.967     3.727
```


redis本体 
```bash
redis-benchmark -h 127.0.0.1 -p 6379 -n 1000000 -c 50 -P 100 -t set,get -d 1024
====== SET ======
  1000000 requests completed in 0.89 seconds
  50 parallel clients
  1024 bytes payload
  keep alive: 1
  host configuration "save": 3600 1 300 100 60 10000
  host configuration "appendonly": no
  multi-thread: no

Latency by percentile distribution:
0.000% <= 0.551 milliseconds (cumulative count 100)
50.000% <= 0.927 milliseconds (cumulative count 504900)
75.000% <= 1.031 milliseconds (cumulative count 751100)
87.500% <= 1.095 milliseconds (cumulative count 880700)
93.750% <= 1.135 milliseconds (cumulative count 939100)
96.875% <= 1.175 milliseconds (cumulative count 974500)
98.438% <= 1.199 milliseconds (cumulative count 986000)
99.219% <= 1.415 milliseconds (cumulative count 992500)
99.609% <= 21.647 milliseconds (cumulative count 996100)
99.805% <= 22.591 milliseconds (cumulative count 998100)
99.902% <= 23.071 milliseconds (cumulative count 999100)
99.951% <= 23.311 milliseconds (cumulative count 999600)
99.976% <= 23.407 milliseconds (cumulative count 999800)
99.988% <= 23.455 milliseconds (cumulative count 999900)
99.994% <= 23.535 milliseconds (cumulative count 1000000)
100.000% <= 23.535 milliseconds (cumulative count 1000000)

Cumulative distribution of latencies:
0.000% <= 0.103 milliseconds (cumulative count 0)
0.240% <= 0.607 milliseconds (cumulative count 2400)
14.670% <= 0.703 milliseconds (cumulative count 146700)
29.370% <= 0.807 milliseconds (cumulative count 293700)
44.890% <= 0.903 milliseconds (cumulative count 448900)
69.550% <= 1.007 milliseconds (cumulative count 695500)
89.330% <= 1.103 milliseconds (cumulative count 893300)
98.730% <= 1.207 milliseconds (cumulative count 987300)
99.030% <= 1.303 milliseconds (cumulative count 990300)
99.200% <= 1.407 milliseconds (cumulative count 992000)
99.330% <= 1.503 milliseconds (cumulative count 993300)
99.380% <= 1.607 milliseconds (cumulative count 993800)
99.390% <= 1.703 milliseconds (cumulative count 993900)
99.450% <= 3.103 milliseconds (cumulative count 994500)
99.500% <= 4.103 milliseconds (cumulative count 995000)
99.700% <= 22.111 milliseconds (cumulative count 997000)
99.910% <= 23.103 milliseconds (cumulative count 999100)
100.000% <= 24.111 milliseconds (cumulative count 1000000)

Summary:
  throughput summary: 1121076.25 requests per second
  latency summary (msec):
          avg       min       p50       p95       p99       max
        1.017     0.544     0.927     1.151     1.279    23.535
====== GET ======
  1000000 requests completed in 0.54 seconds
  50 parallel clients
  1024 bytes payload
  keep alive: 1
  host configuration "save": 3600 1 300 100 60 10000
  host configuration "appendonly": no
  multi-thread: no

Latency by percentile distribution:
0.000% <= 0.519 milliseconds (cumulative count 100)
50.000% <= 1.847 milliseconds (cumulative count 505500)
75.000% <= 2.231 milliseconds (cumulative count 764300)
87.500% <= 2.295 milliseconds (cumulative count 883200)
93.750% <= 2.335 milliseconds (cumulative count 939500)
96.875% <= 2.383 milliseconds (cumulative count 970900)
98.438% <= 2.487 milliseconds (cumulative count 985000)
99.219% <= 2.591 milliseconds (cumulative count 992300)
99.609% <= 3.183 milliseconds (cumulative count 996100)
99.805% <= 3.991 milliseconds (cumulative count 998100)
99.902% <= 4.551 milliseconds (cumulative count 999100)
99.951% <= 4.759 milliseconds (cumulative count 999600)
99.976% <= 4.839 milliseconds (cumulative count 999800)
99.988% <= 4.879 milliseconds (cumulative count 999900)
99.994% <= 4.911 milliseconds (cumulative count 1000000)
100.000% <= 4.911 milliseconds (cumulative count 1000000)

Cumulative distribution of latencies:
0.000% <= 0.103 milliseconds (cumulative count 0)
0.100% <= 0.607 milliseconds (cumulative count 1000)
0.120% <= 0.703 milliseconds (cumulative count 1200)
0.130% <= 0.903 milliseconds (cumulative count 1300)
0.160% <= 1.007 milliseconds (cumulative count 1600)
0.380% <= 1.103 milliseconds (cumulative count 3800)
0.650% <= 1.207 milliseconds (cumulative count 6500)
3.560% <= 1.303 milliseconds (cumulative count 35600)
13.870% <= 1.407 milliseconds (cumulative count 138700)
22.570% <= 1.503 milliseconds (cumulative count 225700)
28.420% <= 1.607 milliseconds (cumulative count 284200)
37.590% <= 1.703 milliseconds (cumulative count 375900)
47.440% <= 1.807 milliseconds (cumulative count 474400)
54.970% <= 1.903 milliseconds (cumulative count 549700)
61.540% <= 2.007 milliseconds (cumulative count 615400)
67.460% <= 2.103 milliseconds (cumulative count 674600)
99.590% <= 3.103 milliseconds (cumulative count 995900)
99.810% <= 4.103 milliseconds (cumulative count 998100)
100.000% <= 5.103 milliseconds (cumulative count 1000000)

Summary:
  throughput summary: 1851851.75 requests per second
  latency summary (msec):
          avg       min       p50       p95       p99       max
        1.863     0.512     1.847     2.351     2.551     4.911
```
```bash
redis-benchmark -h 127.0.0.1 -p 6379 -n 1000000 -c 50 -P 100 -t set,get
====== SET ======
  1000000 requests completed in 0.39 seconds
  50 parallel clients
  3 bytes payload
  keep alive: 1
  host configuration "save": 3600 1 300 100 60 10000
  host configuration "appendonly": no
  multi-thread: no

Latency by percentile distribution:
0.000% <= 0.479 milliseconds (cumulative count 100)
50.000% <= 1.759 milliseconds (cumulative count 505600)
75.000% <= 1.959 milliseconds (cumulative count 755800)
87.500% <= 2.103 milliseconds (cumulative count 881900)
93.750% <= 2.287 milliseconds (cumulative count 938000)
96.875% <= 2.495 milliseconds (cumulative count 968800)
98.438% <= 2.655 milliseconds (cumulative count 985000)
99.219% <= 2.839 milliseconds (cumulative count 992300)
99.609% <= 4.383 milliseconds (cumulative count 996100)
99.805% <= 4.783 milliseconds (cumulative count 998100)
99.902% <= 4.935 milliseconds (cumulative count 999100)
99.951% <= 5.031 milliseconds (cumulative count 999600)
99.976% <= 5.079 milliseconds (cumulative count 999800)
99.988% <= 5.111 milliseconds (cumulative count 999900)
99.994% <= 5.159 milliseconds (cumulative count 1000000)
100.000% <= 5.159 milliseconds (cumulative count 1000000)

Cumulative distribution of latencies:
0.000% <= 0.103 milliseconds (cumulative count 0)
0.030% <= 0.503 milliseconds (cumulative count 300)
0.130% <= 0.607 milliseconds (cumulative count 1300)
0.240% <= 0.703 milliseconds (cumulative count 2400)
0.290% <= 0.807 milliseconds (cumulative count 2900)
0.310% <= 0.903 milliseconds (cumulative count 3100)
0.420% <= 1.007 milliseconds (cumulative count 4200)
0.650% <= 1.103 milliseconds (cumulative count 6500)
5.120% <= 1.207 milliseconds (cumulative count 51200)
11.190% <= 1.303 milliseconds (cumulative count 111900)
12.460% <= 1.407 milliseconds (cumulative count 124600)
14.860% <= 1.503 milliseconds (cumulative count 148600)
28.210% <= 1.607 milliseconds (cumulative count 282100)
41.840% <= 1.703 milliseconds (cumulative count 418400)
58.050% <= 1.807 milliseconds (cumulative count 580500)
70.610% <= 1.903 milliseconds (cumulative count 706100)
79.730% <= 2.007 milliseconds (cumulative count 797300)
88.190% <= 2.103 milliseconds (cumulative count 881900)
99.490% <= 3.103 milliseconds (cumulative count 994900)
99.500% <= 4.103 milliseconds (cumulative count 995000)
99.980% <= 5.103 milliseconds (cumulative count 999800)
100.000% <= 6.103 milliseconds (cumulative count 1000000)

Summary:
  throughput summary: 2577319.50 requests per second
  latency summary (msec):
          avg       min       p50       p95       p99       max
        1.779     0.472     1.759     2.367     2.751     5.159
====== GET ======
  1000000 requests completed in 0.33 seconds
  50 parallel clients
  3 bytes payload
  keep alive: 1
  host configuration "save": 3600 1 300 100 60 10000
  host configuration "appendonly": no
  multi-thread: no

Latency by percentile distribution:
0.000% <= 0.415 milliseconds (cumulative count 100)
50.000% <= 1.487 milliseconds (cumulative count 504100)
75.000% <= 1.695 milliseconds (cumulative count 755700)
87.500% <= 1.831 milliseconds (cumulative count 876500)
93.750% <= 1.999 milliseconds (cumulative count 938500)
96.875% <= 2.135 milliseconds (cumulative count 969900)
98.438% <= 2.231 milliseconds (cumulative count 985900)
99.219% <= 2.279 milliseconds (cumulative count 992900)
99.609% <= 2.303 milliseconds (cumulative count 996300)
99.805% <= 2.335 milliseconds (cumulative count 998200)
99.902% <= 2.375 milliseconds (cumulative count 999200)
99.951% <= 2.391 milliseconds (cumulative count 999600)
99.976% <= 2.415 milliseconds (cumulative count 999800)
99.988% <= 2.431 milliseconds (cumulative count 999900)
99.994% <= 2.543 milliseconds (cumulative count 1000000)
100.000% <= 2.543 milliseconds (cumulative count 1000000)

Cumulative distribution of latencies:
0.000% <= 0.103 milliseconds (cumulative count 0)
0.060% <= 0.503 milliseconds (cumulative count 600)
0.090% <= 0.607 milliseconds (cumulative count 900)
0.140% <= 0.703 milliseconds (cumulative count 1400)
0.190% <= 0.807 milliseconds (cumulative count 1900)
0.280% <= 0.903 milliseconds (cumulative count 2800)
3.790% <= 1.007 milliseconds (cumulative count 37900)
11.800% <= 1.103 milliseconds (cumulative count 118000)
14.480% <= 1.207 milliseconds (cumulative count 144800)
21.000% <= 1.303 milliseconds (cumulative count 210000)
38.060% <= 1.407 milliseconds (cumulative count 380600)
52.840% <= 1.503 milliseconds (cumulative count 528400)
66.100% <= 1.607 milliseconds (cumulative count 661000)
76.500% <= 1.703 milliseconds (cumulative count 765000)
86.460% <= 1.807 milliseconds (cumulative count 864600)
90.740% <= 1.903 milliseconds (cumulative count 907400)
94.040% <= 2.007 milliseconds (cumulative count 940400)
96.280% <= 2.103 milliseconds (cumulative count 962800)
100.000% <= 3.103 milliseconds (cumulative count 1000000)

Summary:
  throughput summary: 3048780.50 requests per second
  latency summary (msec):
          avg       min       p50       p95       p99       max
        1.506     0.408     1.487     2.055     2.263     2.543
```
