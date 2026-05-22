#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "DataProviders/AIDataProvider.h"
#include "EnvQueryTest_CoverDistanceScore.generated.h"

UENUM(BlueprintType)
enum class ECoverDistanceMode : uint8
{
	Querier UMETA(DisplayName = "Distance to Querier"),
	Context UMETA(DisplayName = "Distance to Context"),
	Hybrid UMETA(DisplayName = "Hybrid (Querier + Context)")
};

UCLASS(meta = (DisplayName = "Cover Distance Score"))
class COVERSYSTEM_API UEnvQueryTest_CoverDistanceScore : public UEnvQueryTest
{
	GENERATED_BODY()

public:
	UEnvQueryTest_CoverDistanceScore(const FObjectInitializer& ObjectInitializer);

	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
	
	virtual FText GetDescriptionTitle() const override;
	
	virtual FText GetDescriptionDetails() const override;

	UPROPERTY(EditDefaultsOnly, Category = "Distance")
	ECoverDistanceMode DistanceMode;

	UPROPERTY(EditDefaultsOnly, Category = "Distance", meta = (EditCondition = "DistanceMode != ECoverDistanceMode::Querier"))
	TSubclassOf<UEnvQueryContext> TargetContext;

	UPROPERTY(EditDefaultsOnly, Category = "Distance", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "DistanceMode == ECoverDistanceMode::Hybrid"))
	FAIDataProviderFloatValue QuerierWeight;
};