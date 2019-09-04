/**
 * Code developed and maintained by Sharded-Games.
 * Licensed under the GPLv3
 *
 * @author Dave Cotterill
 *
 * (c)2019 ShardedGames.com, https://github.com/sharded-gaming/regex-renamer
 *
 * Please keep this notice here AS IS going forward.
 * ShardedGames and the author provide no warranty and liability for the code provided AS IS.
 */

#include <windows.h>
#include <stdio.h> 
#include <conio.h>
#include <filesystem>

#include "re2/re2.h"

#define VERSION "0.0.1"

/**
 * Command Line Parameters
 */
typedef enum __tagFlags {
	FLAG_SUBFOLDERS = 0,
	FLAG_PREVIEW,
	FLAG_QUIET,
	FLAG_HELP,

	FLAG_MAX
} flags;

static bool mFlags[] =  {
	false,
	false,
	false,
	false
};

/** 
 * Data
 */
static char* mSrcRegex = NULL;
static char* mDestRegex = NULL;

/**
 * Shows the help page
 */
void showHelp()
{
	printf("RegexRenamer " VERSION "\r\n");
	printf("  RegexRenamer.exe [options] srcRegex destRegex\r\n");
	printf("\r\n");
	printf("  Options: \r\n");
	printf("    -s Follow Subdirectories\r\n");
	printf("    -p Preview mode\r\n");
	printf("    -q Quiet mode, processes immediately\r\n");
	printf("    -h Help, This page\r\n");
	printf("\r\n");
	printf("  Examples:\r\n");
}

/**
 * Shows an error in the console
 * @param errMsg Error Message to display
 */
void showError(const char* errMsg)
{
	printf(errMsg);
	printf("\r\n\r\n");
}

/**
 * Parse the command line
 * @param argc
 * @param argv
 */
bool parseCommandline(int argc, char*argv[])
{
bool ret = false;

	for (int i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			if (_stricmp(argv[i], "-s") == 0)		{ mFlags[FLAG_SUBFOLDERS] = true;	}
			else if (_stricmp(argv[i], "-p") == 0)	{ mFlags[FLAG_PREVIEW] = true;		}
			else if (_stricmp(argv[i], "-q") == 0)	{ mFlags[FLAG_QUIET] = true;		}
			else if (_stricmp(argv[i], "-h") == 0)	{ mFlags[FLAG_HELP] = true;			}
			else
			{
				showError("Unknown command argument passed");
			}
		}
		else
		{
			if (!mSrcRegex)
			{
				int len = strlen(argv[i]) + 1;
				if (len > 0)
				{
					mSrcRegex = new char[len];
					memset(mSrcRegex, 0, len);
					strcpy_s(mSrcRegex, len, argv[i]);
				}
			}
			else if (!mDestRegex)
			{
				int len = strlen(argv[i]) + 1;
				if (len > 0)
				{
					mDestRegex = new char[len];
					memset(mDestRegex, 0, len);
					strcpy_s(mDestRegex, len, argv[i]);
					ret = true;
				}
			}
		}
	}

	return ret;
}

/**
 * Class / Structure to hold a list of files that need renaming
 */
class ChangeEntry {
public:
	std::string mSrc;
	std::string mDest;
};

/**
 * Main process
 * @param argc
 * @param argv
 */
int main(int argc, char*argv[])
{
int ret = 0 ; // Success

	if(argc < 2)
	{
		// Error
		showError("Missing Parameters");

		// Show help
		showHelp();
	} 
	else 
	{
		std::vector<ChangeEntry*> changeList;

		// Parse Commands 
		if(parseCommandline(argc, argv))
		{
			if (mFlags[FLAG_HELP])
			{
				showHelp();
			}
			else if (mSrcRegex && mDestRegex)
			{
				// Process 
				std::filesystem::path path = std::filesystem::current_path();
				printf("Processing Path: %ls\r\n", path.c_str());
				for (const auto & entry : std::filesystem::directory_iterator(path))
				{
					std::string filename = entry.path().u8string().substr(path.u8string().length()+1);

					std::string oFilename = filename;
					RE2::Replace(&oFilename, mSrcRegex, mDestRegex);
					if (oFilename != filename)
					{
						ChangeEntry *ce = new ChangeEntry();
						if (ce)
						{
							ce->mSrc = filename;
							ce->mDest = oFilename;
							changeList.push_back(ce);
						}
						printf(" - %s = [Modified] %s\r\n", filename.c_str(), oFilename.c_str());
					}
					else
					{
						printf(" - %s = [%s]\r\n", filename.c_str(), "Skipped");
					}
				}

				// Process
				if(changeList.size() > 0 && !mFlags[FLAG_PREVIEW])
				{
					bool apply = false;
					apply = mFlags[FLAG_QUIET];
					if(!apply)
					{
						printf("Apply Changes (y/N) :");
						char t = _getch();
						if (t == 'y' || t == 'Y')
						{
							apply = true;
						}
						printf("\r\n");
					}

					if (apply)
					{
						// Run through List;
						printf("Running list: \r\n");

						int errorCount = 0;
						for (ChangeEntry* ce : changeList)
						{
							printf(" - [%s] to [%s]", ce->mSrc.c_str(), ce->mDest.c_str());

							std::string src = path.u8string() + "/" + ce->mSrc;
							std::string dest = path.u8string() + "/" + ce->mDest;

							std::error_code errCode;
							std::filesystem::rename(src, dest, errCode);

							if (errCode.value() != 0)
							{
								errorCount++;
								printf(" Failed (Error: %s)", errCode.message().c_str());
								ret = 1;
							}
							else
							{
								printf(" Successful");
							}

							printf("\r\n");
						}

				
						printf("Task Completed [%d errors][%d successful]\r\n", errorCount, changeList.size() - errorCount);
					}
				}
			}
			else
			{
				showError("Missing src and dest regex parameters");
			}

		}
		else
		{
			showError("Unable to parse command line");
		}
	}
	return ret;
}
