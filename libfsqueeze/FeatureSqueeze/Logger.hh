#ifndef LOGGER_HH
#define LOGGER_HH

#include <iostream>
#include <string>

namespace fsqueeze
{

class Logger
{
public:
	Logger(std::ostream &outStream, std::ostream &errStream) : d_outStream(outStream), d_errStream(errStream) {}
	std::ostream &error();
	std::ostream &message();
private:
	std::ostream &d_outStream;
	std::ostream &d_errStream;
};

inline std::ostream &Logger::error()
{
	return d_errStream;
}

inline std::ostream &Logger::message()
{
	return d_outStream;
}

}

#endif // LOGGER_HH