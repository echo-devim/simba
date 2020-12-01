all:
	g++ -O3 -o simba simba.cpp main.cpp libdsm-0.3.2/.libs/libdsm.a -ltasn1 -lbsd
gui:
	g++ -O3 -DENABLE_GUI -o simbagui simba.cpp main.cpp simbagui.cpp libdsm-0.3.2/.libs/libdsm.a -ltasn1 -lbsd `pkg-config gtkmm-3.0 --cflags --libs`
