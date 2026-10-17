#include <cstddef>
void escape_memory(const void *p,std::size_t n){asm volatile(""::"r"(p),"r"(n):"memory");}
