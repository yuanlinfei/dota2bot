CXX=g++
CPPFLAGS=-std=c++17 -Wall
LIBS=-lcurl -lsqlite3 -lfmt

all: Account.o Match.o Player.o db.o http.o report.o main.o 
	$(CXX) $(CPPFLAGS) $^ -o $@ $(LIBS)

main.o: main.cpp
	$(CXX) $(CPPFLAGS) -c $^ 

db.o: db.cpp
	$(CXX) $(CPPFLAGS) -c $^

http.o: http.cpp
	$(CXX) $(CPPFLAGS) -c $^

report.o: report.cpp
	$(CXX) $(CPPFLAGS) -c $^

Account.o: Account.cpp
	$(CXX) $(CPPFLAGS) -c $^

Player.o: Player.cpp
	$(CXX) $(CPPFLAGS) -c $^

Match.o: Match.cpp
	$(CXX) $(CPPFLAGS) -c $^