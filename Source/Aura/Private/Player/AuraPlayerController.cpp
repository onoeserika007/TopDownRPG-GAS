// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AuraPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/SplineComponent.h"
#include "Input/AuraInputComponent.h"
#include "Interaction/EnemyInterface.h"
#include "GameFramework/Character.h"
#include "UI/Widget/DamageTextComponent.h"

AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;

	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	CursorTrace();

	AutoRun();
}

void AAuraPlayerController::ShowDamageNumber_Implementation(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bCriticalHit)
{
	// Return true if the object is usable: non-null and not pending kill or garbage
	if (IsValid(TargetCharacter) && DamageTextComponentClass)
	{
		UDamageTextComponent* DamageText = NewObject<UDamageTextComponent>(TargetCharacter, DamageTextComponentClass);
		DamageText->RegisterComponent();
		DamageText->AttachToComponent(TargetCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		DamageText->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		DamageText->SetDamageText(DamageAmount, bBlockedHit, bCriticalHit);
	}
}

void AAuraPlayerController::AutoRun()
{
	if (!bAutoRunning) return;
	if (APawn* ControlledPawn = GetPawn())
	{
		const FVector LocationOnSpline = Spline->FindLocationClosestToWorldLocation(ControlledPawn->GetActorLocation(), ESplineCoordinateSpace::World);
		const FVector Direction = Spline->FindDirectionClosestToWorldLocation(LocationOnSpline, ESplineCoordinateSpace::World);
		DrawDebugSphere(GetWorld(), LocationOnSpline, 8.f, 12, FColor::Red, false, 1);
		DrawDebugSphere(GetWorld(), CachedDestination, 8.f, 12, FColor::Cyan, false, 1);
		ControlledPawn->AddMovementInput(Direction);

		const float DistanceToDestination = (LocationOnSpline - CachedDestination).Length();
		if (DistanceToDestination <= AutoRunAcceptanceRadius)
		{
			bAutoRunning = false;
		}
	}
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
	UAuraInputComponent* AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent);
	if (IA_Move) {
		AuraInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &ThisClass::Move);
	}

	if (IA_Shift)
	{
		AuraInputComponent->BindAction(IA_Shift, ETriggerEvent::Started, this, &ThisClass::ShiftPressed);
		AuraInputComponent->BindAction(IA_Shift, ETriggerEvent::Completed, this, &ThisClass::ShiftReleased);
	}

	AuraInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);
	
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
		if (LastActor != ThisActor)
		{
			if (ThisActor) ThisActor->HighlightActor();
			if (LastActor) LastActor->UnHighlightActor();
		}
	}
}

void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (InputTag.MatchesTagExact(FAuraGameplayTags::GetInstance().InputTag_LMB))
	{
		bTargeting = ThisActor? true : false;
		bAutoRunning = false; // since it's a short press
	}
}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::GetInstance().InputTag_LMB))
	{
		if (GetAsc())
		{
			GetAsc()->AbilityInputTagReleased(InputTag);
		}
		return;
	}

	// Notify ASC input released anyway 
	GetAsc()->AbilityInputTagReleased(InputTag);
	
	if (!bTargeting && !bShiftKeyDown)
	{
		if (FollowTime <= ShortPressThreshold)
		{
			if (APawn* ControlledPawn = GetPawn())
			{
				UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlledPawn->GetActorLocation(), CachedDestination);
				if (NavPath)
				{
					Spline->ClearSplinePoints();
					for (const auto& PointLoc: NavPath->PathPoints)
					{
						Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);
						DrawDebugSphere(GetWorld(), PointLoc, 8.f, 8, FColor::Green, false, 5.f);
						CachedDestination = PointLoc; // store last one
					}
					bAutoRunning = true;
				}
			}
		}
		FollowTime = 0.0f;
		bTargeting = false;
	} // Release to Navigate
}

void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::GetInstance().InputTag_LMB))
	{
		if (GetAsc())
		{
			GetAsc()->AbilityInputTagHeld(InputTag);
		}
		return;
	}

	// if targeting enemy,do not move, do asc
	// bShiftKeyDown is only useful for LMB
	if (bTargeting || bShiftKeyDown)
	{
		if (GetAsc())
		{
			GetAsc()->AbilityInputTagHeld(InputTag);
		}
	}
	else
	{
		FollowTime += GetWorld()->GetDeltaSeconds();
		if (CursorHit.bBlockingHit)
		{
			CachedDestination = CursorHit.ImpactPoint;
		}

		if (APawn* ControlledPawn = GetPawn())
		{
			const FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
			ControlledPawn->AddMovementInput(WorldDirection);
		}
	}
}

UAuraAbilitySystemComponent* AAuraPlayerController::GetAsc()
{
	if (AuraAbilitySystemComponent == nullptr)
	{
		AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(
			UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>())
			);
	}
	return AuraAbilitySystemComponent;
}
