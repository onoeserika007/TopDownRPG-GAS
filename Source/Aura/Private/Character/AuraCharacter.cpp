// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AuraCharacter.h"

#include "AbilitySystemComponent.h"
#include "NiagaraComponent.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Player/AuraPlayerController.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"

AAuraCharacter::AAuraCharacter()
{
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>("CameraBoom");
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->SetUsingAbsoluteLocation(false);
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritRoll = false;
	CameraBoom->bInheritYaw = false;
	
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCameraComponent"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false;
	
	
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 1600.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	CharacterClass = ECharacterClass::Elementalist;

	LevelUpNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>("LevelUpNiagaraComponent");
	LevelUpNiagaraComponent->SetupAttachment(GetRootComponent());
	LevelUpNiagaraComponent->bAutoActivate = false;
}

void AAuraCharacter::InitAbilityActorInfo()
{
	Super::InitAbilityActorInfo();
	// 22. Init ability actor info
	if (AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>())
	{
		AuraPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(AuraPlayerState, this);
		AbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
		auto AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent);
		AuraAbilitySystemComponent->OnAbilityActorInfoSet();
		AttributeSet = AuraPlayerState->GetAttributeSet();
		InitializeDefaultAttributes();

		if (AAuraPlayerController* AuraPlayerController = Cast<AAuraPlayerController>(GetController()))
		{
			if (AAuraHUD* AuraHUD = Cast<AAuraHUD>(AuraPlayerController->GetHUD()))
			{
				AuraHUD->InitOverlay(AuraPlayerController, AuraPlayerState, AbilitySystemComponent, AttributeSet);
			}
		}
	}
}

int32 AAuraCharacter::GetPlayerLevel_Implementation() const
{
	if (const AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>())
	{
		return AuraPlayerState->GetPlayerLevel();
	}
	return Super::GetPlayerLevel();
}

void AAuraCharacter::AddToXP_Implementation(int32 InXP)
{
	if (AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>())
	{
		AuraPlayerState->AddToXP(InXP);
	}
}

void AAuraCharacter::LevelUp_Implementation()
{
	MulticastLevelUpParticles();
}

void AAuraCharacter::MulticastLevelUpParticles_Implementation() const
{
	if (IsValid(LevelUpNiagaraComponent))
	{
		const FVector CameraLocation = TopDownCameraComponent->GetComponentLocation();
		const FVector NiagaraSystemLocation = LevelUpNiagaraComponent->GetComponentLocation();
		const FRotator ToCameraRotation = (CameraLocation - NiagaraSystemLocation).Rotation();
		LevelUpNiagaraComponent->SetWorldRotation(ToCameraRotation);
		LevelUpNiagaraComponent->Activate(true);
	}
}

int32 AAuraCharacter::GetXP_Implementation() const 
{
	if (const AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>())
	{
		return AuraPlayerState->GetXP();
	}
	return 0;
}

int32 AAuraCharacter::FindLevelForXP_Implementation(int32 InXP) const
{
	if (const AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>())
	{
		return AuraPlayerState->LevelUpInfo->FindLevelForXP(InXP);
	}
	return 1;
}

int32 AAuraCharacter::GetAttributePointsReward_Implementation(int32 InLevel) const
{
	if (const AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>())
	{
		if (AuraPlayerState->LevelUpInfo->LevelUpInfos.Num() > InLevel)
		{
			return AuraPlayerState->LevelUpInfo->LevelUpInfos[InLevel].AttributePointAward;
		}
	}
	return 0;
}

int32 AAuraCharacter::GetSpellPointsReward_Implementation(int32 InLevel) const
{
	if (const AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>())
	{
		if (AuraPlayerState->LevelUpInfo->LevelUpInfos.Num() > InLevel)
		{
			return AuraPlayerState->LevelUpInfo->LevelUpInfos[InLevel].SpellPointAward;
		}
	}
	return 0;
}

void AAuraCharacter::AddToAttributePoints_Implementation(int32 InAttributePoints)
{
	if (AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>())
	{
		AuraPlayerState->AddToAttributePoints(InAttributePoints);
	}
}

void AAuraCharacter::AddToSpellPoints_Implementation(int32 InSpellPoints)
{
	if (AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>())
	{
		AuraPlayerState->AddToSpellPoints(InSpellPoints);
	}
}

void AAuraCharacter::AddToPlayerLevel_Implementation(int32 InPlayerLevel)
{
	if (AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>())
	{
		AuraPlayerState->AddToLevel(InPlayerLevel);
		UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(GetAbilitySystemComponent());
		if (AuraASC)
		{
			AuraASC->UpdateAbilityStatuses(AuraPlayerState->GetPlayerLevel());
		}
	}
}

int32 AAuraCharacter::GetAttributePoints_Implementation() const
{
	if (const AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>())
	{
		return AuraPlayerState->GetAttributePoints();
	}
	return 0;
}

int32 AAuraCharacter::GetSpellPoints_Implementation() const 
{
	if (const AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>())
	{
		return AuraPlayerState->GetSpellPoints();
	}
	return 0;
}

void AAuraCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	// Init ability actor info for the server
	InitAbilityActorInfo();
	AddCharacterAbilities();
}

void AAuraCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	// Init ability actor info for the client
	InitAbilityActorInfo();
}
