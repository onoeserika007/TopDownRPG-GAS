// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AuraEnemy.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Aura/Aura.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "UI/Widget/AuraUserWidget.h"
#include "AuraGameplayTags.h"
#include "AI/AuraAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AAuraEnemy::AAuraEnemy()
{
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	GetMesh()->SetCollisionResponseToChannel(ECC_PROJECTILE, ECR_Overlap);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_PROJECTILE, ECR_Overlap);

	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet");

	HealthBar = CreateDefaultSubobject<UWidgetComponent>("HealthBar");
	HealthBar->SetupAttachment(GetRootComponent());

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	
}

// HitReact by Tags Added or Removed
// HitReactAbility Activated by Receiving Damage in AuraAttributeSet.
// GA apply GE to ASC, and the GE carry that TAG - Effect.HitReact, and then the callback will be called.
void AAuraEnemy::HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bHitReacting = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bHitReacting? 0.f : BaseWalkSpeed;
	// AiController only valid on server, and AI's behavior is dominated by server.
	if (AuraAIController && AuraAIController->GetBlackboardComponent())
	{
		AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), bHitReacting);
	}
}

void AAuraEnemy::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	InitAbilityActorInfo();
	if (HasAuthority())
	{
		// only valid on server
		UAuraAbilitySystemLibrary::GiveStartupAbilities(this, AbilitySystemComponent, CharacterClass);
	}
	

	auto HealthBarWidget = Cast<UAuraUserWidget>(HealthBar->GetUserWidgetObject());
	if (HealthBarWidget)
	{
		HealthBarWidget->SetWidgetController(this);
	}

	UAuraAttributeSet* AuraAS = CastChecked<UAuraAttributeSet>(AttributeSet);
	if (AuraAS)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)->void
			{
				OnHealthChanged.Broadcast(Data.NewValue);
			});
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetMaxHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)->void
			{
				OnMaxHealthChanged.Broadcast(Data.NewValue);
			});

		// Callbacks for GameplayTags added or removed
		AbilitySystemComponent->RegisterGameplayTagEvent(FAuraGameplayTags::GetInstance().Effects_HitReact, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::HitReactTagChanged);

		// Broadcast initValues
		OnHealthChanged.Broadcast(AuraAS->GetHealth());
		OnMaxHealthChanged.Broadcast(AuraAS->GetMaxHealth());
	}

}

void AAuraEnemy::HighlightActor()
{
	bHighlighted = true;
	GetMesh()->SetRenderCustomDepth(true);
	GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	Weapon->SetRenderCustomDepth(true);
	Weapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
}

void AAuraEnemy::UnHighlightActor()
{
	GetMesh()->SetRenderCustomDepth(false);
	Weapon->SetRenderCustomDepth(false);
	bHighlighted = false;
}

void AAuraEnemy::SetCombatTarget_Implementation(AActor* InCombatTarget)
{
	CombatTarget = InCombatTarget;
}

AActor* AAuraEnemy::GetCombatTarget_Implementation() const
{
	return CombatTarget;
}

int32 AAuraEnemy::GetPlayerLevel_Implementation() const
{
	return Level;
}

void AAuraEnemy::Die()
{
	if (HasAuthority())
	{
		AuraAIController = !IsValid(AuraAIController)? Cast<AAuraAIController>(GetController()) : AuraAIController;
		AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("IsDead"), true);
	}
	Super::Die();
}

void AAuraEnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// AI only exists on server
	if (HasAuthority())
	{
		AuraAIController = Cast<AAuraAIController>(NewController);

		check(BehaviorTree);
		AuraAIController->GetBlackboardComponent()->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
		AuraAIController->RunBehaviorTree(BehaviorTree);
		AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), false);
		AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("RangedAttacker"), CharacterClass != ECharacterClass::Warrior);
		AuraAIController->GetBlackboardComponent()->SetValueAsFloat(FName("MeleeAttackReach"), MeleeAttackReach);
	}
}

void AAuraEnemy::InitAbilityActorInfo()
{
	Super::InitAbilityActorInfo();
	AbilitySystemComponent->InitAbilityActorInfo( this, this);
	Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent)->OnAbilityActorInfoSet();

	// InitializeDefaultAttributes
	// since dependent on CharacterClassInfo, on valid on server
	if (HasAuthority())
	{
		InitializeDefaultAttributes();
	}
}

void AAuraEnemy::InitializeDefaultAttributes() const
{
	UAuraAbilitySystemLibrary::InitializeDefaultAttributes(this, CharacterClass, ICombatInterface::Execute_GetPlayerLevel(this), AbilitySystemComponent);
}
