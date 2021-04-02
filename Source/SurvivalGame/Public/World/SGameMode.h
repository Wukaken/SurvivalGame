// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Mutators/SMutator.h"
#include "SGameMode.generated.h"

class ASPlayerState;
class APlayerStart;
/**
 * 
 */
UCLASS()
class SURVIVALGAME_API ASGameMode : public AGameMode
{
	GENERATED_BODY()
protected:
	ASGameMode();

	virtual void PreInitializeComponents() override;

	virtual void InitGameState();

	virtual void DefaultTimer();

	virtual void StartMatch();

	virtual void OnNightEnded();

	virtual void SpawnDefaultInventory(APawn* PlayerPawn);

	// Make sure pawn properties are back to default also a good place to modify them on spawn
	virtual void SetPlayerDefaults(APawn* PlayerPawn) override;

	// handle for efficient management of defaultTimer timer
	FTimerHandle TimerHandle_DefaultTimer;

	// Van we deal damage to players in the same team
	UPROPERTY(EditDefaultsOnly, Category = "Rules")
	bool bAllowFriendlyFireDamage;

	// allow zombie spawns to be disabled(for debugging)
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bSpawnZombiesAtNight;

	// limit the amount of zombies to have at one point in the world (includes players)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	int32 MaxPawnsInZone;

	float BotSpawnInterval;

	// called once on every new player that enters the gamemode
	virtual FString InitNewPlayer(class APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal = TEXT("")) override;

	// the teamnumber assigned to Players
	int32 PlayerTeamNum;

	// keep reference to the night state of the previous frame
	bool LastIsNight;

	// the start time for the gamemode
	int32 TimeOfDayStart;

	// the enemy pawn class
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TSubclassOf<class APawn> BotPawnClass;

	// Handle for nightly bot spawning
	FTimerHandle TimerHandle_BotSpawns;

	// handles bot spawning (during nighttime)
	void SpawnBotHandler();

	// Player Spawning
	// don't allow spectating of bots
	virtual bool CanSpectate_Implementation(APlayerController* Viewer, APlayerState* ViewTarget) override;

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	// Always pick a random location
	virtual bool ShouldSpawnAtStartSpot(AController* Player) override;

	virtual bool IsSpawnPointAllowed(APlayerStart* SpawnPoint, AController* Controller);

	virtual bool IsSpawnPointPreferred(APlayerStart* SpawnPoint, AController* Controller);

	// return default pawn class for given controller
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
public:
	// Damage and killing
	virtual void Killed(AController* Killer, AController* VictimPlayer, APawn* VictimPawn, const UDamageType* DamageType);

	// can the player deal damage according to gamemode rules(eg. friendly-fire disabled)
	virtual bool CanDealDamage(class ASPlayerState* DamageCauser, class ASPlayerState* DamagedPlayer) const;

	virtual bool ModifyDamage(float Damage, AActor* DamagedActor, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const;
protected:
	// bots
	// (exec only valid when testing in singlePlayer)
	UFUNCTION(BluerpintCallable, Exec, Category = "GameMode")
	void SpawnNewBot();

	// Blueprint hook to find a good spawn location for bots(eg. via EQS queries)
	UFUNCTION(BluerpintImplementableEvent, Category = "GameMode")
	bool FindBotSpawnTransform(FTransform& Transform);

	// Set all bots back to idle mode
	void PassifyAllBots();

	// set all bots to active patrolling state
	void WakeAllBots();
public:
	//Primary sun of the level, assigned in blueprint during BeginPlay(BlueprintReadWrite is required as tag instead of EditDefaultsOnly)
	UPROPERTY(BlueprintReadWrite, Category = "DayNight")	
	class ADirectionalLight* PrimarySunLight;

	// the default weapons to spawn with
	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TArray<TSubclassOf<class ASWeapon>> DefaultInventoryClasses;
protected:
	// modding & mutator
	UPROPERTY(EditAnywhere, Category = "Mutators")
	TArray<TSubclassOf<class ASMutator>> MutatorClasses;

	// first mutator in the execution chain
	ASMutator* BaseMutator;

	void AddMutator(TSubclassOf<ASMutator> MutClass);

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	// from ut source: used to modify, remove, and replace actors, return false to destroy the passed in actor. default implementation queries mutators
	// note that certain critical actors such as playercontrollers can't be destroyed, but we'll still call this code path to allow mutators
	// to change properties on them
	UFUNCTION(BlueprintNativeEvent)
	bool CheckRelevance(AActor* Other);

	// note: functions flagged with BlueprintNativeEvent like above require _Implementation for a c++ implementation
	virtual bool CheckRelevance_Implementation(AActor* Other);
};
