// Copyright (c) MarmoDrake. All Rights Reserved.


#include "LevelUpInfo.h"

int32 ULevelUpInfo::FindLevelForXP(int32 XP) const
{
	int32 Level = 1;
	bool bSearching = true;
	while (bSearching) {

		// номер уровня == индекс массива. LevelUpInformation[0] не содержит нужной инфы  
		if (LevelUpInformation.Num() - 1 <= Level) {
			break;
		}
		if (XP >= LevelUpInformation[Level].LevelUpRequirement) {
			++Level;
		} else {
			bSearching = false;
		}
	}
	return Level;
}