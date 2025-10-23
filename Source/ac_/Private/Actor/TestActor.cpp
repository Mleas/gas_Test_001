// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/TestActor.h"
#include "Character/Aure_Character.h"

// Sets default values
ATestActor::ATestActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	SetRootComponent(RootSceneComponent);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	Mesh->SetupAttachment(RootSceneComponent);

}

// Called when the game starts or when spawned
void ATestActor::BeginPlay()
{
	Super::BeginPlay();
	
	//����һ���������

	

	AAure_Character* TestCharactor = Cast<AAure_Character>(GetWorld()->GetFirstPlayerController()->GetPawn());

	if (TestCharactor)
	{
		TestCharactor->OnTest.BindUObject(this, &ATestActor::Test);
		TestCharactor->OnTestDyn.AddDynamic(this, &ATestActor::TestDym);
	}
}

int32 ATestActor::Test(int32 a)
{

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("OnTest. = %d"),a));
	return a;
}

void ATestActor::TestDym(int32 b)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("OnTestDym. = %d"), b));
}

// Called every frame
void ATestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

