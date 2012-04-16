dl_layer: dl_layer.cpp all.h
	g++ dl_layer.cpp all.h -lpthread -o dl_layer

server: APP_server.cpp dl_layer.cpp phy_q_server_multi.cpp phy_setup.cpp all.h
	rm -f server
	rm -f chat_history.txt
	g++ APP_server.cpp dl_layer.cpp all.h phy_q_server_multi.cpp phy_setup.cpp -lrt -lpthread -o server
	touch chat_history.txt

clean:
	rm -f server
	rm -f chat_history.txt
	

