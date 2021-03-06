// Fill out your copyright notice in the Description page of Project Settings.

#include "TankAIController.h"
#include "TankPlayerController.h"
#include "TankAimingComponent.h"
#include "Tank.h"


void ATankAIController::BeginPlay()
{
    Super::BeginPlay();    
}

void ATankAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UTankAimingComponent *AimingComponent = GetPawn()->FindComponentByClass<UTankAimingComponent>();

    APawn *PlayerTank = GetWorld()->GetFirstPlayerController()->GetPawn();
    
    if (!ensure (PlayerTank && AimingComponent)) { return; }

    // Move towards the player
    MoveToActor(PlayerTank, AcceptanceRadius);      // TODO check radius is in cm
        
    if (!ensure(AimingComponent)) return;

    // Aim towards the player
    AimingComponent->AimAt(PlayerTank->GetActorLocation());
    
    if (AimingComponent->GetFiringState() == EFiringState::Locked)
    {
        AimingComponent->Fire(); // TODO limit firing rate
    }
}

void ATankAIController::SetPawn(APawn* InPawn)
{
    Super::SetPawn(InPawn);
    
    if (InPawn)
    {
        auto PossessedTank = Cast<ATank>(InPawn);
        if (!ensure(PossessedTank)) { return; }
        
        // Subscribe our local method to the tank's death event
        // *** Subscribe here instead of "BeginPlay()" is because to make sure there is a possessedTank ready to subscribe to the Death event
        PossessedTank->OnDeath.AddUniqueDynamic(this, &ATankAIController::TankDeath);
        
    }
}

void ATankAIController::TankDeath()
{
    if (!GetPawn()) { return; }
    
    GetPawn()->DetachFromControllerPendingDestroy();
}
