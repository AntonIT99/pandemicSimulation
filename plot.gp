#!/gnuplot

set timefmt '%d/%m/%Y %H:%M:%S'
set xdata time
set format x '%d/%m/%Y %H:%M:%S'

set term windows 0
plot	'pandemic2.dat' using 1:7 title 'Reproduction Rate' with lines linetype rgb "blue",\
		'pandemic2.dat' using 1:8 title 'Lockdown' with lines linetype rgb "cyan"
set term windows 1
set multiplot layout 2,2 columnsfirst
plot    'pandemic2.dat' using 1:3 title 'new Cases' with lines linetype rgb "red",\
		'pandemic2.dat' using 1:4 title 'Recoveries' with lines linetype rgb "green"
plot	'pandemic2.dat' using 1:5 title 'Deaths' with lines linetype rgb "black"
unset multiplot
set term windows 2
plot    'pandemic.dat' using 1:3 title 'Susceptible' with lines linetype rgb "blue",\
		'pandemic.dat' using 1:4 title 'Infectious' with lines linetype rgb "red",\
		'pandemic.dat' using 1:5 title 'Recovered' with lines linetype rgb "green",\
		'pandemic.dat' using 1:6 title 'Deceased' with lines linetype rgb "black",\
		'pandemic.dat' using 1:7 title 'Vaccinated' with lines linetype rgb "magenta",\
		'pandemic2.dat' using 1:6 title 'Lockdown' with lines linetype rgb "cyan",\
		'pandemic.dat' using 1:8 title 'Cumulated Cases' with lines linetype rgb "yellow"