
#include "server.h"

int main(int argc, char **argv) {
	Server::instance()->run(argc, argv);
	return 0;
}
