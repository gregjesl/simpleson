#include "json.h"
#include <assert.h>

int main(void)
{
	json::jarray jresult = json::jarray::parse(" [ 123 , 456 , 789 ]");
	std::vector<int> result = jresult;
	assert(result[0] == 123);
	assert(result[1] == 456);
	assert(result[2] == 789);

	result = json::jarray::parse((std::string)jresult);
	assert(result[0] == 123);
	assert(result[1] == 456);
	assert(result[2] == 789);
}