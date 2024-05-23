set terminal pdf
set output "packetcomp.pdf"
set title "Packet Loss Comparison"
set xlabel "Node"
set ylabel "Packet Loss"
set key top left
set style data histogram
set style histogram clustered gap 1
set style fill solid 0.5 border -1
set boxwidth 0.9

plot "packetcomparison.txt" using 2:xtic(1) title "During Attack", \
     '' using 3 title "After Mitigation"

