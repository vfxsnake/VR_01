// Fill out your copyright notice in the Description page of Project Settings.


#include "VRCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/EngineTypes.h"
#include "DrawDebugHelpers.h"

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

	// teleport destination marker:
	DestinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Destination Marker"));
	DestinationMarker->SetupAttachment(GetRootComponent());


}

// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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

}

void AVRCharacter::MoveForward(float Throttle) 
{
	AddMovementInput(Camera->GetForwardVector(), Throttle);
}

void AVRCharacter::MoveRight(float Throttle) 
{
	AddMovementInput(Camera->GetRightVector(), Throttle);
}

void AVRCharacter::TeleportTo(FHitResult& HitResult) const
{
	FVector StartPos = Camera->GetComponentLocation();
	FVector EndPos = StartPos + (Camera->GetForwardVector()* MaxTeleportDistance);
	// DrawDebugLine(GetWorld(), StartPos, EndPos, FColor::Red, true);

	bool HitFound = false; 
	HitFound = GetWorld()->LineTraceSingleByChannel(HitResult, StartPos, EndPos, ECollisionChannel::ECC_Visibility);
	if (HitFound)
	{
		// DrawDebugLine(GetWorld(), StartPos, EndPos, FColor::Red, true);	
		FVector TeleportPosition = HitResult.Location;
		DestinationMarker->SetVisibility(true);
		DestinationMarker->SetWorldLocation(TeleportPosition);
	}
	else
	{
		DestinationMarker->SetVisibility(false);
	}
	

}

