include ../../../.env

compile:
	/usr/bin/g++ -c -pipe -O3 -fomit-frame-pointer -march=native  -std=c++14 -g pidigits.gpp-4.c++ -o pidigits.gpp-4.c++.o &&  /usr/bin/g++ pidigits.gpp-4.c++.o -o pidigits.gpp-4.gpp_run -lgmp -lgmpxx 

compile_optimized:
	/usr/bin/g++ -c -pipe -O3 -fomit-frame-pointer -march=native  -std=c++14 -g optimized_pidigits.gpp-4.c++ -o optimized_pidigits.gpp-4.c++.o &&  /usr/bin/g++ optimized_pidigits.gpp-4.c++.o -o optimized_pidigits.gpp-4.gpp_run -lgmp -lgmpxx 

measure:
	sudo modprobe msr
	sudo ${USER_PREFIX}/EEDC/RAPL/main "./pidigits.gpp-4.gpp_run 10000" c++ pidigits
	sudo chmod -R 777 ${USER_PREFIX}/EEDC/energy/src/c++.csv

measure_optimized:
	sudo modprobe msr
	sudo ${USER_PREFIX}/EEDC/RAPL/main "./optimized_pidigits.gpp-4.gpp_run 10000" c++ pidigits
	sudo chmod -R 777 ${USER_PREFIX}/EEDC/energy/src/c++.csv
	
run:
	./pidigits.gpp-4.gpp_run 10000

run_optimized:
	./optimized_pidigits.gpp-4.gpp_run 10000

mem:
	/usr/bin/time -v ./pidigits.gpp-4.gpp_run 10000

valgrind:
	valgrind --tool=massif --stacks=yes ./pidigits.gpp-4.gpp_run 10000
