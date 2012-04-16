client: client.cpp dl_layer.cpp phy_q.cpp phy_setup.cpp all.h
	rm -f chat_history.txt
	rm -f client
	g++ client.cpp dl_layer.cpp all.h phy_q.cpp phy_setup.cpp -lrt -lpthread -o client

clean:
	rm -f chat_history.txt
	rm -f client

