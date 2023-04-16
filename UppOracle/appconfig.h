// pad strings with extra zero for some reason related to .rc files
#ifdef RC_STRING
#error RC_STRING already defined
#endif
#define RC_STRING(x) x"\0"

// important defines
#define UPP_ORACLE_EXE_NAME RC_STRING("UppOracle.exe")

#define APP_VERSION_STRING  RC_STRING("V0.4.0.0")
#define APP_VERSION_ARRAY               0,4,0,0

#ifdef _DEBUG
#define APP_FILEFLAGS   (VS_FF_PRIVATEBUILD|VS_FF_PRERELEASE|VS_FF_DEBUG)
#else
#define APP_FILEFLAGS   (VS_FF_PRIVATEBUILD|VS_FF_PRERELEASE)
#endif
