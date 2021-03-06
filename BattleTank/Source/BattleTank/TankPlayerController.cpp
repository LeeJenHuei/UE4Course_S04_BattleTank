// Fill out your copyright notice in the Description page of Project Settings.

#include "TankPlayerController.h"
#include "TankAimingComponent.h"
#include "Tank.h"

void ATankPlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    auto AutoComponent = GetPawn()->FindComponentByClass<UTankAimingComponent>();
    if (AutoComponent)
    {
        FoundAimingComponent(AutoComponent);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("No TankAimingComponent"))
    }
}

void ATankPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    AimTowardsCrosshair();
}

void ATankPlayerController::AimTowardsCrosshair()
{
    if (GetPawn())  // if not posessing
    {
        UTankAimingComponent *AimingComponent = GetPawn()->FindComponentByClass<UTankAimingComponent>();
        if (!ensure(AimingComponent)) { return; }
    
        FVector HitLocation; // Out parameter
    
        if (GetSightRayHitLocation(HitLocation))
        {
            AimingComponent->AimAt(HitLocation);
        }
    }
}

// Get world location if linetrace through crosshair, true if hits landscape
bool ATankPlayerController::GetSightRayHitLocation(FVector& outHitLocation) const
{
    // Find the crosshair location
    int32 ViewportSizeX, ViewportSizeY;
    GetViewportSize(ViewportSizeX, ViewportSizeY);
    
    FVector2D ScreenLocation = FVector2D(ViewportSizeX * CrossHairXLocation, ViewportSizeY * CrossHairYLocation);
    
    FVector LookDirection;
    if (GetLookDirection(ScreenLocation, LookDirection))
    {
        // Line-trace along that look direction, and see what we hit (up to a max range)
        return GetLookVectorHitLocation(LookDirection, outHitLocation);
    }
    return false;

}

// De-project the screen position of the crosshair to a world direction
bool ATankPlayerController::GetLookDirection(FVector2D ScreenLocation, FVector& LookDirection) const
{
    FVector CameraWorldLocation;    // To be discarded
    return DeprojectScreenPositionToWorld(ScreenLocation.X, ScreenLocation.Y, CameraWorldLocation, LookDirection);
}

bool ATankPlayerController::GetLookVectorHitLocation(FVector LookDirection, FVector &outHitLocation) const
{
    FHitResult HitResult;
    
    auto StartLocation = PlayerCameraManager->GetCameraLocation();
    auto EndLocation = StartLocation + LookDirection * LineTraceRange;
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECollisionChannel::ECC_Camera))
    {
        outHitLocation = HitResult.Location;
        return true;
    }
    return false;
}

void ATankPlayerController::TankDeath()
{
    if (!GetPawn()) { return; }
    
    StartSpectatingOnly();
}

void ATankPlayerController::SetPawn(APawn* InPawn)
{
    Super::SetPawn(InPawn);
    
    if (InPawn)
    {
        auto PossessedTank = Cast<ATank>(InPawn);
        if (!ensure(PossessedTank)) { return; }
        
        // Subscribe our local method to the tank's death event
        // *** Subscribe here instead of "BeginPlay()" is because to make sure there is a possessedTank ready to subscribe to the Death event
        PossessedTank->OnDeath.AddUniqueDynamic(this, &ATankPlayerController::TankDeath);
        
    }
}
