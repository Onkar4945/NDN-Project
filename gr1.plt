set terminal pdf
set output "throughput.pdf"
set title "Throughput vs. Node Number"
set xlabel "Node Number"
set ylabel "Throughput (bps)"
plot "throughput.txt" using 1:2 with linespoints title "Throughput During Attack", \
     "throughput.txt" using 1:3 with linespoints title "Throughput After Mitigation"

