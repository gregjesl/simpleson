#include "json.h"
#include <assert.h>
#include <math.h>

int main(void)
{
	assert((int)(json::jnumber::parse(" 123")) == 123);
	assert((int)(json::jnumber::parse(" -123")) == -123);
	double value;
	value = (double)(json::jnumber::parse(" 123.4"));
	assert(fabs(value - 123.4) < 1e-6);
	value = (double)(json::jnumber::parse(" -123.4"));
	assert(fabs(value + 123.4) < 1e-6);

	value = (double)(json::jnumber::parse(" 123.4e1"));
	assert(fabs(value - 1234.0) < 1e-6);
	value = (double)(json::jnumber::parse(" -123.4e1"));
	assert(fabs(value + 1234.0) < 1e-6);

	value = (double)(json::jnumber::parse(" 123.4e+1"));
	assert(fabs(value - 1234.0) < 1e-6);
	value = (double)(json::jnumber::parse(" -123.4e+1"));
	assert(fabs(value + 1234.0) < 1e-6);

	value = (double)(json::jnumber::parse(" 123.4e-1"));
	assert(fabs(value - 12.34) < 1e-6);
	value = (double)(json::jnumber::parse(" -123.4e-1"));
	assert(fabs(value + 12.34) < 1e-6);
}