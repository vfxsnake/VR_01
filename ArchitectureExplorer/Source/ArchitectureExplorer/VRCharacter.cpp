// Fill out your copyright notice in the Description page of Project Settings.


#include "VRCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/EngineTypes.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
// #include "NavigationData.h" // just in case of failing creatin the FNavLocation
#include "NavigationSystem.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MotionControllerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AVRCharacter::AVRCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// scene component
	VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VR Root"));
	VRRoot->SetupAttachment(GetRootComponent());

	// Camera component default subobject:
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera Component"));
	// Attaching the camera to root:
	Camera->SetupAttachment(VRRoot);

	// Motion controller setup
	LeftController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("Left Controller"));
	LeftController->SetupAttachment(VRRoot);
	LeftController->SetTrackingSource(EControllerHand::Left);
	LeftController->bDisplayDeviceModel = true;

	RightController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("Right Controller"));
	RightController->SetupAttachment(VRRoot);
	RightController->SetTrackingSource(EControllerHand::Right);
	RightController->bDisplayDeviceModel = true;
	// teleport destination marker:
	DestinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Destination Marker"));
	DestinationMarker->SetupAttachment(GetRootComponent());

	//Post proces component defaul sub object:
	PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("Post Process Component"));
	PostProcessComponent->SetupAttachment(GetRootComponent());

	// Create Curve component 
	TeleportCurvePath = CreateDefaultSubobject<USplineComponent>("Curve Path Component");
	TeleportCurvePath->SetupAttachment(RightController);
}

// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (BlinkerMaterialBase)
	{
		BlinkerMaterialInstance = UMaterialInstanceDynamic::Create(BlinkerMaterialBase, this);
		PostProcessComponent->AddOrUpdateBlendable(BlinkerMaterialInstance);	
	}

	DynamicMesh = NewObject<UStaticMeshComponent>(this);
	DynamicMesh->AttachToComponent(VRRoot,FAttachmentTransformRules::KeepRelativeTransform);
	DynamicMesh->RegisterComponent();
}

// Called every frame
void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	AdjustBlinkRadius();

	// find camera movement to correctly place the camera inside the colision object
	FVector NewCameraOffset = Camera->GetComponentLocation() - GetActorLocation();
	NewCameraOffset.Z = 0.0f; // to constrain just to x,y plane offset
	AddActorWorldOffset(NewCameraOffset);
	VRRoot->AddWorldOffset(-NewCameraOffset);



	// temporal FHitResult
	FHitResult HitResult;
	TeleportTo(HitResult);

}

// Called to bind functionality to input
void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// attaching: MoveForward, MoveRight

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AVRCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AVRCharacter::MoveRight);

	// acction bindings:
	PlayerInputComponent->BindAction(TEXT("Teleport"), IE_Released,this, &AVRCharacter::BeginTeleport);

}

void AVRCharacter::MoveForward(float Throttle) 
{
	AddMovementInput(Camera->GetForwardVector(), Throttle);
}

void AVRCharacter::MoveRight(float Throttle) 
{
	AddMovementInput(Camera->GetRightVector(), Throttle);
}

void AVRCharacter::TeleportTo(FHitResult& HitResult) 
{
	FVector StartPos = RightController->GetComponentLocation();
	FVector Look = RightController->GetForwardVector();
	Look = Look.RotateAngleAxis(30, RightController->GetRightVector());
	// FVector EndPos = StartPos + (Look* MaxTeleportDistance);
	// DrawDebugLine(GetWorld(), StartPos, EndPos, FColor::Red, true);

	bool bHitFound = false; 
	// bHitFound = GetWorld()->LineTraceSingleByChannel(HitResult, StartPos, EndPos, ECollisionChannel::ECC_Visibility);
	
	// Predic projectile path setup:
	// Creatin parameter Param using the constructur of the class
	FPredictProjectilePathParams Params = FPredictProjectilePathParams (TeleportRadius, 
																		StartPos, 
																		Look * TeleportProjectileSpeed, 
																		TeleportSimulationTime, 
																		ECollisionChannel::ECC_Visibility); // the last parameter is an ignore object.

	// Params.ActorsToIgnore.Add(Cast<AActor*>(GetActor()));
	Params.DrawDebugType = EDrawDebugTrace::ForOneFrame;
	Params.bTraceComplex = true;

	FPredictProjectilePathResult Result;
	bHitFound = UGameplayStatics::PredictProjectilePath(this, Params, Result);

	// project on to nav mesh
	FNavLocation NavLocation;
	bool bOnNavMesh = UNavigationSystemV1::GetCurrent(GetWorld())->ProjectPointToNavigation(Result.HitResult.Location, NavLocation, TeleportProjectionExtent);


	if (bHitFound && bOnNavMesh)
	{
		// DrawDebugLine(GetWorld(), StartPos, EndPos, FColor::Red, true);	
		// FVector TeleportPosition = HitResult.Location; // used before projecting on Nav Mesh
		UpdatePathCurve(Result);
		DestinationMarker->SetVisibility(true);
		DestinationMarker->SetWorldLocation(NavLocation.Location);
	}
	else
	{
		DestinationMarker->SetVisibility(false);
	}
	

}

void AVRCharacter::CameraFade(float InValue, float OutValue) const
{
	// fade in camera:
	// get controller and cast as a playerController
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		PC->PlayerCameraManager->StartCameraFade(InValue, OutValue, TeleportCameraFade, FLinearColor::Black);
	}
	
}

void AVRCharacter::BeginTeleport() 
{

	CameraFade(0,1);
	//setup timer:
	FTimerHandle Handle;
	GetWorldTimerManager().SetTimer(Handle, this, &AVRCharacter::EndTeleport, TeleportCameraFade);

}

void AVRCharacter::EndTeleport() 
{	
	FVector HeightOffset = FVector(0);
	HeightOffset.Z = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	SetActorLocation(DestinationMarker->GetComponentLocation() +  HeightOffset);
	CameraFade(1,0);
}

void AVRCharacter::AdjustBlinkRadius() 
{
	if(! RadiusRampVelocity)
	{
		return;
	}
	
	float CurrentVelocity = GetVelocity().Size();
	float Radius = RadiusRampVelocity->GetFloatValue(CurrentVelocity);

	BlinkerMaterialInstance->SetScalarParameterValue(TEXT("Radius"), Radius);
	FVector2D Centre = GetBlinkerCentre();
	BlinkerMaterialInstance->SetVectorParameterValue(TEXT("Centre"), FLinearColor(Centre.X, Centre.Y, 0, 1));
}

FVector2D AVRCharacter::GetBlinkerCentre() 
{
	FVector MovementDirection = GetVelocity().GetSafeNormal();
	
	if (MovementDirection.IsNearlyZero())
	{
		return FVector2D(0.5,0.5);
	}
	
	FVector WorldStationaryLocation = Camera->GetComponentLocation() + MovementDirection * 100;

	// controller
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		FVector2D OutScreenPosition;
		bool bIsProjected = PC->ProjectWorldLocationToScreen(WorldStationaryLocation, OutScreenPosition, true);
		if (bIsProjected && bIsMovingForward(WorldStationaryLocation))
		{
			int32 OutSizeX;
			int32 OutSizeY;
			PC->GetViewportSize(OutSizeX, OutSizeY);

			return FVector2D(OutScreenPosition.X / (float)OutSizeX , OutScreenPosition.Y / (float)OutSizeY) ;
		}
	}

	return FVector2D(0.5,0.5);
}

void AVRCharacter::UpdatePathCurve(FPredictProjectilePathResult Result) 
{
	TeleportCurvePath->ClearSplinePoints(true);
	for (FPredictProjectilePathPointData PointData : Result.PathData)
	{
		// trasfrom from world to locas space using the tranform of the component:
		FVector LocalPosition = TeleportCurvePath->GetComponentTransform().InverseTransformPosition(PointData.Location);
		TeleportCurvePath->AddSplineLocalPoint(LocalPosition);
	}

	TeleportCurvePath->UpdateSpline();
}

bool AVRCharacter::bIsMovingForward(FVector VelDirection) 
{
	float DotProduct = FVector::DotProduct(Camera->GetForwardVector(), VelDirection);


	if (DotProduct >= 0.0f)
	{
		return true;
	}

	return false;
}

