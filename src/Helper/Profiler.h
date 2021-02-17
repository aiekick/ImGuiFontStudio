#pragma once

#include <tracy/Tracy.hpp>
#include <glad/glad.h>
#include <tracy/TracyOpenGL.hpp>

/* code a inserer dans le main pour tracer la memoire dans tracy
#include <Helper/Profiler.h>
void* operator new(std::size_t count)
{
	auto ptr = malloc(count);
	TracyAlloc(ptr, count);
	return ptr;
}
void* operator new[](std::size_t count)
{
	auto ptr = malloc(count);
	TracyAlloc(ptr, count);
	return ptr;
}
void operator delete(void* ptr) noexcept
{
	TracyFree(ptr);
	free(ptr);
}
void operator delete[](void* ptr) noexcept
{
	TracyFree(ptr);
	free(ptr);
}
*/