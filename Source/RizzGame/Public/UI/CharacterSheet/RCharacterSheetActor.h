// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RCharacterSheetActor.generated.h"


class USceneComponent;
class USkeletalMeshComponent;
class UStaticMeshComponent;
class UPointLightComponent;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class ARCharacterBase;

UCLASS()
class RIZZGAME_API ARCharacterSheetActor : public AActor
{
	GENERATED_BODY()

public:
	ARCharacterSheetActor();

protected:
	virtual void BeginPlay() override;

	/** Root / pivot */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterSheet")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterSheet")
	USceneComponent* Pivot;

	/** Main preview skeletal mesh (character) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterSheet")
	USkeletalMeshComponent* PreviewMesh; 
	/** Simple floor / base */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterSheet")
	UStaticMeshComponent* FloorMesh;

	
	/** Backdrop / background panel */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterSheet")
	UStaticMeshComponent* BackdropMesh;

	/** Lights */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterSheet")
	UPointLightComponent* KeyLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterSheet")
	UPointLightComponent* FillLight;

	/** Scene capture that renders to the render target */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterSheet")
	USceneCaptureComponent2D* SceneCapture;

	/** Render target to draw into (assign in editor) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterSheet")
	UTextureRenderTarget2D* RenderTarget;

public:
	/** Copy visual data from an in-game character to this preview */
	UFUNCTION(BlueprintCallable, Category = "CharacterSheet")
	void ApplyFromCharacter(ARCharacterBase* SourceCharacter);

	/** Set absolute yaw for preview (useful for snapping to preset angles) */
	UFUNCTION(BlueprintCallable, Category = "CharacterSheet")
	void SetPreviewYaw(float YawDegrees);

	/** Add delta yaw (use from mouse drag) */
	UFUNCTION(BlueprintCallable, Category = "CharacterSheet")
	void AddPreviewYaw(float DeltaYawDegrees);
};
