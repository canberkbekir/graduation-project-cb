

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "EnvQueryTest_IsCoverPointAvailable.generated.h"

UCLASS(meta = (DisplayName = "Is Cover Available"))
class COVERSYSTEM_API UEnvQueryTest_IsCoverPointAvailable : public UEnvQueryTest
{
	GENERATED_BODY()
public:
	UEnvQueryTest_IsCoverPointAvailable(const FObjectInitializer& ObjectInitializer);
	
	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
	
	virtual FText GetDescriptionTitle() const override;
	
	virtual FText GetDescriptionDetails() const override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Cover")
	FAIDataProviderBoolValue IgnoreSelfOccupied;
	
	UPROPERTY(EditDefaultsOnly, Category = "Cover")
	FAIDataProviderBoolValue AcceptReserved;
};
