#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/HitResult.h"
#include "RClickResolver.generated.h"

class ARPlayerControllerBase;
class URActionBarWidget;
class ARCharacterBase;

/**
 * Resolves what a world click should do based on context:
 *   1. Active ability selection  → give target to ability
 *   2. Interactable actor        → interact (NPC, door, item, etc.)
 *   3. Enemy character           → select / target
 *   4. Party member              → select character
 *   5. Ground                    → move (exploration only; no-op in combat)
 *
 * Also provides cursor icon feedback for hover.
 * Add new cases here as features expand — the controller stays thin.
 */
UCLASS()
class RIZZGAME_API URClickResolver : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(ARPlayerControllerBase* InController);
	void SetActionBar(URActionBarWidget* InActionBar);

	/** Call on every primary click with the world hit under cursor. */
	void Resolve(const FHitResult& Hit);

	/** Call every frame on hover to get the appropriate cursor icon. */
	EMouseCursor::Type GetCursorFor(const FHitResult& Hit) const;

private:
	TWeakObjectPtr<ARPlayerControllerBase> Controller;
	TWeakObjectPtr<URActionBarWidget> ActionBarWidget;

	bool IsInCombat() const;
	bool IsPlayerTurn() const;
	ARCharacterBase* GetSelectedCharacter() const;

	void ClearSelectionVisual();
	void ApplySelectionVisual(AActor* Actor);

	// Resolution steps — each returns true if the click was handled.
	bool TryAbilityTarget(const FHitResult& Hit);
	bool TryInteractable(AActor* HitActor);
	bool TryCharacter(AActor* HitActor);
	void DoGroundMove(const FVector& Location);
};
