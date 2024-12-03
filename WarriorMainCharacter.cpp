// Can Gures, All Rights Reserved.


#include "Characters/WarriorMainCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DataAssets/Input/DataAsset_InputConfig.h"
#include "Components/Input/WarriorInputComponent.h"
#include "GameplayTags/WarriorGameplayTags.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "TerekDebugHelper.h"
#include "DataAssets/StartupData/DataAsset_StartUpDatabase.h"


AWarriorMainCharacter::AWarriorMainCharacter()
{

	//  karakterin en ve boy geni�liklerinin capsule boyutu.
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	//birazdan kameray� karakterimize ba�lay�p sabitleyece�iz. Bunlar�n false olmas�n�n temel sebebi kamera ve karakter aras�ndaki senkronize hareketi sa�lamak.
	//bununla ilgili video �ekece�im daha iyi a��klayabilmek ad�na.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	//SpringArmComponent'� bir kamera tutucu-s�n�rlay�c� olarak d���n�n
	//akl�n�zda karakterin arkas�nda kamera tutan hayali bir el gibi kals�n, bu eli olu�turmak ve karaktere ba�lamak i�in u�ra��yoruz a�a��da
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	//burada Camera'm�z�, karakterimizin ana bile�enine ba�l�yoruz.
	CameraBoom -> SetupAttachment(GetRootComponent());
	//b�y�l� nokta buras�, ben TPS (ThirdPersonShooter) yazd���m i�in, kameram�n karakterime uzakl��� 200 birim olacak, yani arkadan g�r�necek.
	//fps yazsayd�m, 0 ya da +3-5 gibi denemelerle karakterin g�z�nden bakmaya �al���rd�m
	//neden +3-5 diyorum, karaktere verece�im mesh'e ve capsule size'a g�re davranmal�y�m, yukar�da initCapsuleSize'da yazd���m�z de�erlere uygun �ekilde olmal�
	CameraBoom->TargetArmLength = 250.f;
	CameraBoom->SocketOffset = FVector(0.f, 55.f, 65.f);
	//cameraboom art�k pawn'�m�z�n (karakterimizin) kontrol y�n�n� belirleyebilir. Sabitledik ��nk�.
	CameraBoom->bUsePawnControlRotation = true;

	//Karakter ve Kamera ba�lamalar� yap�ld�. �imdi, takip i�lemlerini ger�ekleyelim
	/*
	followcamera bizim as�l kameram�z, cameraboom ise belirleyici, �zellikleyici, yani takip mesafesi,
	nereden takip edecek, y�n� ne olacak ? kamera �zellikleri bir nevi. Socket name ise kameran�n hangi noktaya, nereye ba�l� olaca��
	USpringArm'da nereye ba�lanaca��, ba�lanaca�� yerin ad� yani.
	*/

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	//charactermovement'� hareket edip edemeyece�ini se�tik
	GetCharacterMovement()->bOrientRotationToMovement = true;
	//bu karakterin y ekseninde etrafa  bakma h�z�
	//karakter stun vb. yerse mesela bu de�eri deltatime' ile s�f�rlars�n bir s�re boyunca.
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);
	//max 400f h�zla ilerleyebilir
	GetCharacterMovement()->MaxWalkSpeed = 400.f;
	//karakterin durma h�z�, BrakingDecelerationWalking ne kadar y�ksek olursa karakter o h�zda durur.
	//araba yas��� oyunlar�nda, ara� 300km ile ilerliyorsa 2000f'de durmas� sa�ma olur, fizik kurallar�na ayk�r�d�r
	//mesela ne kullan�l�rd� onlarda ��yle bir �ey bence

	/*
	void AMyCarCharacter::UpdateSpeedBreaking(float DeltaTime)
	{
	static float i=150.f;
	GetCharacterMovement()->BrakingDecelerationWalking = GetCharacterMovement()->BrakingDecelerationWalking - (i * DeltaTime);
	i+=52.f*DeltaTime;


	burada i her saniye 52 birim artar, tabi i her ge�en saniye s�rekli artaca�� i�in hareket h�z� blok�r� haline gelir
	if(i>=350.f)
	{
	break;
	}
	yaparak i 350'ye geldi�i ya da ge�ti�i an d�ng�y� sonland�rabiliriz.

	}
	
	
	*/
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;






}

void AWarriorMainCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	if (!CharacterStartUpData.IsNull())
	{
		if (UDataAsset_StartUpDatabase* LoadedData = CharacterStartUpData.LoadSynchronous())
		{
			LoadedData->GiveToAbilitySystemComponent(WarriorAbilitySystemComponent);
		}
	}
}

void AWarriorMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	checkf(InputConfigDataAsset, TEXT("InputConfigDataAsset check"));

	//oyundaki main karakterimizi se�tik
	ULocalPlayer* LocalPlayer = GetController<APlayerController>()->GetLocalPlayer();
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	checkf(Subsystem, TEXT("Subsystem kontrol�"));
	Subsystem->AddMappingContext(InputConfigDataAsset->DefaultMappingContext,0);
	UWarriorInputComponent* WarriorInputComponent = CastChecked<UWarriorInputComponent>(PlayerInputComponent);
	WarriorInputComponent->BindNativeInputAction(InputConfigDataAsset, WarriorGameplayTags::InputTag_Move,ETriggerEvent::Triggered,this,&ThisClass::Input_Move);
	WarriorInputComponent->BindNativeInputAction(InputConfigDataAsset, WarriorGameplayTags::InputTag_Look, ETriggerEvent::Triggered, this, &ThisClass::Input_Look);


}

void AWarriorMainCharacter::BeginPlay()
{
	Super::BeginPlay();
	Debug::Print(TEXT("Kagan, sorun yok!"));
}

void AWarriorMainCharacter::Input_Move(const FInputActionValue& InputActionValue)
{
	const FVector2D MovementVector = InputActionValue.Get<FVector2D>();
	const FRotator MovementRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);

	if (MovementVector.Y != 0.f)
	{
		const FVector ForwardDirection = MovementRotation.RotateVector(FVector::ForwardVector);
		AddMovementInput(ForwardDirection, MovementVector.Y);
	}
	if (MovementVector.X != 0.f)
	{
		const FVector RightDirection = MovementRotation.RotateVector(FVector::RightVector);
		AddMovementInput(RightDirection, MovementVector.X);

	}



}

void AWarriorMainCharacter::Input_Look(const FInputActionValue& InputActionValue)
{
	const FVector2D LookAxisVector = InputActionValue.Get<FVector2D>();
	if (LookAxisVector.X != 0.f)
	{
		AddControllerYawInput(LookAxisVector.X);
	}
	if (LookAxisVector.Y != 0.f)
	{
		AddControllerPitchInput(LookAxisVector.Y);
	}

}






