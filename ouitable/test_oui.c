#include <stdio.h>

#include "ouitable.h"

int main()
{
	int i;

	printf("Testing OUI table...\n");
	printf("Table size: %lu entries\n", oui_table_size);

	for (i = 0; i < oui_table_size; i++) {
		printf("Prefix: %06X, Vendor: %s\n", oui_table[i].prefix, oui_table[i].vendor);
	}
	return 0;
}
