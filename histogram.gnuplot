clear
reset

set samples 500

set key off
set border 3
set yzeroaxis
set boxwidth 0.1 absolute

binwidth = 0.1
set boxwidth binwidth
sum = 0

s(x)          = ((sum=sum+1), 0)
bin(x, width) = width*floor(x/width) + binwidth/2.0

set multiplot
set xr [-2:2]

unset ytics
unset xtics
unset border

set size 1.,1./2.
set origin 0./2.,1./2.
plot "cmake-build-release-mpi/samples0.dat" u ($1):(s($1)) lt rgb "#FF0000"
plot exp(-(x*x - 1)*(x*x - 1)) lt rgb "#000000"  lw 2
plot "cmake-build-release-mpi/samples0.dat" u (bin($1, binwidth)):(1.0/(binwidth*sum)) smooth freq w boxes lt rgb "#FF0000" lw 1.5

set size 1.,1./2.
set origin 0./2.,0./2.
plot "cmake-build-release-mpi/samples1.dat" u ($1):(s($1)) lt rgb "#0000FF"
plot exp(-30*(x*x - 1)*(x*x - 1)) lt rgb "#000000"  lw 2
plot "cmake-build-release-mpi/samples1.dat" u (bin($1, binwidth)):(1.0/(binwidth*sum)) smooth freq w boxes lt rgb "#0000FF" lw 1.5
