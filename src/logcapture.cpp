/** ==========================================================================
 * 2013 by KjellKod.cc. This is PUBLIC DOMAIN to use at your own risk and comes
 * with no warranties. This code is yours to share, use and modify with no
 * strings attached and no restrictions or obligations.
 *
 * For more information see g3log/LICENSE or refer refer to http://unlicense.org
 * ============================================================================*/

#include "g3log/logcapture.hpp"
#include "g3log/crashhandler.hpp"

// For Windows we need force a thread_local install per thread of three
// signals that must have a signal handler instealled per thread-basis
// It is really a royal pain. Seriously Microsoft? Seriously?
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
#define SIGNAL_HANDLER_VERIFY() g3::installSignalHandlerForThread()
#else
// Does nothing  --- enforces that semicolon must be written
#define SIGNAL_HANDLER_VERIFY() do {} while(0)
#endif



/** logCapture is a simple struct for capturing log/fatal entries. At destruction the
* captured message is forwarded to background worker.
* As a safety precaution: No memory allocated here will be moved into the background
* worker in case of dynamic loaded library reasons instead the arguments are copied
* inside of g3log.cpp::saveMessage*/
LogCapture::~LogCapture() {
   using namespace g3::internal;
   SIGNAL_HANDLER_VERIFY();
   saveMessage(_stream.str().c_str(), _file, _line, _function, _level, _log_stream._labels, _expression, _fatal_signal, _stack_trace.c_str());
}


/// Called from crash handler when a fatal signal has occurred (SIGSEGV etc)
LogCapture::LogCapture(const LEVELS &level, g3::SignalType fatal_signal, const char *dump) : LogCapture("", 0, "", level, "", fatal_signal, dump) {
}

/**
 * @file, line, function are given in g3log.hpp from macros
 * @level INFO/DEBUG/WARNING/FATAL
 * @expression for CHECK calls
 * @fatal_signal for failed CHECK:SIGABRT or fatal signal caught in the signal handler
 */
LogCapture::LogCapture(const char *file, const int line, const char *function, const LEVELS &level,
                       const char *expression, g3::SignalType fatal_signal, const char *dump)
   : _log_stream(_stream, _labels), _file(file), _line(line), _function(function), _level(level), _expression(expression), _fatal_signal(fatal_signal) {

   if (g3::internal::wasFatal(level)) {
      _stack_trace = {"\n*******\tSTACKDUMP *******\n"};
      _stack_trace.append(g3::internal::stackdump(dump));
   }
}



/**
* capturef, used for "printf" like API in CHECKF, LOGF, LOGF_IF
* See also for the attribute formatting ref:  http://www.codemaestro.com/reviews/18
*/
void LogCapture::capturef(const char *printf_like_message, ...) {
   static const int kMaxMessageSize = 2048;
   static const std::string kTruncatedWarningText = "[...truncated...]";
   char finished_message[kMaxMessageSize];
   va_list arglist;
   va_start(arglist, printf_like_message);

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) && !defined(__GNUC__))
   const int nbrcharacters = vsnprintf_s(finished_message, _countof(finished_message), _TRUNCATE, printf_like_message, arglist);
#else
   const int nbrcharacters = vsnprintf(finished_message, sizeof (finished_message), printf_like_message, arglist);
#endif
   va_end(arglist);

   if (nbrcharacters <= 0) {
      stream() << "\n\tERROR LOG MSG NOTIFICATION: Failure to parse successfully the message";
      stream() << '"' << printf_like_message << '"' << std::endl;
   } else if (nbrcharacters > kMaxMessageSize) {
      stream() << finished_message << kTruncatedWarningText;
   } else {
      stream() << finished_message;
   }
}


