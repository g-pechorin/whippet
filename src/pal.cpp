
// can be a single-header
#include <pal.hpp>
#ifndef pal_cpp
#	define pal_cpp
#endif

pal_cpp pal::adler::adler(void) :
	_a(1),
	_b(0)
{
}

pal_cpp pal::adler::adler(const adler& last, const char next) :
	_a((last._a + next) % MOD_ADLER),
	_b((last._b + ((last._a + next) % MOD_ADLER)) % MOD_ADLER)
{
}

pal_cpp pal::adler::operator uint32_t(void) const
{
	return (_b << 16) | _a;
}

pal_cpp pal::adler pal::adler::operator()(const size_t l, const char* t) const
{
	return l ? ((*this) << t)(l - 1, t + 1) : *this;
}

pal_cpp uint32_t pal::adler::operator()(void) const
{
	return *this;
}

pal_cpp pal::adler pal::adler:: operator << (const char* s) const
{
	return (s && *s) ? pal::adler(*this, *s) << (1 + s) : *this;
}

pal_cpp pal::adler pal::adler::operator << (const char c) const
{
	return pal::adler(*this, c);
}

pal_cpp pal::adler::sum::sum(const char* text) :
	pal::strong<uint32_t>::strong(pal::adler() << text)
{
}

pal_cpp pal::adler::sum::sum(const pal::adler& adler) :
	pal::strong<uint32_t>::strong(adler)
{
}


pal__operator_implement(pal_cpp,pal::adler::sum, _weak, const char*, pal::adler::sum(them)._weak);

pal__operator_implement(pal_cpp,pal::adler::sum, _weak, const pal::adler::sum, them._weak);
