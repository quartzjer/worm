all: node_modules worms wormd
	
node_modules:
	npm install

worms: worms.c
	gcc -g -o worms worms.c node_modules/telehash-c/libtelehash.a -I node_modules/telehash-c/include

wormd: wormd.c
	gcc -g -o wormd wormd.c node_modules/telehash-c/libtelehash.a -I node_modules/telehash-c/include
