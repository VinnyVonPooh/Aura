// Copyright (c) MarmoDrake. All Rights Reserved.

#include "AttributeInfo.h"

FAuraAttributeInfo UAttributeInfo::FindAttributeInfoForTag(const FGameplayTag& AttributeTag, const bool bLogNotFound) const
{
	for (const auto& Info : AttributeInformation) {
		if (Info.AttributeTag == AttributeTag) {
			return Info;
		}
	}

	if (bLogNotFound) {
		UE_LOG(LogTemp, Error, TEXT("Can't find Info for AttributeTag [%s] on AttributeInfo [%s]. "), *AttributeTag.ToString(),
			   *GetNameSafe(this));
	}

	return FAuraAttributeInfo();
}