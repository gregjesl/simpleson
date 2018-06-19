#include "json.h"
#include <assert.h>

int main(void)
{
	assert(json::remove_leading_spaces("") == "");
	assert(json::remove_leading_spaces(" ") == "");
	assert(json::remove_leading_spaces(" \r") == "");
	assert(json::remove_leading_spaces(" \t") == "");
	assert(json::remove_leading_spaces(" \n") == "");
	assert(json::remove_leading_spaces(" \rA") == "A");
	assert(json::remove_leading_spaces(" \tA") == "A");
	assert(json::remove_leading_spaces(" \nA") == "A");
}