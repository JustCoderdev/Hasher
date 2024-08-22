/* Logger module for JustCoderdev Core library v3
 * */

#include <core.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


void core_log(LogLevel level, CString module,
		CString file, int line, CString format, ...)
{
	va_list args;
	CString labels[LL_COUNT] = {0};

#if SGR_ENABLE
	CString colors[LL_COUNT] = {0};
#endif

	assert(LL_COUNT == 4);
	labels[LL_INFO]  = "INFO";
	labels[LL_DEBUG] = "DEBUG";
	labels[LL_WARN]  = "WARN";
	labels[LL_ERROR] = "ERROR";

#if SGR_ENABLE
	colors[LL_INFO]  = FG_BLUE;
	colors[LL_DEBUG] = FG_GREEN;
	colors[LL_WARN]  = FG_YELLOW;
	colors[LL_ERROR] = FG_RED;
#endif

	va_start(args, format);

	/* CSI RESET Z BOLD Z FG_WHITE Z "%s" M "%s" */
	/* CSI FG_BR_BLACK M "%s:%d: " */

#if DEBUG_ENABLE
#	if SGR_ENABLE
		fprintf(CORE_LOG_STREAM, CSI "%s" M "%s" CSI RESET M, colors[level], labels[level]);
#	else
		fprintf(CORE_LOG_STREAM, "%s", labels[level]);
#	endif

	fprintf(CORE_LOG_STREAM, ":%s:%d: ", file, line);
#else
	(void)file, (void)line;
	if(level == LL_DEBUG) {
		va_end(args);
		return;
	}

	fprintf(CORE_LOG_STREAM, "[%s] ", labels[level]);
#endif

	if(module != NULL) fprintf(CORE_LOG_STREAM, "(%s) ", module);

	vfprintf(CORE_LOG_STREAM, format, args);
	va_end(args);
}
