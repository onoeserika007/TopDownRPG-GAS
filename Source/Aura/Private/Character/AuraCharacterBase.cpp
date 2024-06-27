// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AuraCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/Debuff/DebuffNiagaraComponent.h"
#include "AbilitySystem/Passive/PassiveNiagaraComponent.h"
#include "Aura/Aura.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AAuraCharacterBase::AAuraCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BurnDebuffComponent = CreateDefaultSubobject<UDebuffNiagaraComponent>(TEXT("BurnDebuffComponent"));
	BurnDebuffComponent->SetupAttachment(GetRootComponent());
	BurnDebuffComponent->DebuffTag = FAuraGameplayTags::GetInstance().Debuff_Burn;

	StunDebuffComponent = CreateDefaultSubobject<UDebuffNiagaraComponent>(TEXT("StunDebuffComponent"));
	StunDebuffComponent->SetupAttachment(GetRootComponent());
	StunDebuffComponent->DebuffTag = FAuraGameplayTags::GetInstance().Debuff_Stun;

	EffectAttachComponent = CreateDefaultSubobject<USceneComponent>(TEXT("EffectAttachComponent"));
	EffectAttachComponent->SetupAttachment(GetRootComponent());

	HaloOfProtectionNiagaraComponent = CreateDefaultSubobject<UPassiveNiagaraComponent>(TEXT("HaloOfProtectionNiagaraComponent"));
	HaloOfProtectionNiagaraComponent->SetupAttachment(EffectAttachComponent);
	HaloOfProtectionNiagaraComponent->PassiveSpellTag = FAuraGameplayTags::GetInstance().Abilities_Passive_HaloOfProtection;

	LifeSiphonNiagaraComponent = CreateDefaultSubobject<UPassiveNiagaraComponent>(TEXT("LifeSiphonNiagaraComponent"));
	LifeSiphonNiagaraComponent->SetupAttachment(EffectAttachComponent);
	LifeSiphonNiagaraComponent->PassiveSpellTag = FAuraGameplayTags::GetInstance().Abilities_Passive_LifeSiphon;

	ManaSiphonNiagaraComponent = CreateDefaultSubobject<UPassiveNiagaraComponent>(TEXT("ManaSiphonNiagaraComponent"));
	ManaSiphonNiagaraComponent->SetupAttachment(EffectAttachComponent);
	ManaSiphonNiagaraComponent->PassiveSpellTag = FAuraGameplayTags::GetInstance().Abilities_Passive_ManaSiphon;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_PROJECTILE, ECR_Overlap);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Exclude_Players, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Exclude_Players, ECR_Ignore);

	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon"));
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Weapon->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}

void AAuraCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAuraCharacterBase, bIsStunned);
	DOREPLIFETIME(AAuraCharacterBase, bIsBurned);
	DOREPLIFETIME(AAuraCharacterBase, bIsBeingShocked);
}

void AAuraCharacterBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	EffectAttachComponent->SetWorldRotation(FRotator::ZeroRotator);
}

float AAuraCharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	const float DamageTaken =  Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	OnTakeDamageDelegate.Broadcast(DamageTaken);
	return DamageTaken;
}

UAbilitySystemComponent* AAuraCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAttributeSet* AAuraCharacterBase::GetAttributeSet() const
{
	return AttributeSet;
}

// Called when the game starts or when spawned
void AAuraCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
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

void AAuraCharacterBase::Die(const FVector& DeathImpulse)
{
	/** bInCallModify Whether to call Modify() on the components concerned when detaching */
	Weapon->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
	
	SetLifeSpan(DieLifeSpan);

	MulticastHandleDeath(DeathImpulse);
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

FOnASCRegisteredSignature& AAuraCharacterBase::GetOnASCRegisteredDelegate()
{
	return OnASCRegisteredDelegate;
}

FOnActorDeathSignature& AAuraCharacterBase::GetOnActorDeathDelegate()
{
	return OnActorDeathDelegate;
}

USkeletalMeshComponent* AAuraCharacterBase::GetWeapon_Implementation()
{
	return Weapon;
}

bool AAuraCharacterBase::IsBeingShocked_Implementation()
{
	return bIsBeingShocked;
}

void AAuraCharacterBase::SetIsBeingShocked_Implementation(const bool bInIsBeingShocked)
{
	bIsBeingShocked = bInIsBeingShocked;
	GetCharacterMovement()->MaxWalkSpeed = bIsBeingShocked || bIsStunned? 0.f : BaseWalkSpeed;
}

FOnDamageSignature& AAuraCharacterBase::GetOnTakeDamageDelegate()
{
	return OnTakeDamageDelegate;
}

void AAuraCharacterBase::OnRep_Stun()
{
}

void AAuraCharacterBase::OnRep_Burn()
{
}

void AAuraCharacterBase::StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bIsStunned = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bIsStunned || bIsBeingShocked ? 0.f : BaseWalkSpeed;
}

void AAuraCharacterBase::MulticastHandleDeath_Implementation(const FVector& DeathImpulse)
{
	Weapon->SetSimulatePhysics(true);
	Weapon->SetEnableGravity(true);
	Weapon->SetCollisionEnabled(ECollisionEnabled::Type::PhysicsOnly);
	Weapon->AddImpulse(DeathImpulse * 0.1f, NAME_None, true);

	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::PhysicsOnly);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	GetMesh()->AddImpulse(DeathImpulse, NAME_None, true);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

	// Dissolve
	Dissolve();

	// dead
	bDead = true;
	BurnDebuffComponent->Deactivate();
	StunDebuffComponent->Deactivate();
	if (DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	}
	OnActorDeathDelegate.Broadcast(this);
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


