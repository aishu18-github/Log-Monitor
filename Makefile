all:
	gcc src/main.c src/daemon.c src/log_monitor.c -o logmon -Iinclude -Wall -O2

clean:
	rm -f logmon
