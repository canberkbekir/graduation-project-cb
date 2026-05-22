#include "UI/CharacterSheet/RCharacterSheetActor.h"

#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture.h"
#include "Core/RCharacterBase.h"   

ARCharacterSheetActor::ARCharacterSheetActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Root
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	// Pivot (döndürme için merkez nokta)
	Pivot = CreateDefaultSubobject<USceneComponent>(TEXT("Pivot"));
	Pivot->SetupAttachment(RootComponent);

	// Preview skeletal mesh
	PreviewMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PreviewMesh"));
	PreviewMesh->SetupAttachment(Pivot);
	PreviewMesh->SetRelativeLocation(FVector::ZeroVector);
	PreviewMesh->SetRelativeRotation(FRotator::ZeroRotator); 

	// Floor
	FloorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMesh"));
	FloorMesh->SetupAttachment(Pivot);
	FloorMesh->SetRelativeLocation(FVector(0.f, 0.f, -90.f)); // karakterin ayağının altında olsun diye örnek offset

	// Backdrop
	BackdropMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BackdropMesh"));
	BackdropMesh->SetupAttachment(Pivot);
	BackdropMesh->SetRelativeLocation(FVector(0.f, -150.f, 0.f));
	BackdropMesh->SetRelativeRotation(FRotator(0.f, 0.f, 90.f)); // dik durması için örnek

	// Lights
	KeyLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("KeyLight"));
	KeyLight->SetupAttachment(Pivot);
	KeyLight->SetRelativeLocation(FVector(150.f, 150.f, 150.f));
	KeyLight->Intensity = 5000.f;

	FillLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FillLight"));
	FillLight->SetupAttachment(Pivot);
	FillLight->SetRelativeLocation(FVector(-150.f, -150.f, 100.f));
	FillLight->Intensity = 3000.f;

	// Scene capture
	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	SceneCapture->SetupAttachment(Pivot);
	SceneCapture->SetRelativeLocation(FVector(0.f, 200.f, 100.f));
	SceneCapture->SetRelativeRotation(FRotator(-10.f, -180.f, 0.f)); // karaktere bakacak şekilde ayarla

	SceneCapture->FOVAngle = 45.f;
	SceneCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	SceneCapture->bCaptureEveryFrame = true;
	SceneCapture->bCaptureOnMovement = true;

	// RenderTarget burada atanmıyor; editorden set edeceksin
	RenderTarget = nullptr;
}

void ARCharacterSheetActor::BeginPlay()
{
	Super::BeginPlay();

	if (SceneCapture && RenderTarget)
	{
		SceneCapture->TextureTarget = RenderTarget;
	}
}

void ARCharacterSheetActor::ApplyFromCharacter(ARCharacterBase* SourceCharacter)
{
	if (!SourceCharacter || !PreviewMesh)
	{
		return;
	}

	USkeletalMeshComponent* SourceMesh = SourceCharacter->GetMesh();
	if (!SourceMesh)
	{
		return;
	}

	// Skeletal mesh kopyala
	PreviewMesh->SetSkeletalMeshAsset(SourceMesh->GetSkeletalMeshAsset());
	
	// Materyalleri kopyala
	const int32 MatCount = SourceMesh->GetNumMaterials();
	for (int32 Index = 0; Index < MatCount; ++Index)
	{
		UMaterialInterface* Mat = SourceMesh->GetMaterial(Index);
		PreviewMesh->SetMaterial(Index, Mat);
	}

	// Anim BP kopyala (varsa)
	if (UAnimInstance* SourceAnimInstance = SourceMesh->GetAnimInstance())
	{
		PreviewMesh->SetAnimInstanceClass(SourceAnimInstance->GetClass());
	}

	// Buraya istersen ekipman / weapon mesh kopyalama mantığını da ekleyebilirsin
	// Örn: SourceCharacter->GetEquippedWeaponMesh() gibi bir fonksiyonun varsa.
}

void ARCharacterSheetActor::SetPreviewYaw(float YawDegrees)
{
	if (!Pivot)
	{
		return;
	}

	FRotator Rot = Pivot->GetRelativeRotation();
	Rot.Yaw = YawDegrees;
	Pivot->SetRelativeRotation(Rot);
}

void ARCharacterSheetActor::AddPreviewYaw(float DeltaYawDegrees)
{
	if (!Pivot)
	{
		return;
	}

	FRotator Rot = Pivot->GetRelativeRotation();
	Rot.Yaw += DeltaYawDegrees;
	Pivot->SetRelativeRotation(Rot);
}
