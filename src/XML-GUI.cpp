#include <XML-Server.h>

int main() {
	xmlPrs::XMLServer server;

	server.run(64000, "XML_GUI_log");

	return EXIT_SUCCESS;
}