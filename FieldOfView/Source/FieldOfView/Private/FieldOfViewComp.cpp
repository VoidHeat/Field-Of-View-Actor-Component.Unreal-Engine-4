// Fill out your copyright notice in the Description page of Project Settings.


#include "FieldOfViewComp.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "ProceduralMeshComponent.h"

UFieldOfViewComp::UFieldOfViewComp()
{
	PrimaryComponentTick.bCanEverTick = true;

	viewAngle = 120;
	amountOfTraces = 120;
	viewDistance = 1000.f;
	resolutionOfEdgeDetection = 5;
	edgeDistThreshold = 100.f;
	locationOffset = FVector::ZeroVector;
	rotationYawOffset = 0.f;

	CalculateData();

	//debug start
	debugColor = FColor::Emerald;
	lineThickness = 2.f;
	bEnableDebug = false;
	bEdgeDebug = true;
	//debug end

	procMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
}

void UFieldOfViewComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	vertices.Empty();
	FVector startLocation = GetOwnerLocation() + locationOffset;
	if (procMesh)
	{
		procMesh->SetWorldLocation(FVector::ZeroVector);
		procMesh->SetWorldRotation(FRotator::ZeroRotator);
	}
	if (bEnableDebug)
	{
		DrawDebugSphere(GetWorld(), startLocation, viewDistance, 24, debugColor, false, -1, 0, lineThickness);
	}
	vertices.Add(startLocation);
	for (int i = 0; i < amountOfTraces; i++)
	{
		float angleOfTrace = viewAngle / 2 + GetOwnerRotation().Yaw + rotationYawOffset - angleBetweenTraces * i;
		FHitResult Hit = Trace(startLocation, startLocation + DirFromAngle(angleOfTrace) * viewDistance);
		FViewCastRes hit;
		hit = Hit;
		hit.traceAngle = angleOfTrace;
		if (i > 0)
		{
			bool edgeDistThresholdSuccess = FMath::Abs(oldRay.dist - hit.dist) > edgeDistThreshold;
			if ((oldRay.bBlockSomething != Hit.bBlockingHit) || (oldRay.bBlockSomething && Hit.bBlockingHit && edgeDistThresholdSuccess))
			{
				FVector maxPoint;
				FVector minPoint;
				FindEdge(oldRay, hit, minPoint, maxPoint);
				if (minPoint != FVector::ZeroVector)
				{
					vertices.Add(minPoint );
					if (bEnableDebug && bEdgeDebug)
					{
						DrawDebugLine(GetWorld(), startLocation, minPoint, FColor::Red, false, -1, 0, lineThickness);
					}
				}
				if (maxPoint != FVector::ZeroVector)
				{
					vertices.Add(maxPoint );
					if (bEnableDebug && bEdgeDebug)
					{
						DrawDebugLine(GetWorld(), startLocation, maxPoint, FColor::Blue, false, -1, 0, lineThickness);
					}
				}
			}
		}
		if (Hit.bBlockingHit)
		{
			if (bEnableDebug)
			{
				DrawDebugLine(GetWorld(), startLocation, Hit.ImpactPoint, debugColor, false, -1, 0, lineThickness);
				DrawDebugBox(GetWorld(), Hit.ImpactPoint, FVector(5, 5, 5), debugColor);
			}
			vertices.Add(Hit.ImpactPoint );
		}
		else
		{
			if (bEnableDebug)
			{
				DrawDebugLine(GetWorld(), startLocation, Hit.TraceEnd, debugColor, false, -1, 0, lineThickness);
			}
			vertices.Add(Hit.TraceEnd );
		}
		oldRay = hit;
	}
	CreateMesh();
}

void UFieldOfViewComp::CalculateData()
{
	angleBetweenTraces = viewAngle / (amountOfTraces - 1);
}

void UFieldOfViewComp::BeginPlay()
{
	Super::BeginPlay();
	CalculateData();
	AssignMaterial();
	procMesh->CastShadow = false;
	procMesh->SetRenderCustomDepth(true);
}

void UFieldOfViewComp::FindEdge(const FViewCastRes& minRay, const FViewCastRes& maxRay, FVector& outMinPoint, FVector& outMaxPoint)
{
	float minAngle = minRay.traceAngle;
	float maxAngle = maxRay.traceAngle;
	FVector minPoint = FVector::ZeroVector;
	FVector maxPoint = FVector::ZeroVector;

	for (int i = 0; i < resolutionOfEdgeDetection; i++)
	{
		float angle = (minAngle + maxAngle) / 2;
		FVector startLocation = GetOwnerLocation() + locationOffset;
		FViewCastRes newViewCast;
		FHitResult Hit = Trace(startLocation, startLocation + DirFromAngle(angle) * viewDistance);
		newViewCast = Hit;

		bool edgeDistThresholdSuccess = FMath::Abs(minRay.dist - newViewCast.dist) > edgeDistThreshold;
		if (newViewCast.bBlockSomething == minRay.bBlockSomething && !edgeDistThresholdSuccess )
		{
			minAngle = angle;
			minPoint = newViewCast.point;
		}
		else
		{
			maxAngle = angle;
			maxPoint = newViewCast.point;
		}
		
	}
	outMinPoint = minPoint;
	outMaxPoint = maxPoint;
}
void UFieldOfViewComp::CreateMesh()
{
	TArray<int32> Triangles;
	Triangles.SetNum((vertices.Num() - 2) * 3);
	for (int i = 0; i < vertices.Num() - 2; i++)
	{
		Triangles[i * 3] = 0;
		Triangles[i * 3 + 1] = i + 1;
		Triangles[i * 3 + 2] = i + 2;
	}
	TArray<FVector> normals;
	TArray<FVector2D> UV0;
	TArray<FProcMeshTangent> tangents;
	TArray<FLinearColor> vertexColors;
	procMesh->CreateMeshSection_LinearColor(0, vertices, Triangles, normals, UV0, vertexColors, tangents, false);
}

FHitResult UFieldOfViewComp::Trace(const FVector& StartTrace, const FVector& EndTrace)
{
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this->GetOwner());
	GetWorld()->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_Visibility, QueryParams);
	return HitResult;
}
void UFieldOfViewComp::AssignMaterial(UMaterialInterface* Material)
{
	if (Material)
	{
		procMesh->SetMaterial(0, Material);
	}
	else if(defaultMaterial)
	{
		procMesh->SetMaterial(0, defaultMaterial);
	}
}

#if WITH_EDITOR
void UFieldOfViewComp::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	CalculateData();
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif
