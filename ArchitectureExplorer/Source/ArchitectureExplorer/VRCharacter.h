// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VRCharacter.generated.h"

//forward Declaring:
class USceneComponent;
class UCameraComponent;

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


	// private Funcitons:
	void MoveForward(float Throttle);
	void MoveRight(float Throttle);


};
