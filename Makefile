
# 将 .o 文件集中到 ./obj
$(shell mkdir -p ./obj)

objFile = main.o client.o manager.o
obj = ./obj
bin = mem

all: $(objFile)
	@cd $(obj); $(CXX) -Wall -o ../$(bin) $(objFile)
	@echo "make succ"

main.o: 
	$(CXX) -c  main.cpp -o $(obj)/main.o

client.o:
	$(CXX) -c MemClient.cpp -o $(obj)/client.o

manager.o: 
	$(CXX) -c MemManager.cpp -o $(obj)/manager.o

.PHONY : clean

clean:
	-rm -rf $(bin) $(obj)


