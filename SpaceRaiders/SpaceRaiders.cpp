// SpaceRaiders.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include <algorithm>
#include <map>

#include "Vector2D.h"
#include "Renderer.h"
#include "Randomization.h"
#include "PlayField.h"

using namespace std;

std::default_random_engine rGen;

void printHelp(const char *appName)
{
	cout << "Usage:\n" << endl;
    cout << appName << " [--testRun] [--testIterations <value>] [--displayGameInfo] " << endl;
    cout << "\t\t [--hardMode] [--specialFeature] [--noAliensFriendFire]" << endl;
    cout << "\t\t [--seed <value>] [--iterationSeepTimeInMs <value>] [--help]" << endl;
	cout << "Args description:" << endl;
    cout << "\t--testRun - ran in test mode (random user input)" << endl;
    cout << "\t--testIterations - set number of test iterations (valid only with --testRun option)" << endl;
	cout << "\t--displayGameInfo - displays runtime info about current iteration index, score and existing objects" << endl;
	cout << "\t--hardMode - there are much more aliens which are shooting more frequently (and more deadly)" << endl;
	cout << "\t--specialFeature - try extra feature" << endl;
	cout << "\t--noAliensFriendFire - aliens will not be able to kill each other" << endl;
	cout << "\t--seed - set randomization seed (integer value)" << endl;
    cout << "\t--iterationSeepTimeInMs - set sleep time between game iterations in milliseconds" << endl;
	cout << "\t--help - display help" << endl;

}

typedef enum
{
	CP_TestRun = 0,
    CP_TestIterations,
	CP_DisplayGameInfo,
	CP_HardMode,
	CP_SpecialFeature,
	CP_NoAliensFriendFire,
	CP_Seed,
    CP_IterationSleepTimeInMs,
	CP_Help
} CmdParameter;

bool parseCommandLineParamenters(int argc, char** argv, GameConfig& gameConfig)
{
	static map<string, CmdParameter> parametersSet =
    {
        { "--testRun", CP_TestRun},
        { "--displayGameInfo", CP_DisplayGameInfo} ,
        { "--hardMode", CP_HardMode },
        { "--specialFeature", CP_SpecialFeature },
        { "--noAliensFriendFire", CP_NoAliensFriendFire },
        { "--help", CP_Help },
        { "--testIterations", CP_TestIterations },
        { "--seed", CP_Seed },
        { "--iterationSeepTimeInMs", CP_IterationSleepTimeInMs }
	};
	for (int i = 1; i < argc; i++)
	{
        string str(argv[i]);
        auto it = parametersSet.find(str);
        int val = 0;
		if (it == parametersSet.end())
		{
			continue;
		}
		switch (it->second)
		{
		case CP_TestRun: gameConfig.testRun = true; break;
		case CP_DisplayGameInfo: gameConfig.displayGameInfo = true; break;
		case CP_HardMode: gameConfig.hardMode = true; break;
		case CP_SpecialFeature: gameConfig.useSpecialFeature = true; break;
		case CP_NoAliensFriendFire: gameConfig.aliensFriendFire = false; break;
        case CP_Help: printHelp(argv[0]); return false;
		case CP_Seed:
        case CP_TestIterations:
        case CP_IterationSleepTimeInMs:
			if (i + 1 == argc) 
			{
                return false;
			}
            val = atoi(argv[++i]);
            switch (it->second)
            {
            case CP_Seed: gameConfig.seed = val; break;
            case CP_TestIterations: gameConfig.testIterations = val; break;
            case CP_IterationSleepTimeInMs: gameConfig.iterationSleepTimeInMs = val; break;
            }
			break;
		}
	}
	return true;
}

int main(int argc, char** argv)
{
	GameConfig config = {false, 500, true, false, false, true, 1, 50};
	if (!parseCommandLineParamenters(argc, argv, config))
	{
		return 0;
	}
	rGen.seed(config.seed);
	Vector2D size(80, 29);
	Renderer mainRenderer(size);
    if (!mainRenderer.AdjustConsoleSize())
    {
        return -1;
    }
    mainRenderer.SetcursorVisibility(false);

	PlayField world(size, config);
    world.SetupGame();
	while(world.IsStillRunning())
	{
		world.Update();
		mainRenderer.Update(world);
		// Sleep a bit so updates don't run too fast
        world.WaitBetweenIterations();
	}
    mainRenderer.SetcursorVisibility(true);
    cout << "Press Enter to exit" << endl;
    cin.get();
	return 0;
}

