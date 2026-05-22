

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "EnvQueryTest_CoverHidesFromThreats.generated.h"

UCLASS()
class COVERSYSTEM_API UEnvQueryTest_CoverHidesFromThreats : public UEnvQueryTest
{
	GENERATED_BODY()
public:
	UEnvQueryTest_CoverHidesFromThreats(const FObjectInitializer& ObjectInitializer);
	
	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
	
	virtual FText GetDescriptionTitle() const override;
	
	virtual FText GetDescriptionDetails() const override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Threats")
	TSubclassOf<UEnvQueryContext> ThreatsContext;
	
	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	FAIDataProviderFloatValue EnemyHeightOffset;
	
	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	FAIDataProviderFloatValue CoverHeightOffset;
	
	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	TEnumAsByte<ETraceTypeQuery> TraceChannel;
	
	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	FAIDataProviderBoolValue bRequireAllHidden;
};
