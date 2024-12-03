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

	//  karakterin en ve boy geniþliklerinin capsule boyutu.
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	//birazdan kamerayý karakterimize baðlayýp sabitleyeceðiz. Bunlarýn false olmasýnýn temel sebebi kamera ve karakter arasýndaki senkronize hareketi saðlamak.
	//bununla ilgili video çekeceðim daha iyi açýklayabilmek adýna.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	//SpringArmComponent'ý bir kamera tutucu-sýnýrlayýcý olarak düþünün
	//aklýnýzda karakterin arkasýnda kamera tutan hayali bir el gibi kalsýn, bu eli oluþturmak ve karaktere baðlamak için uðraþýyoruz aþaðýda
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	//burada Camera'mýzý, karakterimizin ana bileþenine baðlýyoruz.
	CameraBoom -> SetupAttachment(GetRootComponent());
	//büyülü nokta burasý, ben TPS (ThirdPersonShooter) yazdýðým için, kameramýn karakterime uzaklýðý 200 birim olacak, yani arkadan görünecek.
	//fps yazsaydým, 0 ya da +3-5 gibi denemelerle karakterin gözünden bakmaya çalýþýrdým
	//neden +3-5 diyorum, karaktere vereceðim mesh'e ve capsule size'a göre davranmalýyým, yukarýda initCapsuleSize'da yazdýðýmýz deðerlere uygun þekilde olmalý
	CameraBoom->TargetArmLength = 250.f;
	CameraBoom->SocketOffset = FVector(0.f, 55.f, 65.f);
	//cameraboom artýk pawn'ýmýzýn (karakterimizin) kontrol yönünü belirleyebilir. Sabitledik çünkü.
	CameraBoom->bUsePawnControlRotation = true;

	//Karakter ve Kamera baðlamalarý yapýldý. Þimdi, takip iþlemlerini gerçekleyelim
	/*
	followcamera bizim asýl kameramýz, cameraboom ise belirleyici, özellikleyici, yani takip mesafesi,
	nereden takip edecek, yönü ne olacak ? kamera özellikleri bir nevi. Socket name ise kameranýn hangi noktaya, nereye baðlý olacaðý
	USpringArm'da nereye baþlanacaðý, baðlanacaðý yerin adý yani.
	*/

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	//charactermovement'ý hareket edip edemeyeceðini seçtik
	GetCharacterMovement()->bOrientRotationToMovement = true;
	//bu karakterin y ekseninde etrafa  bakma hýzý
	//karakter stun vb. yerse mesela bu deðeri deltatime' ile sýfýrlarsýn bir süre boyunca.
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);
	//max 400f hýzla ilerleyebilir
	GetCharacterMovement()->MaxWalkSpeed = 400.f;
	//karakterin durma hýzý, BrakingDecelerationWalking ne kadar yüksek olursa karakter o hýzda durur.
	//araba yasýþý oyunlarýnda, araç 300km ile ilerliyorsa 2000f'de durmasý saçma olur, fizik kurallarýna aykýrýdýr
	//mesela ne kullanýlýrdý onlarda þöyle bir þey bence

	/*
	void AMyCarCharacter::UpdateSpeedBreaking(float DeltaTime)
	{
	static float i=150.f;
	GetCharacterMovement()->BrakingDecelerationWalking = GetCharacterMovement()->BrakingDecelerationWalking - (i * DeltaTime);
	i+=52.f*DeltaTime;


	burada i her saniye 52 birim artar, tabi i her geçen saniye sürekli artacaðý için hareket hýzý blokörü haline gelir
	if(i>=350.f)
	{
	break;
	}
	yaparak i 350'ye geldiði ya da geçtiði an döngüyü sonlandýrabiliriz.

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

	//oyundaki main karakterimizi seçtik
	ULocalPlayer* LocalPlayer = GetController<APlayerController>()->GetLocalPlayer();
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	checkf(Subsystem, TEXT("Subsystem kontrolü"));
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






