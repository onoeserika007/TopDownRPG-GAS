﻿#include "AuraAbilityTypes.h"

UScriptStruct* FAuraGameplayEffectContext::GetScriptStruct() const
{
	return StaticStruct();
}

bool FAuraGameplayEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	uint32 RepBits = 0;
	if (Ar.IsSaving())
	{
		if (bReplicateInstigator && Instigator.IsValid())
		{
			RepBits |= 1 << 0;
		}
		if (bReplicateEffectCauser && EffectCauser.IsValid() )
		{
			RepBits |= 1 << 1;
		}
		if (AbilityCDO.IsValid())
		{
			RepBits |= 1 << 2;
		}
		if (bReplicateSourceObject && SourceObject.IsValid())
		{
			RepBits |= 1 << 3;
		}
		if (Actors.Num() > 0)
		{
			RepBits |= 1 << 4;
		}
		if (HitResult.IsValid())
		{
			RepBits |= 1 << 5;
		}
		if (bHasWorldOrigin)
		{
			RepBits |= 1 << 6;
		}
		if (bIsCriticalHit)
		{
			RepBits |= 1 << 7;
		}
		if (bIsBlockedHit)
		{
			RepBits |= 1 << 8;
		}
		if (bHasSuccessfulDebuff)
		{
			RepBits |= 1 << 9;
		}
		if (bIsDebuff)
		{
			RepBits |= 1 << 10;
		}
		if (DebuffDamage > 0.f)
		{
			RepBits |= 1 << 11;
		}
		if (DebuffDuration > 0.f)
		{
			RepBits |= 1 << 12;
		}
		if (DebuffFrequency > 0.f)
		{
			RepBits |= 1 << 13;
		}
		if (DamageType.IsValid())
		{
			RepBits |= 1 << 14;
		}
		if (!DeathImpulse.IsZero())
		{
			RepBits |= 1 << 15;
		}
		if (!KnockbackForce.IsZero())
		{
			RepBits |= 1 << 16;
		}
		if (bIsRadialDamage)
		{
			RepBits |= 1 << 17;
		}
		if (RadialDamageInnerRadius > 0.f)
		{
			RepBits |= 1 << 18;
		}
		if (RadialDamageOuterRadius > 0.f)
		{
			RepBits |= 1 << 19;
		}
		if (!RadialDamageOrigin.IsZero())
		{
			RepBits |= 1 << 20;
		}
	}

	Ar.SerializeBits(&RepBits, 20);

	if (RepBits & (1 << 0))
	{
		Ar << Instigator;
	}
	if (RepBits & (1 << 1))
	{
		Ar << EffectCauser;
	}
	if (RepBits & (1 << 2))
	{
		Ar << AbilityCDO;
	}
	if (RepBits & (1 << 3))
	{
		Ar << SourceObject;
	}
	if (RepBits & (1 << 4))
	{
		SafeNetSerializeTArray_Default<31>(Ar, Actors);
	}
	if (RepBits & (1 << 5))
	{
		if (Ar.IsLoading())
		{
			// UnSerializing, make sure that the Pointer is valid to receive the result.
			if (!HitResult.IsValid())
			{
				HitResult = TSharedPtr<FHitResult>(new FHitResult());
			}
		}
		HitResult->NetSerialize(Ar, Map, bOutSuccess);
	}
	if (RepBits & (1 << 6))
	{
		Ar << WorldOrigin;
		bHasWorldOrigin = true;
	}
	else
	{
		bHasWorldOrigin = false;
	}
	if (RepBits & (1 << 7))
	{
		Ar << bIsCriticalHit;
	}
	if (RepBits & (1 << 8))
	{
		Ar << bIsBlockedHit;
	}
	if (RepBits & (1 << 9))
	{
		Ar << bHasSuccessfulDebuff;
	}
	if (RepBits & (1 << 10))
	{
		Ar << bIsDebuff;
	}
	if (RepBits & (1 << 11))
	{
		Ar << DebuffDamage;
	}
	if (RepBits & (1 << 12))
	{
		Ar << DebuffDuration;
	}
	if (RepBits & (1 << 13))
	{
		Ar << DebuffFrequency;
	}
	if (RepBits & (1 << 14))
	{
		// So why use shared pointer here???
		if (Ar.IsLoading())
		{
			if (!DamageType.IsValid())
			{
				DamageType = MakeShared<FGameplayTag>();
			}
		}
		DamageType->NetSerialize(Ar, Map, bOutSuccess);
	}
	if (RepBits & (1 << 15))
	{
		DeathImpulse.NetSerialize(Ar, Map, bOutSuccess);
	}
	if (RepBits & (1 << 16))
	{
		KnockbackForce.NetSerialize(Ar, Map, bOutSuccess);
	}
	if (RepBits & (1 << 17))
	{
		Ar << bIsRadialDamage;
	}
	if (RepBits & (1 << 18))
	{
		Ar << RadialDamageInnerRadius;
	}
	if (RepBits & (1 << 19))
	{
		Ar << RadialDamageOuterRadius;
	}
	if (RepBits & (1 << 20))
	{
		RadialDamageOrigin.NetSerialize(Ar, Map, bOutSuccess);
	}
	

	if (Ar.IsLoading())
	{
		AddInstigator(Instigator.Get(), EffectCauser.Get()); // Just to initialize InstigatorAbilitySystemComponent
	}	
	
	bOutSuccess = true;
	return true;
}

FAuraGameplayEffectContext* FAuraGameplayEffectContext::Duplicate() const
{
	FAuraGameplayEffectContext* NewContext = new FAuraGameplayEffectContext();
	*NewContext = *this;
	if (GetHitResult())
	{
		// Dose a deep copy of the hit result
		NewContext->AddHitResult(*GetHitResult(), true);
	}
	if (DamageType.IsValid())
	{
		NewContext->DamageType = MakeShared<FGameplayTag>(*DamageType);
	}
	return NewContext;
}
