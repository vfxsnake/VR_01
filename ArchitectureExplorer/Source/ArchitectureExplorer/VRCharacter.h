// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VRCharacter.generated.h"

//forward Declaring:
class USceneComponent;
class UCameraComponent;
class UStaticMeshComponent;
class UPostProcessComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UCurveFloat;
class USplineComponent;
class UMotionControllerComponent;
class UStaticMeshComponent;
struct FPredictProjectilePathResult;


// struct FHitResult;

UCLASS()
class ARCHITECTUREEXPLORER_API AVRCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AVRCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	// camera pointer
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* Camera;
	
	// vr head set root offset:
	UPROPERTY(VisibleAnywhere)
	USceneComponent* VRRoot;

	// destination head set root offset:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* DestinationMarker;

	// PostProces component head set root offset:
	UPROPERTY(EditAnywhere)
	UPostProcessComponent* PostProcessComponent;

	UPROPERTY(EditAnywhere)
	USplineComponent* TeleportCurvePath;

	// adding the interface of material to character:
	UPROPERTY(EditAnywhere)
	UMaterialInterface* BlinkerMaterialBase;

	// material instance store:
	UMaterialInstanceDynamic* BlinkerMaterialInstance;

	// blinker Curve Radius pointer:
	UPROPERTY(EditAnywhere)
	UCurveFloat* RadiusRampVelocity;
	
	UPROPERTY(EditDefaultsOnly)
	float TeleportProjectileSpeed =  1000;

	UPROPERTY(EditDefaultsOnly)
	float TeleportSimulationTime =  10;

	UPROPERTY(EditDefaultsOnly)
	float TeleportRadius =  10.f;

	UPROPERTY(EditDefaultsOnly)
	float TeleportCameraFade =  0.6f;

	UPROPERTY(EditDefaultsOnly)
	FVector TeleportProjectionExtent =  FVector(100.f);

	UPROPERTY(EditDefaultsOnly)
	UMotionControllerComponent* LeftController;
	
	UPROPERTY(EditDefaultsOnly)
	UMotionControllerComponent* RightController;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* DynamicMesh;

	// private Funcitons:
	void MoveForward(float Throttle);
	void MoveRight(float Throttle);

	void TeleportTo(FHitResult& HitResult); 

	void CameraFade(float InValue, float OutValue) const;

	void BeginTeleport();
	
	void EndTeleport();

	void AdjustBlinkRadius();

	FVector2D GetBlinkerCentre();

	void UpdatePathCurve(FPredictProjectilePathResult Result);

	bool bIsMovingForward(FVector VelDirection);
};
