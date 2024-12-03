// Can Gures, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "WarriorBaseCharacter.generated.h"


class  UWarriorAbilitySystemComponent;
class UWarriorAttributeSet;
class UDataAsset_StartUpDatabase;
UCLASS()
class TEREK_API AWarriorBaseCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AWarriorBaseCharacter();
	//~ Begin IAbilitySystemInterface.
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const;
	//~ End IAbilitySystemInterface.

protected:
	//~ Begin APawn Interface.
	virtual void PossessedBy(AController* NewController) override;
	//~ End APawn Interface
	 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	 UWarriorAbilitySystemComponent* WarriorAbilitySystemComponent;

	 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	 UWarriorAttributeSet* WarriorAttributeSet;

	 UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CharacterData")
	 //soft reference.
	 TSoftObjectPtr<UDataAsset_StartUpDatabase> CharacterStartUpData;

public:

	FORCEINLINE UWarriorAbilitySystemComponent* GetWarriorAbilitySystemComponent() const {
		
		return WarriorAbilitySystemComponent;
	}

	FORCEINLINE UWarriorAttributeSet* GetWarriorAttributeSet() const
	{
		return WarriorAttributeSet;
	}
};
