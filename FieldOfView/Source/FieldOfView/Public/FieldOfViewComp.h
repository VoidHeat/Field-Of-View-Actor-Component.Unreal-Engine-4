// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FieldOfViewComp.generated.h"

USTRUCT(BlueprintType)
struct FViewCastRes
{
	GENERATED_BODY()

	bool bBlockSomething;
	float dist;
	FVector point;
	float traceAngle;

	FViewCastRes(bool bBlockSomething_ = 0, float dist_ = 0, float traceAngle_ = 0, FVector point_ = FVector::ZeroVector)
		:bBlockSomething(bBlockSomething_), dist(dist_), traceAngle(traceAngle_), point(point_) {}
	FViewCastRes& operator= (const FViewCastRes& other)
	{
		bBlockSomething = other.bBlockSomething;
		dist = other.dist;
		point = other.point;
		traceAngle = other.traceAngle;
		return *this;
	}

	FViewCastRes& operator= (const FHitResult& other)
	{
		bBlockSomething = other.bBlockingHit;
		if (bBlockSomething)
		{
			point = other.ImpactPoint;
		}
		else
		{
			point = other.TraceEnd;
		}
		dist = other.Distance;
		return *this;
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FIELDOFVIEW_API UFieldOfViewComp : public UActorComponent
{
	GENERATED_BODY()

public:	
	UFieldOfViewComp();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void FindEdge(const FViewCastRes& minRay, const FViewCastRes& maxRay, FVector& outMinPoint, FVector& outMaxPoint);

	void CreateMesh();

	FHitResult Trace(const FVector& StartTrace, const FVector& EndTrace);

	FVector DirFromAngle(float angleInDegrees) {
		return FVector(FMath::Cos(FMath::DegreesToRadians(angleInDegrees)), FMath::Sin(FMath::DegreesToRadians(angleInDegrees)), 0);
	}

	UFUNCTION(BlueprintCallable)
	void AssignMaterial(class UMaterialInterface* Material = nullptr);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "360.0"))
	float viewAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "2"))
		int amountOfTraces;

protected:
	UPROPERTY(VisibleAnywhere)
	float angleBetweenTraces;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
	float viewDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector locationOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "360.0"))
	float rotationYawOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	int resolutionOfEdgeDetection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
	float edgeDistThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UMaterialInterface* defaultMaterial;

	UPROPERTY(VisibleAnywhere)
	class UProceduralMeshComponent* procMesh;

	//debug start
	UPROPERTY(EditAnywhere)
	bool bEnableDebug;
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bEnableDebug"))
	FColor debugColor;
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bEnableDebug"))
	float lineThickness;
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bEnableDebug"))
	bool bEdgeDebug;
	//debug end

protected:

	virtual void BeginPlay() override;

	void CalculateData();

	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	inline FVector GetOwnerLocation() { return  GetOwner()->GetActorLocation(); };
	inline FRotator GetOwnerRotation() { return GetOwner()->GetActorRotation(); };

	FViewCastRes oldRay;

	TArray<FVector> vertices;
};
