#include <stdio.h>
#include <string.h>
#include <time.h>

// this program uses a .bin file containing a version increment,
// which is incremented every time the generator is called again.
// (thanks, SM dudes)
int main()
{
	FILE *f;
	unsigned build = 0;
	char strtime[25];
	time_t tm;

	if( f = fopen("version.bin", "rb") )
		fread( &build, sizeof(unsigned), 1, f );

	build++;

	if( f = fopen("version.bin", "wb") )
		fwrite( &build, sizeof(unsigned), 1, f );

	// get the current time
	time(&tm);

	memcpy( strtime, asctime(localtime(&tm)), 24 );

	// zero the last character
	strtime[24] = '\0';

	if( f = fopen("verinfo.h", "wb") )
	{
		fprintf( f,
			"const unsigned BUILD_VERSION = %lu;\n"
			"const char *const BUILD_DATE = \"%s\";\n",
			build, strtime
		);

		fclose( f );
	}

	printf( "Version: %lu\n", build );

	return 0;
}
