#include "ConfigReader.h"

int main()
{
	ConfigReader config_reader("test.config");

	printf("show config key-value:\n");
	config_reader.ShowKeyValue();

	printf("\ntest:\n");
	string b = config_reader.GetValueString("b", "default value");
	printf("b = \"%s\"\n", b.c_str());
	
	string c = config_reader.GetValueString("c", "default value");
	printf("c = \"%s\"\n", c.c_str());
	return 0;
}
