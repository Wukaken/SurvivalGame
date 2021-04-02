// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SBaseCharacter.h"
#include "UObject/UObjectGlobals.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "World/SGameMode.h"

// Sets default values
ASBaseCharacter::ASBaseCharacter(const class FObjectInitializer& ObjectInitializer) 
	// Override the movement class from the base class to our own to support multiple speeds (eg. sprinting)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	Health = 100;
	TargetingSpeedModifier = 0.5f;
	SprintingSpeedModifier = 2.0f;

	// Noise emitter for both players and enemies. This keeps track of MakeNoise data and is used by the pawnsensing component in our SZombieCharacter class
	NoiseEmitterComp = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("NoiseEmitterComp"));
	// Don't collide with camera checkes to keep 3rd person camera at position when zombies or other players are standing behind us
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}

float ASBaseCharacter::GetHealth() const
{
	return Health;
}

float ASBaseCharacter::GetMaxHealth() const
{
	//Retrieve the default value of the health property that is assigned on instantiation
	return GetClass()->GetDefaultObject<ASBaseCharacter>()->Health;
}

bool ASBaseCharacter::IsAlive() const
{
	return Health > 0;
}

float ASBaseCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser)
{
	if(Health < 0.f)
		return 0.f;

	// Modify based on gametype rules
	ASGameMode* MyGameMode = Cast<ASGameMode>(GetWorld()->GetAuthGameMode());
	Damage = MyGameMode ? MyGameMode->ModifyDamage(Damage, this, DamageEvent, EventInstigator, DamageCauser) : Damage;

	const float ActualDamage = Super::TakeDamage(Damage, this, DamageEvent, EventInstigator, DamageCauser);
	if(ActualDamage > 0.f){
		Health -= ActualDamage;
		if(Health <= 0){
			bool bCanDie = true;
			// Check the damageType, always allow dying if the cast failed, otherwise check the property if player can die from damagetype
			if(DamageEvent.DamageTypeClass){
				USDamageType* DmgType = Cast<UDamageType>(DamageEvent.DamageTypeClass->GetDefaultObject());
				bCanDie = (DmgType == nullptr || (DmgType && DmgType->GetCanDieFrom()));
			}

			if(bCanDie)
				Die(ActualDamage, DamageEvent, EventInstigator, DamageCauser);
			else
				Health = 1.f;
		}
		else{
			// shorthand for if x != null, pick1 else pick2
			APawn* Pawn = EventInstigator ? EventInstigator->GetPawn() : nullptr;
			PlayHit(ActualDamage, DamageEvent, Pawn, DamageCauser, false);
		}
	}
	return ActualDamage;
}

bool ASBaseCharacter::CanDie(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser) const
{
	if(bIsDyning || IsPendingKill() || !HasAuthority() || GetWorld()->GetAuthGameMode() == NULL)
		return false;
	return true;
}

void ASBaseCharacter::FellOutOfWorld(const class UDamageType& DmgType)
{
	Die(Health, FDamageEvent(DmgType.GetClass()), NULL, NULL);
}

bool ASBaseCharacter::Die(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser)
{
	if(!CanDie(KillingDamage, DamageEvent, Killer, DamageCauser))
		return false;

	Health = FMath::Min(0.f, Health);

	// Fallback to default DamageType if none is specified
	UDamageType const* const DamageType = DamageEvent.DamageTypeClass ? DamageEvent.DamageTypeClass->GetDefaultObject<UDamageType>() : GetDefault<UDamageType>();
	Killer = GetDamageInstigator(Killer, *DamageType);

	// notify the gamemode we got killed for scoring and game over state
	AController* KilledPlayer = Controller ? Controller : Cast<AController>(GetOwner());
	GetWorld()->GetAuthGameMode<ASGameMode>()->Killed(Killer, KilledPlayer, this, DamageType);
}
// Called to bind functionality to input
void ASBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

