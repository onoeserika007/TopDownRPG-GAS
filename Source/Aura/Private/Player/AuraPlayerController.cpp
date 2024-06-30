// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AuraPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Actor/MagicCircle.h"
#include "Aura/Aura.h"
#include "Components/DecalComponent.h"
#include "Components/SplineComponent.h"
#include "Input/AuraInputComponent.h"
#include "Interaction/EnemyInterface.h"
#include "GameFramework/Character.h"
#include "Interaction/HighlightInterface.h"
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
	UpdateMagicCircleLocation();
}

void AAuraPlayerController::ShowMagicCircle(UMaterialInterface* DecalMaterial)
{
	if (!IsValid(MagicCircle))
	{
		MagicCircle = GetWorld()->SpawnActor<AMagicCircle>(MagicCircleClass);
		if (DecalMaterial)
		{
			MagicCircle->MagicCircleDecal->SetMaterial(0, DecalMaterial);
		}
	}
}

void AAuraPlayerController::HideMagicCircle()
{
	if (IsValid(MagicCircle))
	{
		MagicCircle->Destroy();
		MagicCircle = nullptr;
	}
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
	if (GetAsc() && GetAsc()->HasMatchingGameplayTag(FAuraGameplayTags::GetInstance().Player_Block_InputPressed))
	{
		return;
	}
	
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
	if (GetAsc() && GetAsc()->HasMatchingGameplayTag(FAuraGameplayTags::GetInstance().Player_Block_CursorTrace))
	{
		if (ThisActor) IHighlightInterface::Execute_HighlightActor(ThisActor);
		if (LastActor) IHighlightInterface::Execute_UnHighlightActor(LastActor);
		ThisActor = nullptr;
		LastActor = nullptr;
		return;
	}

	const ECollisionChannel TraceChannel = IsValid(MagicCircle)? ECC_Exclude_Players : ECollisionChannel::ECC_Visibility;
	
	GetHitResultUnderCursor(TraceChannel, false, CursorHit);
	LastActor = ThisActor;
	if (IsValid(CursorHit.GetActor()) && CursorHit.GetActor()->Implements<UHighlightInterface>())
	{
		ThisActor = CursorHit.GetActor();
	}
	else
	{
		ThisActor = nullptr;
	}
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
		if (ThisActor) IHighlightInterface::Execute_HighlightActor(ThisActor);
		if (LastActor) IHighlightInterface::Execute_UnHighlightActor(LastActor);
	}
}

void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (GetAsc() && GetAsc()->HasMatchingGameplayTag(FAuraGameplayTags::GetInstance().Player_Block_InputPressed))
	{
		return;
	}
	
	if (InputTag.MatchesTagExact(FAuraGameplayTags::GetInstance().InputTag_LMB))
	{
		if (IsValid(ThisActor))
		{
			TargetingStatus = ThisActor->Implements<UEnemyInterface>() ? ETargetingStatus::TargetingEnemy : ETargetingStatus::TargetingNoEnemy;
		}
		else
		{
			TargetingStatus = ETargetingStatus::NotTargeting;
		}
		bAutoRunning = false; // since it's a short press
	}

	if (GetAsc())
	{
		GetAsc()->AbilityInputTagPressed(InputTag);
	}
}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (GetAsc() && GetAsc()->HasMatchingGameplayTag(FAuraGameplayTags::GetInstance().Player_Block_InputReleased))
	{
		return;
	}
	
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
	
	if (TargetingStatus != ETargetingStatus::TargetingEnemy && !bShiftKeyDown)
	{
		if (FollowTime <= ShortPressThreshold) // Treat as click ground
		{
			if (IsValid(ThisActor) && ThisActor->Implements<UHighlightInterface>())
			{
				IHighlightInterface::Execute_SetMoveToLocation(ThisActor, CachedDestination); // Override CachedDestination if this func overrided.
			}
			else
			{
				if (GetAsc() && !GetAsc()->HasMatchingGameplayTag(FAuraGameplayTags::GetInstance().Player_Block_InputPressed))
				{
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ClickNiagaraSystem, CachedDestination); // Click effect
				}
			}
			
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
		TargetingStatus = ETargetingStatus::TargetingNoEnemy;
	} // Release to Navigate
}

void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (GetAsc() && GetAsc()->HasMatchingGameplayTag(FAuraGameplayTags::GetInstance().Player_Block_InputHeld))
	{
		return;
	}
	
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
	if (TargetingStatus == ETargetingStatus::TargetingEnemy || bShiftKeyDown)
	{
		if (GetAsc())
		{
			GetAsc()->AbilityInputTagHeld(InputTag);
		}
	}
	else
	{
		// Since input trigger every frame???
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

void AAuraPlayerController::UpdateMagicCircleLocation()
{
	if (IsValid(MagicCircle))
	{
		MagicCircle->SetActorLocation(CursorHit.ImpactPoint);
	}
}
