

#include "EnvQueryTest_CoverHidesFromThreats.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_ActorBase.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"

UEnvQueryTest_CoverHidesFromThreats::UEnvQueryTest_CoverHidesFromThreats(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ValidItemType = UEnvQueryItemType_ActorBase::StaticClass();
	Cost = EEnvTestCost::High;
	SetWorkOnFloatValues(false);
	
	EnemyHeightOffset.DefaultValue = 80.0f;
	CoverHeightOffset.DefaultValue = 45.0f;
	
	TraceChannel = TraceTypeQuery1;
	bRequireAllHidden.DefaultValue = true;
    
    BoolValue.DefaultValue = true;
}

void UEnvQueryTest_CoverHidesFromThreats::RunTest(FEnvQueryInstance& QueryInstance) const
{
    UObject* queryOwner = QueryInstance.Owner.Get();
    if (!queryOwner)
        return;

    UWorld* world = GEngine->GetWorldFromContextObject(queryOwner, EGetWorldErrorMode::LogAndReturnNull);
    if (!world)
        return;
    
    if (!ThreatsContext)
        return;
    
    EnemyHeightOffset.BindData(queryOwner, QueryInstance.QueryID);
    CoverHeightOffset.BindData(queryOwner, QueryInstance.QueryID);
    bRequireAllHidden.BindData(queryOwner, QueryInstance.QueryID);

    const float enemyOffset = EnemyHeightOffset.GetValue();
    const float coverOffset = CoverHeightOffset.GetValue();
    const bool brequireAll = bRequireAllHidden.GetValue();
    
    TArray<AActor*> threatActors;
    QueryInstance.PrepareContext(ThreatsContext, threatActors);
    
    if (threatActors.Num() == 0)
    {
        for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
            It.SetScore(TestPurpose, FilterType, true, true);
        return;
    }
    
    FCollisionQueryParams traceParams(SCENE_QUERY_STAT(EnvQueryTrace), false); 
    ECollisionChannel collisionChannel = UEngineTypes::ConvertToCollisionChannel(TraceChannel);
    
    for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
    {
        AActor* coverActor = GetItemActor(QueryInstance, It.GetIndex());
        if (!coverActor)
        {
            It.SetScore(TestPurpose, FilterType, false, true);
            continue;
        }

        const FVector CoverTestLocation = coverActor->GetActorLocation() + FVector(0, 0, coverOffset);

        bool bhiddenFromAll = true;
        bool bhiddenFromAny = false;
        
        for (AActor* threat : threatActors)
        {
            if (!IsValid(threat))
                continue;

            const FVector ThreatEyeLocation = threat->GetActorLocation() + FVector(0, 0, enemyOffset);
            
            FCollisionQueryParams CurrentParams = traceParams;
            CurrentParams.AddIgnoredActor(coverActor);
            CurrentParams.AddIgnoredActor(threat);

            FHitResult HitResult;
            bool bBlocked = world->LineTraceSingleByChannel(
                HitResult,
                ThreatEyeLocation,
                CoverTestLocation,
                collisionChannel,
                CurrentParams);

            DrawDebugLine(world, ThreatEyeLocation, CoverTestLocation,
                bBlocked ? FColor::Green : FColor::Red, 
    false, 2.0f, 0, 2.0f);
            DrawDebugLine(world, ThreatEyeLocation, CoverTestLocation,
                 bBlocked ? FColor::Green : FColor::Red, 
    false, 2.0f, 0, 2.0f);
            
            if (bBlocked)
            {
                DrawDebugSphere(world, HitResult.ImpactPoint, 15.0f, 12, 
                    FColor::Yellow, false, 2.0f);
            }
            
            if (bBlocked)
            {
                bhiddenFromAny = true;
            }
            else
            {
                bhiddenFromAll = false;
                if (brequireAll)
                    break;
            }
        }
        
        const bool bIsValid = brequireAll ? bhiddenFromAll : bhiddenFromAny;
        It.SetScore(TestPurpose, FilterType, bIsValid, BoolValue.GetValue());
    }
}

FText UEnvQueryTest_CoverHidesFromThreats::GetDescriptionTitle() const
{
    FString ContextName = TEXT("None");
    
    if (ThreatsContext)
    {
        ContextName = ThreatsContext->GetName();
        ContextName.RemoveFromStart(TEXT("Default__"));
        ContextName.RemoveFromStart(TEXT("EnvQueryContext_"));
    }
    
    return FText::FromString(FString::Printf(TEXT("Cover Hides From: %s"), *ContextName));
}

FText UEnvQueryTest_CoverHidesFromThreats::GetDescriptionDetails() const
{
    FString Description;

    Description += FString::Printf(TEXT("Require All Hidden: %s\n"),
        bRequireAllHidden.IsDynamic() ? TEXT("Dynamic") :
        bRequireAllHidden.DefaultValue ? TEXT("True") : TEXT("False"));

    Description += FString::Printf(TEXT("Enemy Offset: %s, Cover Offset: %s"),
        EnemyHeightOffset.IsDynamic() ? TEXT("Dynamic") : *FString::SanitizeFloat(EnemyHeightOffset.DefaultValue),
        CoverHeightOffset.IsDynamic() ? TEXT("Dynamic") : *FString::SanitizeFloat(CoverHeightOffset.DefaultValue));

    return FText::FromString(Description);
}
