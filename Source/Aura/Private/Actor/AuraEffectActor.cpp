// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/AuraEffectActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "ActiveGameplayEffectHandle.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values
AAuraEffectActor::AAuraEffectActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>("SceneRoot");
}

// Called when the game starts or when spawned
void AAuraEffectActor::BeginPlay()
{
	Super::BeginPlay();

	InitialLoaction = GetActorLocation();
	CalculatedLocation = InitialLoaction;
	CalculatedRotation = GetActorRotation();
}

void AAuraEffectActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	RunningTime += DeltaSeconds;
	if (RunningTime > SinePeriod)
	{
		RunningTime -= SinePeriod;
	}
	ItemMovement(DeltaSeconds);
}

void AAuraEffectActor::ItemMovement(float DeltaTime)
{
	if (bRotates)
	{
		const FRotator DeltaRotation(0.f, DeltaTime * RotationRate, 0.f);
		CalculatedRotation = UKismetMathLibrary::ComposeRotators(CalculatedRotation, DeltaRotation);
	}

	if (bSinusoidalMovement)
	{
		const float Sine = SineAmplitude * FMath::Sin(SinePeriod);
		CalculatedLocation = InitialLoaction + FVector(0.f, 0.f, Sine);
	}
}

void AAuraEffectActor::StartSinusoidalMovement()
{
	bSinusoidalMovement = true;
	InitialLoaction = GetActorLocation();
	CalculatedLocation = InitialLoaction;
}

void AAuraEffectActor::StartRotation()
{
	bRotates = true;
	CalculatedRotation = GetActorRotation();
}

void AAuraEffectActor::ApplyEffectToTarget(AActor* EffectTarget, const TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	// If overlap with enemy
	if (EffectTarget->ActorHasTag(FName("Enemy")) && !bApplyEffectsToEnemies) return;
	
	if (const TScriptInterface<IAbilitySystemInterface> ASCInterface = EffectTarget)
	{
		if (const auto TargetASC = ASCInterface->GetAbilitySystemComponent())
		{
			check(GameplayEffectClass);
			auto EffectContextHandle = TargetASC->MakeEffectContext();
			EffectContextHandle.AddSourceObject(this);
			const FGameplayEffectSpecHandle EffectSpecHandle = TargetASC->MakeOutgoingSpec(GameplayEffectClass, ActorLevel, EffectContextHandle);
			FActiveGameplayEffectHandle ActiveEffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data);

			const bool bIsInfinite = EffectSpecHandle.Data->Def->DurationPolicy == EGameplayEffectDurationType::Infinite;
			if (bIsInfinite && InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
			{
				ActiveEffectHandles.Add(ActiveEffectHandle, TargetASC);
			}

			if (bDestroyOnEffectApplication && !bIsInfinite)
			{
				Destroy();
			}
		}
	}
}

void AAuraEffectActor::OnOverlap(AActor* EffectTarget)
{
	if (InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(EffectTarget, InstantGameplayEffectClass);
	}

	if (DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(EffectTarget, DurationGameplayEffectClass);
	}

	if (InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(EffectTarget, InfiniteGameplayEffectClass);
	}
}

void AAuraEffectActor::EndOverlap(AActor* EffectTarget)
{
	// Application
	if (InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(EffectTarget, InstantGameplayEffectClass);
	}

	if (DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(EffectTarget, DurationGameplayEffectClass);
	}

	if (InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(EffectTarget, InfiniteGameplayEffectClass);
	}

	// Removal
	if (InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(EffectTarget);
		if (TargetASC)
		{
			TArray<FActiveGameplayEffectHandle> HandlesToRemove;
			for (auto& [Handle, PairASC]: ActiveEffectHandles)
			{
				if (TargetASC == PairASC)
				{
					TargetASC->RemoveActiveGameplayEffect(Handle, 1);
					HandlesToRemove.AddUnique(Handle);
				}
			}
			

			for (auto& Handle: HandlesToRemove)
			{
				ActiveEffectHandles.FindAndRemoveChecked(Handle);
			}
		}
		
	}
}



