// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AuraCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Aura/Aura.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AAuraCharacterBase::AAuraCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_PROJECTILE, ECR_Overlap);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon"));
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Weapon->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}

UAbilitySystemComponent* AAuraCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

// Called when the game starts or when spawned
void AAuraCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AAuraCharacterBase::InitAbilityActorInfo()
{
}

FVector AAuraCharacterBase::GetCombatSocketLocation_Implementation(const FGameplayTag& MontageTag) const
{
	if (MontageTag.MatchesTagExact(FAuraGameplayTags::GetInstance().CombatSocket_Weapon))
	{
		if (IsValid(Weapon))
		{
			// UKismetSystemLibrary::DrawDebugSphere(
			// 	this,
			// 	Weapon->GetSocketLocation(WeaponTipSocketName),
			// 	10,
			// 	12,
			// 	FLinearColor::Yellow,
			// 	10
			// );
			return Weapon->GetSocketLocation(WeaponTipSocketName);
		}
	}

	if (MontageTag.MatchesTagExact(FAuraGameplayTags::GetInstance().CombatSocket_Tail))
	{
		return GetMesh()->GetSocketLocation(TailSocketName);
	}

	if (MontageTag.MatchesTagExact(FAuraGameplayTags::GetInstance().CombatSocket_LeftHand))
	{
		return GetMesh()->GetSocketLocation(LeftHandSocketName);
	}

	if (MontageTag.MatchesTagExact(FAuraGameplayTags::GetInstance().CombatSocket_RightHand))
	{
		return GetMesh()->GetSocketLocation(RightHandSocketName);
	}

	return {};
	
}

UAnimMontage* AAuraCharacterBase::GetHitReactMontage_Implementation()
{
	return HitReactMontage;
}

TArray<FTaggedMontage> AAuraCharacterBase::GetAttackMontage_Implementation()
{
	return AttackMontage;
}

void AAuraCharacterBase::Die()
{
	/** bInCallModify Whether to call Modify() on the components concerned when detaching */
	Weapon->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
	
	SetLifeSpan(DieLifeSpan);

	MulticastHandleDeath();
}

bool AAuraCharacterBase::IsDead_Implementation() const
{
	return bDead;
}

AActor* AAuraCharacterBase::GetAvatar_Implementation()
{
	return this;
}

UNiagaraSystem* AAuraCharacterBase::GetBloodEffect_Implementation()
{
	return BloodEffect;
}

FTaggedMontage AAuraCharacterBase::GetTaggedMontageByTag_Implementation(const FGameplayTag& MontageTag)
{
	for (const auto& TaggedMontage: AttackMontage)
	{
		if (TaggedMontage.MontageTag.MatchesTagExact(MontageTag))
		{
			return TaggedMontage;
		}
	}
	return {};
}

int32 AAuraCharacterBase::GetMinionCount_Implementation()
{
	return MinionCount;
}

void AAuraCharacterBase::IncrementMinionCount_Implementation(int Amount)
{
	MinionCount += Amount;
}

ECharacterClass AAuraCharacterBase::GetCharacterClass_Implementation()
{
	return CharacterClass;
}

void AAuraCharacterBase::MulticastHandleDeath_Implementation()
{
	Weapon->SetSimulatePhysics(true);
	Weapon->SetEnableGravity(true);
	Weapon->SetCollisionEnabled(ECollisionEnabled::Type::PhysicsOnly);

	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::PhysicsOnly);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

	// Dissolve
	Dissolve();

	// dead
	bDead = true;
	if (DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	}
}

void AAuraCharacterBase::ApplyEffectToSelf(const TSubclassOf<UGameplayEffect> GameplayEffectClass, const float Level) const
{
	if (GetAbilitySystemComponent() && GameplayEffectClass)
	{
		auto ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
		// important!!!
		ContextHandle.AddSourceObject(this);
		const FGameplayEffectSpecHandle EffectSpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(GameplayEffectClass, Level, ContextHandle);
		GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data, GetAbilitySystemComponent());
		// GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data);
	}
}

void AAuraCharacterBase::InitializeDefaultAttributes() const
{
	ApplyEffectToSelf(DefaultPrimaryAttributesEffect, 1.f);
	ApplyEffectToSelf(DefaultSecondaryAttributesEffect, 1.f);
	ApplyEffectToSelf(DefaultVitalAttributesEffect, 1.f);
}

void AAuraCharacterBase::AddCharacterAbilities()
{
	if (HasAuthority())
	{
		// If you drag a AuraCharacter into the map, it may be controlled by AIController, then the Initialization of GAS may be a mess.
		UAuraAbilitySystemComponent* AuraASC = CastChecked<UAuraAbilitySystemComponent>(AbilitySystemComponent);
		AuraASC->AddCharacterAbilities(StartupAbilities);
		AuraASC->AddCharacterPassiveAbilities(StartupPassiveAbilities);
	}
}

void AAuraCharacterBase::Dissolve()
{
	if (DissolveMaterialInstance)
	{
		UMaterialInstanceDynamic* DynamicMatInst = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicMatInst);
		StartDissolveTimeline(DynamicMatInst);
	}

	if (WeaponDissolveMaterialInstance)
	{
		UMaterialInstanceDynamic* DynamicMatInst = UMaterialInstanceDynamic::Create(WeaponDissolveMaterialInstance, this);
		Weapon->SetMaterial(0, DynamicMatInst);
		StartWeaponDissolveTimeline(DynamicMatInst);
	}
}


