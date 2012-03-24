all:
	g++ -lpthread server.cpp all.h -o server
	g++ client.cpp all.h -o client
phy_q: phy_q.cpp phy_setup.cpp all.h
	g++ phy_q.cpp phy_setup.cpp all.h -lpthread -o phy_q
phy_q_server: phy_q_server.cpp
	g++ phy_q_server.cpp phy_setup.cpp all.h -lpthread -o phy_q_server

threads:
	g++ -lpthread threads_example.cpp -o threads
clean:
	rm client server phy_q threads phy_q_server
