all: backend frontend client

backend: backend_server.cpp httplib.h
	g++ backend_server.cpp -o backend -lpqxx -lpq -pthread

frontend: frontend_server.cpp httplib.h
	g++ frontend_server.cpp -o frontend -pthread

client: client.cpp httplib.h
	g++ client.cpp -o client

clean:
	rm -f backend frontend client
