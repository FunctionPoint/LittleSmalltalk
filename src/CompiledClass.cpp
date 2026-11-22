#include "CompiledClass.h"

std::string CompiledClass::metaclassName() const
{
	return "Meta" + className;
}
