// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/PointCollection.h"

#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

#define CONCATENATE(x, y) x##y
#define TO_STRING(x) #x

#define INIT_POINT_COMPONENTS(Point) \
	Point = CreateDefaultSubobject<USceneComponent>(#Point);\
	Point->SetupAttachment(GetRootComponent());\
	ImmutablePoints.Add(Point);


APointCollection::APointCollection()
{
	PrimaryActorTick.bCanEverTick = false;

	Pt_0 = CreateDefaultSubobject<USceneComponent>("Pt_0");
	SetRootComponent(Pt_0);
	ImmutablePoints.Add(Pt_0);

	INIT_POINT_COMPONENTS(Pt_1);
	INIT_POINT_COMPONENTS(Pt_2);
	INIT_POINT_COMPONENTS(Pt_3);
	INIT_POINT_COMPONENTS(Pt_4);
	INIT_POINT_COMPONENTS(Pt_5);
	INIT_POINT_COMPONENTS(Pt_6);
	INIT_POINT_COMPONENTS(Pt_7);
	INIT_POINT_COMPONENTS(Pt_8);
	INIT_POINT_COMPONENTS(Pt_9);
	INIT_POINT_COMPONENTS(Pt_10);
}

void APointCollection::BeginPlay()
{
	Super::BeginPlay();
	
}

void APointCollection::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

TArray<USceneComponent*> APointCollection::GetGroundPoints(const FVector& GroundLocation, int32 NumPoints,
	float YawOverride)
{
	checkf(ImmutablePoints.Num() >= NumPoints, TEXT("Attempted to access Immutable out of bound."));

	TArray<USceneComponent*> ArrayCopy;
	for (USceneComponent* Pt: ImmutablePoints)
	{
		if (ArrayCopy.Num() >= NumPoints) return ArrayCopy;

		if (Pt != GetRootComponent())
		{
			FVector ToPoint = Pt->GetComponentLocation() - GetRootComponent()->GetComponentLocation();
			ToPoint = ToPoint.RotateAngleAxis(YawOverride, FVector::UpVector);
			Pt->SetWorldLocation(GetRootComponent()->GetComponentLocation() + ToPoint);
		}

		const FVector RaisedLocation = {Pt->GetComponentLocation().X, Pt->GetComponentLocation().Y, Pt->GetComponentLocation().Z + 500.f};
		const FVector LoweredLocation = {Pt->GetComponentLocation().X, Pt->GetComponentLocation().Y, Pt->GetComponentLocation().Z - 500.f};

		FHitResult HitResult;
		TArray<AActor*> IgnoreActors;
		UAuraAbilitySystemLibrary::GetLivePlayersWithinRadius(this, IgnoreActors, {}, 1500.f, GetActorLocation());

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActors(IgnoreActors);
		GetWorld()->LineTraceSingleByProfile(HitResult, RaisedLocation, LoweredLocation, FName("BlockAll"), QueryParams);

		const FVector& AdjustedLocation = HitResult.ImpactPoint;
		Pt->SetWorldLocation(AdjustedLocation);
		// Set Normal as local Z Axis
		Pt->SetWorldRotation(UKismetMathLibrary::MakeRotFromZ(HitResult.ImpactNormal));
		ArrayCopy.Add(Pt);
	}
	return ArrayCopy;
}

