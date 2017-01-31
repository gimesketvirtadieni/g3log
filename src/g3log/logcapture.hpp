/** ==========================================================================
 * 2013 by KjellKod.cc. This is PUBLIC DOMAIN to use at your own risk and comes
 * with no warranties. This code is yours to share, use and modify with no
 * strings attached and no restrictions or obligations.
 *
 * For more information see g3log/LICENSE or refer refer to http://unlicense.org
 * ============================================================================*/

#pragma once

#include "g3log/loglevels.hpp"
#include "g3log/g3log.hpp"
#include "g3log/crashhandler.hpp"

#include <string>
#include <sstream>
#include <cstdarg>
#include <csignal>
#include <set>


/**
 * This 'labels' alias can be used for passing labels to log stream
 */
namespace g3 {
	using labels = std::set<std::string>;
}


struct LogStream
{
	LogStream(std::ostream& o, g3::labels& l) : _out(o), _labels(l) {}

	template<typename T>
	inline const LogStream& operator<<(const T& v) const
	{
		_out << v;
		return *this;
	}

	inline const LogStream& operator<<(std::ostream& (*F)(std::ostream&)) const
	{
		F(_out);
		return *this;
	}

	// specializations for 'labels' type
	inline const LogStream& operator<<(const g3::labels& l) const
	{
		_labels.insert(l.begin(), l.end());
		return *this;
	}

	std::ostream& _out;
	g3::labels& _labels;
};


/**
 * Simple struct for capturing log/fatal entries. At destruction the captured message is
 * forwarded to background worker.
 * As a safety precaution: No memory allocated here will be moved into the background
 * worker in case of dynamic loaded library reasons
*/
struct LogCapture {
   /// Called from crash handler when a fatal signal has occurred (SIGSEGV etc)
   LogCapture(const LEVELS &level, g3::SignalType fatal_signal, const char *dump = nullptr);


   /**
    * @file, line, function are given in g3log.hpp from macros
    * @level INFO/DEBUG/WARNING/FATAL
    * @expression for CHECK calls
    * @fatal_signal for failed CHECK:SIGABRT or fatal signal caught in the signal handler
    */
   LogCapture(const char *file, const int line, const char *function, const LEVELS &level, const char *expression = "", g3::SignalType fatal_signal = SIGABRT, const char *dump = nullptr);


   // At destruction the message will be forwarded to the g3log worker.
   // in case of dynamically (at runtime) loaded libraries the important thing to know is that
   // all strings are copied so the original are not destroyed at the receiving end, only the copy
   virtual ~LogCapture();




   // Use "-Wall" to generate warnings in case of illegal printf format.
   //      Ref:  http://www.unixwiz.net/techtips/gnu-c-attributes.html
#ifndef __GNUC__
#define  __attribute__(x) // Disable 'attributes' if compiler does not support 'em
#endif
   void capturef(const char *printf_like_message, ...) __attribute__((format(printf, 2, 3))); // 2,3 ref:  http://www.codemaestro.com/reviews/18


   /// prettifying API for this completely open struct
   LogStream& stream() {
      return _log_stream;
   }



   std::ostringstream _stream;
   LogStream _log_stream;
   std::string _stack_trace;
   g3::labels _labels;
   const char *_file;
   const int _line;
   const char *_function;
   const LEVELS &_level;
   const char *_expression;
   const g3::SignalType _fatal_signal;

};
//} // g3
