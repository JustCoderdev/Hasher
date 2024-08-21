/* Logger module for JustCoderdev Core library v2
 * */

#include <core.h>
#include <sgr.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>


void core_log(CString level, CString color, CString file, int line,
		CString module, CString format, ...)
{
	va_list args;
	va_start(args, format);

	/* CSI RESET Z BOLD Z FG_WHITE Z "%s" M "%s" */
	/* CSI FG_BR_BLACK M "%s:%d: " */

#if DEBUG_ENABLE
	fprintf(LOG_STREAM, CSI "%s" M "%s" CSI RESET M ":%s:%d: ",
			color, level, file, line);
#else
	(void)color, (void)file, (void)line;
	if(!strncmp(level, "DEBUG", strlen("DEBUG"))) {
		va_end(args);
		return;
	}

	fprintf(LOG_STREAM, "[%s] ", level);
#endif

	if(module != NULL) fprintf(LOG_STREAM, "(%s) ", module);


	vfprintf(LOG_STREAM, format, args);
	va_end(args);
}
