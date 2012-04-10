dl_layer: dl_layer.cpp all.h
	g++ dl_layer.cpp all.h -lpthread -o dl_layer

dl_total: APP_server.cpp dl_layer.cpp phy_q_server_multi.cpp phy_setup.cpp all.h
	g++ APP_server.cpp dl_layer.cpp all.h phy_q_server_multi.cpp phy_setup.cpp -rt -lpthread -o dl_total

