Program supports a couple of modes and features that may be configured by passing cmd parameters.

To run game in basic mode (ony features described in PDF instuction) execute programe without any extra cmd parameters.
Defaults:
1. Game starts in keyboard input mode. Shoot by pressing 'F', move left/right with arrows.
2. Aliens are able to kill each other.

To get info about extra options, type "SpaceRaiders.exe --help".
Options may be combined with each other despite cases described in program help.

More info about some extra options:
--hardMode:
	1. There are more aliens spawned at start
	2. Aliens have higher y speed
	3. Aliens will shoot lasers more frequently
	4. Starting min/max numbers of aliens spawned per wave is higher
	5. Better aliens are able to spawn "Strong lasers" with 50% prob. per shoot (marked as 'w'), 
	   which are resistant to player lasers and may destroy block walls (despite normal alien lasers)

--specialFeature:
	At each wave one special "Exploding alien" can be spawned with 50% probability. 
	Exploding alien is marked with 'E', it is not able to transoform to better alien and it has higher y speed than normal aliens (x speed remains the same).
	If laser hits exploding alien, it generates explosion ring that kills anything within it's reach 
	(including player ship so be careful if you want to shoot it when it's got too close).
	
You'll find implementation details in source code.
