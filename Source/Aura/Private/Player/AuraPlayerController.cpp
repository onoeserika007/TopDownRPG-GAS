// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AuraPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "Interaction/EnemyInterface.h"

AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	CursorTrace();
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(AuraContext);

	// subsystem is a singleton during game
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(AuraContext, 0);
	}

	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	if (IA_Move) {
		EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &ThisClass::Move);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();

	const FRotator YawRotation(0.f, GetControlRotation().Yaw, 0.f);
	const FVector RightVec = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	const FVector ForwardVec = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	if (APawn* ControlledPawn = GetPawn<APawn>()) {
		ControlledPawn->AddMovementInput(RightVec, InputAxisVector.X);
		ControlledPawn->AddMovementInput(ForwardVec, InputAxisVector.Y);
	}

}

void AAuraPlayerController::CursorTrace()
{
	FHitResult CursorHit;
	GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, CursorHit);
	if (CursorHit.bBlockingHit)
	{
		LastActor = ThisActor;
		ThisActor = CursorHit.GetActor();
		/**
		 * Line trace from cursor. There are several scenarios:
		 *	A. LastActor is null && ThisActor is null
		 *		- Do nothing
		 *	B. LastActor is null && ThisActor is valid
		 *		- Hightlight ThisActor
		 *	C. LastActor is valid && ThisActor is null
		 *		- UnHighlight LastActor
		 *	D. Both actors are valid, but LastActor != ThisActor
		 *		- Hightlight ThisActor, and UnHighlight LastActor
		 *	E. Both actors are valid, and are the same actor
		 *		- Do nothing
		 */
		if (LastActor == nullptr)
		{
			if (ThisActor == nullptr) 
			{
				// Case A
			}
			else // Case B
			{
				ThisActor->HighlightActor();
			}
		}
		else
		{
			if (ThisActor == nullptr) // Case C
			{
				LastActor->UnHighlightActor();
			}
			else
			{
				if (LastActor != ThisActor) // Case D
				{
					LastActor->UnHighlightActor();
					ThisActor->HighlightActor();
				}
				else
				{
					// Case E
				}
			}
		}
	}
}
