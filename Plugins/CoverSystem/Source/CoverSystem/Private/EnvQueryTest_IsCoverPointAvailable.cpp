


#include "EnvQueryTest_IsCoverPointAvailable.h"

#include "CoverPoint.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_ActorBase.h"

UEnvQueryTest_IsCoverPointAvailable::UEnvQueryTest_IsCoverPointAvailable(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ValidItemType = UEnvQueryItemType_ActorBase::StaticClass();
	Cost = EEnvTestCost::Low;
	SetWorkOnFloatValues(false);
	
	IgnoreSelfOccupied.DefaultValue = true;
	AcceptReserved.DefaultValue = false;
}

void UEnvQueryTest_IsCoverPointAvailable::RunTest(FEnvQueryInstance& QueryInstance) const
{
	UObject* queryOwner = QueryInstance.Owner.Get();
	if (!queryOwner)
		return;
	
	IgnoreSelfOccupied.BindData(queryOwner, QueryInstance.QueryID);
	AcceptReserved.BindData(queryOwner, QueryInstance.QueryID);

	const bool bIgnoreSelf = IgnoreSelfOccupied.GetValue();
	const bool bAcceptRes = AcceptReserved.GetValue();
	
	AActor* querier = Cast<AActor>(queryOwner);
	
	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		AActor* itemActor = GetItemActor(QueryInstance, It.GetIndex());
		ACoverPoint* coverPoint = Cast<ACoverPoint>(itemActor);
		
		if (!coverPoint)
		{
			It.SetScore(TestPurpose, FilterType, false, true);
			continue;
		}

		bool bIsValid = false;

		switch (coverPoint->CoverState)
		{
			case ECoverState::Available:
				bIsValid = true;
				break;
			case ECoverState::Reserved:
				bIsValid = bAcceptRes;
				break;
			case ECoverState::Occupied:
				bIsValid = bIgnoreSelf && coverPoint->CoverActor == querier;
				break;
			default:
				bIsValid = false;
				break;
		}
		It.SetScore(TestPurpose, FilterType, bIsValid, true);
	}
}

FText UEnvQueryTest_IsCoverPointAvailable::GetDescriptionTitle() const
{
	return FText::FromString("Is Cover Available");
}

FText UEnvQueryTest_IsCoverPointAvailable::GetDescriptionDetails() const
{
	FString Description;

	Description += FString::Printf(TEXT("Ignore Self Occupied: %s\n"),IgnoreSelfOccupied.IsDynamic() ? TEXT("Dynamic") 
		: IgnoreSelfOccupied.DefaultValue ? TEXT("True") : TEXT("False"));
	Description += FString::Printf(TEXT("Accept Reserved: %s"), AcceptReserved.IsDynamic() ? TEXT("Dynamic")
		: AcceptReserved.DefaultValue ? TEXT("True") : TEXT("False"));

	return FText::FromString(Description);
}
