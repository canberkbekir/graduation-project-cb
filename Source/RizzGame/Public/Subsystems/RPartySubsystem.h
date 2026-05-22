// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Events/PartyEvents.h"
#include "RPartySubsystem.generated.h"

class ARCharacterBase;
/**
 * 
 */
UCLASS()
class RIZZGAME_API URPartySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	
	/** 
 * Adds a character to the party if it's not already present. 
 * @param Character - The character to add to the party.
 */
	UFUNCTION(BlueprintCallable, Category = "PartySubsystem")
	void AddToParty(ARCharacterBase* Character);
	/** 
	 * Removes a character from the party.
	 * @param Character - The character to remove from the party.
	 */
	UFUNCTION(BlueprintCallable, Category = "PartySubsystem")
	void RemoveFromParty(ARCharacterBase* Character);
	/** 
	 * Sets the selected character in the party.
	 * @param Character - The character to set as currently selected.
	 */
	UFUNCTION(BlueprintCallable, Category = "PartySubsystem")
	void SetSelectedCharacter(ARCharacterBase* Character);
	/** 
	 * Gets the currently selected character in the party.
	 * @return The selected character.
	 */
	UFUNCTION(BlueprintCallable, Category = "PartySubsystem")
	ARCharacterBase* GetSelectedCharacter() const;
	
	UFUNCTION(BlueprintCallable,BlueprintPure, Category = "PartySubsystem")
	bool IsInParty(ARCharacterBase* Character);
	
	
	UFUNCTION(BlueprintCallable, Category = "PartySubsystem")
	TArray<ARCharacterBase*> GetPartyMembers() const
	{
		TArray<ARCharacterBase*> RawMembers;
		RawMembers.Reserve(PartyMembers.Num());
		for (const TObjectPtr<ARCharacterBase>& Ptr : PartyMembers)
		{
			RawMembers.Add(Ptr.Get());
		}
		return RawMembers;
	}
	
	/** 
	 * All characters currently in the party.
	 */
	UPROPERTY()
	TArray<TObjectPtr<ARCharacterBase>> PartyMembers;
	/** 
	 * The currently selected character in the party.
	 */
	UPROPERTY()
	TObjectPtr<ARCharacterBase> SelectedCharacter;
	
};
