#include "App.h"

static UppOracle* app = nullptr;

UppOracle& App()
{
	return *app;
}

GUI_APP_MAIN
{
	SetAppName("UppOracle");

#ifndef flagPORTABLE
	UseHomeDirectoryConfig();
#endif

    StdLogSetup(LOG_FILE);
    DUMP(GetConfigFolder());
	
#if defined(flagDEBUG)
	//HttpRequest::Trace();
#endif
	
	UppOracle oracle;
	
	app = &oracle;
	app->Execute();
	app = nullptr;
}
