


#include "EnvQueryTest_CoverDistanceScore.h"
#include "GameFramework/Actor.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_ActorBase.h"

UEnvQueryTest_CoverDistanceScore::UEnvQueryTest_CoverDistanceScore(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ValidItemType = UEnvQueryItemType_ActorBase::StaticClass();
	Cost = EEnvTestCost::Low;
	SetWorkOnFloatValues(true);
	
	DistanceMode = ECoverDistanceMode::Querier;
	QuerierWeight.DefaultValue = 0.5f;
    
    ScoringEquation = EEnvTestScoreEquation::InverseLinear;
    ClampMinType = EEnvQueryTestClamping::None;
    ClampMaxType = EEnvQueryTestClamping::None;
}

void UEnvQueryTest_CoverDistanceScore::RunTest(FEnvQueryInstance& QueryInstance) const
{
    UObject* queryOwner = QueryInstance.Owner.Get();
    if (!queryOwner)
        return;
    
    AActor* querier = Cast<AActor>(queryOwner);
    FVector querierLocation = querier ? querier->GetActorLocation() : FVector::ZeroVector;
    
    TArray<FVector> contextLocations;

    if (DistanceMode != ECoverDistanceMode::Querier)
    {
        if (!TargetContext)
            return; 
        QueryInstance.PrepareContext(TargetContext, contextLocations);
        if (contextLocations.Num() == 0)
            return;
    }

    float weight = 0.5f;
    if (DistanceMode == ECoverDistanceMode::Hybrid)
    {
        QuerierWeight.BindData(queryOwner, QueryInstance.QueryID);
        weight = QuerierWeight.GetValue();
    }
    
    for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
    {
        const FVector itemLocation = GetItemLocation(QueryInstance, It.GetIndex());

        float finalDistance = 0.0f;

        switch (DistanceMode)
        {
            case ECoverDistanceMode::Querier:
                finalDistance = FVector::Dist(itemLocation, querierLocation);
                break;
            case ECoverDistanceMode::Context:
            {
                finalDistance = MAX_FLT;
                for (const FVector& contextLoc : contextLocations)
                {
                    const float dist = FVector::Dist(itemLocation, contextLoc);
                    if (dist < finalDistance)
                        finalDistance = dist;
                }
                break;
            }
            case ECoverDistanceMode::Hybrid:
            {
                const float querierDist = FVector::Dist(itemLocation, querierLocation);
                float contextDist = MAX_FLT;
                for (const FVector& contextLoc : contextLocations)
                {
                    const float dist = FVector::Dist(itemLocation, contextLoc);
                    if (dist < contextDist)
                        contextDist = dist;
                }
                finalDistance = querierDist * weight + contextDist * (1.0f - weight);
                break;
            }
        }
        It.SetScore(TestPurpose, FilterType, finalDistance, FloatValueMin.GetValue(), FloatValueMax.GetValue());
    }
}

FText UEnvQueryTest_CoverDistanceScore::GetDescriptionTitle() const
{
    FString ModeStr;
    switch (DistanceMode)
    {
        case ECoverDistanceMode::Querier:
            ModeStr = TEXT("Querier");
            break;
        case ECoverDistanceMode::Context:
        {
            FString ContextName = TEXT("None");
            if (TargetContext)
            {
                ContextName = TargetContext->GetName();
                ContextName.RemoveFromStart(TEXT("Default__"));
                ContextName.RemoveFromStart(TEXT("EnvQueryContext_"));
            }
            ModeStr = ContextName;
            break;
        }
        case ECoverDistanceMode::Hybrid:
        {
            FString ContextName = TEXT("None");
            if (TargetContext)
            {
                ContextName = TargetContext->GetName();
                ContextName.RemoveFromStart(TEXT("Default__"));
                ContextName.RemoveFromStart(TEXT("EnvQueryContext_"));
            }
            ModeStr = FString::Printf(TEXT("Querier + %s"), *ContextName);
            break;
        }
    }

    return FText::FromString(FString::Printf(TEXT("Distance to: %s"), *ModeStr));
}

FText UEnvQueryTest_CoverDistanceScore::GetDescriptionDetails() const
{
    if (DistanceMode == ECoverDistanceMode::Hybrid)
    {
        FString WeightStr = QuerierWeight.IsDynamic() ? TEXT("Dynamic") : *FString::SanitizeFloat(QuerierWeight.DefaultValue);
        return FText::FromString(FString::Printf(TEXT("Querier Weight: %s"), *WeightStr));
    }
    
    return FText::GetEmpty();
}
