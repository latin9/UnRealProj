// Fill out your copyright notice in the Description page of Project Settings.


#include "Play/Actor/Player/WarriorCharacter.h"
#include "../Object/Hit/UR_NormalAttackHit.h"
#include "../Object/UR_UltimateSpline.h"
#include "../Object/UR_UltimateCharge.h"
#include "../Object/UR_GlowEffectActor.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PostProcessComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Global/URGameInstance.h"
#include "Global/URStructs.h"
#include "Camera/PlayerCameraManager.h"
#include "../../Skill/UR_LightningActor.h"
#include "../../Skill/Boss/UR_CircleActor.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetArrayLibrary.h"
#include "Math/UnrealMathUtility.h"
#include "Math/Matrix.h"
#include "Engine/EngineTypes.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "MatineeCameraShake.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

AWarriorCharacter::AWarriorCharacter() :
	m_Stream(FDateTime::Now().GetTicks()),
	m_CombatIdleMontage(nullptr),
	m_ComboType(EWarriorComboType::Default),
	m_IsRun(false),
	m_IsForwardDown(false),
	m_IsBackwardDown(false),
	m_IsLeftDown(false),
	m_IsRightDown(false),
	m_IsQSkill(false),
	m_IsESkillAttacking(false),
	m_IsRSkillAttacking(false),
	m_IsComboAttack(false),
	m_IsCombating(false),
	m_CombatTime(7.f),
	m_MouseXDPI(0.5f),
	m_MouseYDPI(0.5f),
	m_MoveSpeed(1.f),
	m_DashDist(600.f),
	m_OnDash(false),
	m_IsJump(false),
	m_JumpType(EWarriorJumpType::Default),
	m_AttackSpeed(1.f),
	m_UltimateAttackCount(8),
	m_NextUUID(0),
	m_PostProcessDeleteTime(4.f),
	m_TestTime(0.f),
	m_BlendAlpha(0.f)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	static ConstructorHelpers::FObjectFinder<UCurveFloat> Curve(TEXT("CurveFloat'/Game/Resource/Play/Curve/UR_DashCurve.UR_DashCurve'"));
	check(Curve.Succeeded());

	FloatCurve = Curve.Object;

	Tags.Add("Player");

	UCapsuleComponent* Component = FindComponentByClass<UCapsuleComponent>();

	if (Component)
	{
		Component->SetCollisionProfileName(FName(TEXT("Player")));
	}

	RootComponent = Component;

	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;

	m_DamageCollision = CreateDefaultSubobject<USphereComponent>(FName(TEXT("PlayerDamageCollision")));
	m_DamageCollision->SetCollisionProfileName(FName(TEXT("PlayerAttack")));
	m_DamageCollision->SetupAttachment(GetMesh(), FName("Attack_Socket"));

	m_CameraSpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(FName(TEXT("CameraSpringArm")));
	m_CameraSpringArmComponent->SetupAttachment(RootComponent);
	m_CameraSpringArmComponent->TargetArmLength = 800.0f;
	m_CameraComponentDefaultRot = m_CameraSpringArmComponent->GetTargetRotation();
	//m_CameraSpringArmComponent->SetRelativeRotation(FRotator(-55.0f, 0.0f, 0.0f));
	m_CameraSpringArmComponent->bDoCollisionTest = false;

	m_CameraComponent = CreateDefaultSubobject<UCameraComponent>(FName(TEXT("CameraComponent")));
	m_CameraComponent->SetupAttachment(m_CameraSpringArmComponent);
	//m_CameraComponent->bUsePawnControlRotation = false;

	{
		m_IconArm = CreateDefaultSubobject<USpringArmComponent>(FName(TEXT("IconArm")));
		m_IconArm->SetRelativeRotation(FRotator(-90, 0, 0));
		// �ڽ����� �޾��ش�.
		m_IconArm->SetupAttachment(RootComponent);
		m_IconArm->TargetArmLength = 300.f;

		//Material'/Game/Resource/Play/Sprite/UI/MonsterIcon_Mat.MonsterIcon_Mat'
		ConstructorHelpers::FObjectFinder<UStaticMesh> MeshPath(TEXT("StaticMesh'/Engine/BasicShapes/Plane.Plane'"));
		ConstructorHelpers::FObjectFinder<UMaterialInterface> MatPath(TEXT("Material'/Game/Resource/Play/UI/Blue_Circle_full_Mat.Blue_Circle_full_Mat'"));

		m_PlaneComponent = CreateDefaultSubobject<UStaticMeshComponent>(FName("PlayerIconPlane"));
		m_PlaneComponent->SetRelativeRotation(FRotator(0, 90, 90));
		m_PlaneComponent->SetupAttachment(m_IconArm);
		m_PlaneComponent->SetHiddenInSceneCapture(true);
		m_PlaneComponent->SetHiddenInGame(false);
		m_PlaneComponent->SetCastHiddenShadow(true);

		if (nullptr != MeshPath.Object
			&& MeshPath.Object->IsValidLowLevel())
		{
			m_PlaneComponent->SetStaticMesh(MeshPath.Object);
		}

		if (nullptr != MatPath.Object
			&& MatPath.Object->IsValidLowLevel())
		{
			m_PlaneComponent->SetMaterial(0, MatPath.Object);
		}

	}

	// ����Ʈ ���μ���
	{
		m_DrunkPostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(FName(TEXT("DrunkPostProcessComponent")));
		m_DrunkPostProcessComponent->SetupAttachment(m_CameraComponent);

		static ConstructorHelpers::FObjectFinder<UMaterialInstance> DrunkMatInst(TEXT("MaterialInstanceConstant'/Game/Resource/Play/Effect/Drunk_Effect/Drunk_Effect_Inst.Drunk_Effect_Inst'"));

		m_DrunkWeightedBlend = FWeightedBlendable(1.f, DrunkMatInst.Object);

		m_GlowPostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(FName(TEXT("GlowPostProcessComponent")));
		m_GlowPostProcessComponent->SetupAttachment(m_CameraComponent);

		static ConstructorHelpers::FObjectFinder<UMaterialInstance> GlowMatInst(TEXT("MaterialInstanceConstant'/Game/Resource/Play/Effect/Q_Effect/Mat_Q_Effect_Inst.Mat_Q_Effect_Inst'"));

		m_GlowWeightedBlend = FWeightedBlendable(1.f, GlowMatInst.Object);
	}

	// m_PostProcessComponent->Settings.WeightedBlendables.Array.Add(m_WeightedBlend);

	{
		static ConstructorHelpers::FObjectFinder<UNiagaraSystem> Trace(TEXT("NiagaraSystem'/Game/BluePrint/Play/Monster/BossSubObj/Khaimera/FX_Body.FX_Body'"));

		m_BlackHoleFX = Trace.Object;

		// Parameters can be set like this (see documentation for further info) - the names and type must match the user exposed parameter in the Niagara System
		m_BlackHoleNiagara = CreateDefaultSubobject<UNiagaraComponent>(FName(TEXT("BlackholeBodyParticles")));
		m_BlackHoleNiagara->SetAsset(m_BlackHoleFX);
		m_BlackHoleNiagara->SetupAttachment(GetMesh());
		m_BlackHoleNiagara->bAutoActivate = false;
		//Start Position_SpawnRate
	}

	m_SkillQConsumedMP = 10.0;
	m_SkillEConsumedMP = 20.0;
	m_SkillRConsumedMP = 30.0;
	
	//VoiceSound ����
	{
		static ConstructorHelpers::FObjectFinder<USoundBase> NoMPVoice(TEXT("SoundCue'/Game/Resource/Play/Sound/Voice/Kwang/Kwang_Ability_LowMana.Kwang_Ability_LowMana'"));

		if (NoMPVoice.Succeeded())
			m_Sound.NoMPVoice = NoMPVoice.Object;

		static ConstructorHelpers::FObjectFinder<USoundBase> CoolTimeVoice(TEXT("SoundCue'/Game/Resource/Play/Sound/Voice/Kwang/Kwang_Ability_OnCooldown.Kwang_Ability_OnCooldown'"));

		if (CoolTimeVoice.Succeeded())
			m_Sound.CoolTimeVoice = CoolTimeVoice.Object;

		static ConstructorHelpers::FObjectFinder<USoundBase> UltimateDash(TEXT("SoundWave'/Game/Resource/Play/Sound/SKill/Kwang/UltimateDash_Sound.UltimateDash_Sound'"));

		if (UltimateDash.Succeeded())
			m_Sound.UltimateDash = UltimateDash.Object;
	}
	{
		/*static ConstructorHelpers::FObjectFinder<UUserWidget> QuestSideWidget(TEXT("WidgetBlueprint'/Game/BluePrint/Play/UI/BP_QuestSideUI.BP_QuestSideUI'"));

		if (QuestSideWidget.Succeeded())
			m_QuestSizeWidget = QuestSideWidget.Object;*/
	}
}

void AWarriorCharacter::MouseMoveX(float Value)
{
	if (Value == 0.0f)
	{
		return;
	}


	//AddControllerYawInput(Value * m_MouseXDPI);

	FRotator Rot = FRotator(0.0, 1.0, 0.0);
	m_CameraSpringArmComponent->AddWorldRotation(Rot * Value);
}

void AWarriorCharacter::MouseMoveY(float Value)
{
	if (nullptr == m_CameraSpringArmComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("USpringArmComponent Cast Error!"));
		return;
	}

	// ����� ���Ҽ������ ����� �������ִ� 4���� ����� �̷���� �������� ����ε�.
	// 4x4ũ���� ȸ������� ����� ���� �ִ�.

	if (Value == 0.0f)
	{
		return;
	}

	// UE_LOG(LogTemp, Error, TEXT("ControlRotation : %s"), *GetControlRotation().Euler().ToString());

	FRotator NextRot = GetControlRotation();
	NextRot.Add(Value * 2.0f, 0.0, 0.0f);
	FVector3d EulerRot = NextRot.Euler();

	if (EulerRot.Y > 50 && EulerRot.Y < 300)
	{
		// GetController()->SetControlRotation(NextRot);
		return;
	}

	//m_CameraSpringArmComponent->TargetArmLength -= Value * 10.0f;

	AddControllerPitchInput(-Value * m_MouseYDPI);
}


void AWarriorCharacter::PlayerLeftMove(float Value)
{
	if (Value == 0.f || true == IsAttack())
	{
		return;
	}

	if (FirstJudgeFunc())
	{
		return;
	}

	TurnFunc();

	m_IsLeftDown = true;
	m_BlendAlpha = 0.f;
	AddMovementInput(-GetActorRightVector(), Value);

	if (JudgeFunc())
	{
		return;
	}

	if (!m_IsBlocking)
	{
		if (!m_IsCombating)
		{
			if (!m_IsRun)
			{
				if (m_IsLeftDown && !m_IsForwardDown && !m_IsRightDown && !m_IsBackwardDown)
				{
					GetAnimationInstance()->ChangeAnimMontage(DefaultAnimation::Left);
				}
				else if (m_IsLeftDown && m_IsForwardDown && !m_IsRightDown && !m_IsBackwardDown)
				{
					GetAnimationInstance()->ChangeAnimMontage(WarriorAnimation::ForWardLeft);
				}
				else if (m_IsLeftDown && m_IsBackwardDown && !m_IsRightDown && !m_IsForwardDown)
				{
					GetAnimationInstance()->ChangeAnimMontage(WarriorAnimation::BackWardLeft);
				}
			}
			else
			{
				if (m_IsLeftDown && !m_IsForwardDown && !m_IsRightDown && !m_IsBackwardDown)
				{
					GetAnimationInstance()->ChangeAnimMontage(WarriorAnimation::RunLeft);
				}
				else if (m_IsLeftDown && m_IsForwardDown && !m_IsRightDown && !m_IsBackwardDown)
				{
					GetAnimationInstance()->ChangeAnimMontage(WarriorAnimation::RunForwardLeft);
				}
				else if (m_IsLeftDown && m_IsBackwardDown && !m_IsRightDown && !m_IsForwardDown)
				{
					GetAnimationInstance()->ChangeAnimMontage(WarriorAnimation::RunBackwardLeft);
				}
			}
		}
		else
		{
			if (!m_IsRun)
			{
				if (m_IsLeftDown && !m_IsForwardDown && !m_IsRightDown && !m_IsBackwardDown)
				{
					GetAnimationInstance()->ChangeAnimMontage(WarriorCombatAnimation::CombatWalkLeft);
				}
				else if (m_IsLeftDown && m_IsForwardDown && !m_IsRightDown && !m_IsBackwardDown)
				{
					GetAnimationInstance()->ChangeAnimMontage(WarriorCombatAnimation::CombatWalkForWardLeft);
				}
				else if (m_IsLeftDown && m_IsBackwardDown && !m_IsRightDown && !m_IsForwardDown)
				{
					GetAnimationInstance()->ChangeAnimMontage(WarriorCombatAnimation::CombatWalkBackWardLeft);
				}
			}
			else
			{
				if (m_IsLeftDown && !m_IsForwardDown && !m_IsRightDown && !m_IsBackwardDown)
				{
					GetAnimationInstance()->ChangeAnimMontage(WarriorCombatAnimation::CombatRunLeft);
				}
				else if (m_IsLeftDown && m_IsForwardDown && !m_IsRightDown && !m_IsBackwardDown)
				{
					GetAnimationInstance()->ChangeAnimMontage(WarriorCombatAnimation::CombatRunForWardLeft);
				}
				else if (m_IsLeftDown && m_IsBackwardDown && !m_IsRightDown && !m_IsForwardDown)
				{
					GetAnimationInstance()->ChangeAnimMontage(WarriorCombatAnimation::CombatRunBackWardLeft);
				}
			}
		}
	}
	else
	{
		if (m_IsLeftDown && !m_IsForwardDown && !m_IsRightDown && !m_IsBackwardDown)
		{
			GetAnimationInstance()->ChangeAnimMontage(WarriorBlockAnimation::BlockWalkLoopLeft);
		}
		else if (m_IsLeftDown && m_IsForwardDown && !m_IsRightDown && !m_IsBackwardDown)
		{
			GetAnimationInstance()->ChangeAnimMontage(WarriorBlockAnimation::BlockWalkLoopLF);
		}
		else if (m_IsLeftDown && m_IsBackwardDown && !m_IsRightDown && !m_IsForwardDown)
		{
			GetAnimationInstance()->ChangeAnimMontage(WarriorBlockAnimation::BlockWalkLoopLB);
		}
	}
}

void AWarriorCharacter::PlayerRightMove(float Value)
{
	if (Value == 0.f || true == IsAttack())
	{
		return;
	}


	if (FirstJudgeFunc())
	{
		return;
	}


	TurnFunc();

	m_IsRightDown = true;

	m_BlendAlpha = 0.f;
	AddMovementInput(GetActorRightVector(), Value);

	if (JudgeFunc())
	{
		return;
	}

	if (!m_IsBlocking)
	{
		if (!m_IsCombating)
		{
			if (!m_IsRun)
			{
				if (m_IsRightDown && !m_IsForwardDown && !m_IsLeftDown && !m_IsBackwardDown)
				{
					GetAnimationInstance()->ChangeAnimMontage(DefaultAnimation::Right);
				}
				else if (m_IsRightDown && m_IsForwardDown && !m_IsLeftDown && !m_IsBackwardDown)
				{
					GetAnimationInstance()->ChangeAnimMontage(WarriorAnimation::ForWardRight);
				}
				else if (m_IsRightDown && m_IsBackwardDown && !m_IsLeftDown && !m_IsForwardDown)
				{
					GetAnimationInstance()->ChangeAnimMontage(WarriorAnimation::BackWardRight);
				}
			}
			else
			{
				if (m_IsRightDown && !m_IsForwardDown && !m_IsLeftDown && !m_IsBackwardDown)
				{
					GetAnimationInstance()->ChangeAnimMontage(WarriorAnimation::RunRight);
				}
				else if (m_IsRightDown && m_IsForwardDown && !m_IsLeftDown && !m_IsBackwardDown)
				{
					GetAnimationInstance()->ChangeAnimMontage(WarriorAnimation::RunForwardRight);
				}
				else if (m_IsRightDown && m_IsBackwardDown && !m_IsLeftDown && !m_IsForwardDown)
				{
					GetAnimationInstance()->ChangeAnimMontage(WarriorAnimation::RunBackwardRight);
				}
			}
		}
		else
		{
			if (!m_IsRun)
			{
				if (m_IsRightDown && !m_IsForwardDown && !m_IsLeftDown && !m_IsBackwardDown)
				{
					GetAnimationInstance()->ChangeAnimMontage(WarriorCombatAnimation::CombatWalkRight);
				}
				else if (m_IsRightDown && m_IsForwardDown && !m_IsLeftDown && !m_IsBackwardDown)
				{
					GetAnimationInstance()->ChangeAnimMontage(WarriorCombatAnimation::CombatWalkForWardRight);
				}
				else if (m_IsRightDown && m_IsBackwardDown && !m_IsLeftDown && !m_IsForwardDown)
				{
					GetAnimationInstance()->ChangeAnimMontage(WarriorCombatAnimation::CombatWalkBackWardRight);
				}
			}
			else
			{
				if (m_IsRightDown && !m_IsForwardDown && !m_IsLeftDown && !m_IsBackwardDown)
				{
					GetAnimationInstance()->ChangeAnimMontage(WarriorCombatAnimation::CombatRunRight);
				}
				else if (m_IsRightDown && m_IsForwardDown && !m_IsLeftDown && !m_IsBackwardDown)
				{
					GetAnimationInstance()->ChangeAnimMontage(WarriorCombatAnimation::CombatRunForWardRight);
				}
				else if (m_IsRightDown && m_IsBackwardDown && !m_IsLeftDown && !m_IsForwardDown)
				{
					GetAnimationInstance()->ChangeAnimMontage(WarriorCombatAnimation::CombatRunBackWardRight);
				}
			}
		}
	}
	else
	{
		/*if (GetAnimationInstance()->IsAnimMontage(WarriorBlockAnimation::BlockWalkLoopRight))
		{
			return;
		}*/
		if (m_IsRightDown && !m_IsForwardDown && !m_IsLeftDown && !m_IsBackwardDown)
		{
			GetAnimationInstance()->ChangeAnimMontage(WarriorBlockAnimation::BlockWalkLoopRight);
		}
		else if (m_IsRightDown && m_IsForwardDown && !m_IsLeftDown && !m_IsBackwardDown)
		{
			GetAnimationInstance()->ChangeAnimMontage(WarriorBlockAnimation::BlockWalkLoopRF);
		}
		else if (m_IsRightDown && m_IsBackwardDown && !m_IsLeftDown && !m_IsForwardDown)
		{
			GetAnimationInstance()->ChangeAnimMontage(WarriorBlockAnimation::BlockWalkLoopRB);
		}
	}

	AttackOff();
}

void AWarriorCharacter::PlayerForwardMove(float Value)
{
	if (Value == 0.f || true == IsAttack())
	{
		return;
	}

	if (FirstJudgeFunc())
	{
		return;
	}

	m_IsForwardDown = true;

	TurnFunc();

	m_BlendAlpha = 0.f;
	AddMovementInput(GetActorForwardVector(), Value);

	if (JudgeFunc())
	{
		return;
	}

	if (!m_IsBlocking)
	{
		if (!m_IsCombating)
		{
			if (!m_IsRun)
			{
				if (m_IsForwardDown && !m_IsLeftDown && !m_IsRightDown)
				{
					GetAnimationInstance()->ChangeAnimMontage(DefaultAnimation::Forward);
				}
			}
			else
			{
				if (m_IsForwardDown && !m_IsLeftDown && !m_IsRightDown)
				{
					GetAnimationInstance()->ChangeAnimMontage(WarriorAnimation::RunForward);
				}
			}
		}
		else
		{
			if (!m_IsRun)
			{
				if (m_IsForwardDown && !m_IsLeftDown && !m_IsRightDown)
				{
					GetAnimationInstance()->ChangeAnimMontage(WarriorCombatAnimation::CombatWalkForward);
				}
			}
			else
			{
				if (m_IsForwardDown && !m_IsLeftDown && !m_IsRightDown)
				{
					GetAnimationInstance()->ChangeAnimMontage(WarriorCombatAnimation::CombatRunForward);
				}
			}
		}
	}
	else
	{
		if (m_IsForwardDown && !m_IsLeftDown && !m_IsRightDown)
		{
			GetAnimationInstance()->ChangeAnimMontage(WarriorBlockAnimation::BlockWalkLoopForward);
		}
	}

	AttackOff();
}

void AWarriorCharacter::PlayerBackwardMove(float Value)
{
	if (Value == 0.f || true == IsAttack())
	{
		return;
	}

	if (FirstJudgeFunc())
	{
		return;
	}

	m_IsBackwardDown = true;

	TurnFunc();

	m_BlendAlpha = 0.f;
	AddMovementInput(-GetActorForwardVector(), Value);

	if (JudgeFunc())
	{
		return;
	}

	if (!m_IsBlocking)
	{
		if (!m_IsCombating)
		{
			if (!m_IsRun)
			{
				if (m_IsBackwardDown && !m_IsLeftDown && !m_IsRightDown)
				{
					GetAnimationInstance()->ChangeAnimMontage(DefaultAnimation::BackWard);
				}
			}
			else
			{
				if (m_IsBackwardDown && !m_IsLeftDown && !m_IsRightDown)
				{
					GetAnimationInstance()->ChangeAnimMontage(WarriorAnimation::RunBackward);
				}
			}
		}
		else
		{
			if (!m_IsRun)
			{
				if (m_IsBackwardDown && !m_IsLeftDown && !m_IsRightDown)
				{
					GetAnimationInstance()->ChangeAnimMontage(WarriorCombatAnimation::CombatWalkBackWard);
				}
			}
			else
			{
				if (m_IsBackwardDown && !m_IsLeftDown && !m_IsRightDown)
				{
					GetAnimationInstance()->ChangeAnimMontage(WarriorCombatAnimation::CombatRunBackWard);
				}
			}
		}
	}
	else
	{
		if (m_IsBackwardDown && !m_IsLeftDown && !m_IsRightDown)
		{
			GetAnimationInstance()->ChangeAnimMontage(WarriorBlockAnimation::BlockWalkLoopBackward);
		}
	}

	AttackOff();
}


void AWarriorCharacter::WheelKey(float Value)
{
	if (m_CameraSpringArmComponent->TargetArmLength + Value * 30.f < 2200.f &&
		m_CameraSpringArmComponent->TargetArmLength + Value * 30.f > 400.f)
	{
		m_CameraSpringArmComponent->TargetArmLength += Value * 30.f;
	}
	else if (m_CameraSpringArmComponent->TargetArmLength > 2200.f)
	{
		m_CameraSpringArmComponent->TargetArmLength = 2199.f;
	}
	else if (m_CameraSpringArmComponent->TargetArmLength < 400.f)
	{
		m_CameraSpringArmComponent->TargetArmLength = 401.f;
	}
}

void AWarriorCharacter::LeftAttack()
{
	m_IsComboAttack = true;
	m_IsCombating = true;
	m_BlendAlpha = 1.f;

	if (true == IsAttack() || m_IsESkillAttacking)
	{
		return;
	}

	m_CombatTime = 7.f;

	if (m_Shield != nullptr)
	{
		m_Shield->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		m_Shield->DestroyComponent();
		GetCharacterMovement()->MaxWalkSpeed = m_PlayerInfo->Speed;
	}

	if (m_IsQSkill)
	{
		m_MonsterActor = GetTargetActor();
	}

	if (m_IsRun && m_IsForwardDown)
	{
		AttackOn();
		GetAnimationInstance()->ChangeAnimMontage(WarriorAnimation::CommandDashAttack);
	}

	else
	{
		// 1 ~ 3�� ���� �������� �޺��� 3���̱� ����
		m_ComboType = (EWarriorComboType)m_Stream.RandRange(1, 3);

		switch (m_ComboType)
		{
		case EWarriorComboType::ComboA:
			AttackOn();
			GetAnimationInstance()->ChangeAnimMontage(DefaultAnimation::Attack);
			break;
		case EWarriorComboType::ComboB:
			AttackOn();
			GetAnimationInstance()->ChangeAnimMontage(WarriorAnimation::ComboB1);
			break;
		case EWarriorComboType::ComboC:
			AttackOn();
			GetAnimationInstance()->ChangeAnimMontage(WarriorAnimation::ComboC1);
			break;
		}
	}
	CameraShake(CameraShake_Type::HitShake);
}

void AWarriorCharacter::LeftAttackUp()
{
}

void AWarriorCharacter::RightBlock()
{
	if (true == IsAttack() || m_IsESkillAttacking)
	{
		return;
	}

	if (FirstJudgeFunc())
	{
		return;
	}

	m_IsBlocking = true;
	m_IsCombating = true;

	GetCharacterMovement()->MaxWalkSpeed = 300.f;

	AttackOff();

	if (!GetAnimationInstance()->IsAnimMontage(WarriorBlockAnimation::BlockLoop))
		GetAnimationInstance()->ChangeAnimMontage(WarriorBlockAnimation::BlockStart);

	if (m_ShieldEffect)
	{
		m_Shield = UNiagaraFunctionLibrary::SpawnSystemAttached(m_ShieldEffect, RootComponent, NAME_None, FVector(0.f), FRotator(0.f), EAttachLocation::Type::KeepRelativeOffset, true);
	}
}

void AWarriorCharacter::RightBlockUp()
{
	if (FirstJudgeFunc())
	{
		return;
	}
	m_IsBlocking = false;
	m_IsCombating = false;

	GetCharacterMovement()->MaxWalkSpeed = m_PlayerInfo->Speed;

	if (m_Shield != nullptr)
	{
		m_Shield->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		m_Shield->DestroyComponent();
	}

	if (JudgeFunc())
	{
		return;
	}

	GetAnimationInstance()->ChangeAnimMontage(WarriorBlockAnimation::BlockEnd);



	// ������
	//if (m_IsRightDown && !m_IsForwardDown && !m_IsLeftDown && !m_IsBackwardDown)
	//{
	//	GetAnimationInstance()->ChangeAnimMontage(WarriorBlockAnimation::BlockWalkStopRight);
	//}
	//// ����
	//else if (m_IsLeftDown && !m_IsForwardDown && !m_IsRightDown && !m_IsBackwardDown)
	//{
	//	GetAnimationInstance()->ChangeAnimMontage(WarriorBlockAnimation::BlockWalkStopLeft);
	//}
	//// ��
	//else if (m_IsForwardDown && !m_IsLeftDown && !m_IsRightDown && !m_IsBackwardDown)
	//{
	//	GetAnimationInstance()->ChangeAnimMontage(WarriorBlockAnimation::BlockWalkStop);
	//}
	//// ��
	//else if (m_IsBackwardDown && !m_IsForwardDown && !m_IsRightDown && !m_IsLeftDown)
	//{
	//	GetAnimationInstance()->ChangeAnimMontage(WarriorBlockAnimation::DashBackward);
	//}
	//// ���
	//else if (m_IsRightDown && m_IsForwardDown && !m_IsLeftDown && !m_IsBackwardDown)
	//{
	//	GetAnimationInstance()->ChangeAnimMontage(WarriorBlockAnimation::DashForwardRight);
	//}
	//// �޾�
	//else if (m_IsLeftDown && m_IsForwardDown && !m_IsRightDown && !m_IsBackwardDown)
	//{
	//	GetAnimationInstance()->ChangeAnimMontage(WarriorBlockAnimation::DashForwardLeft);
	//}
	//// ���
	//else if (m_IsRightDown && m_IsBackwardDown && !m_IsLeftDown && !m_IsForwardDown)
	//{
	//	GetAnimationInstance()->ChangeAnimMontage(WarriorBlockAnimation::DashBackwardRight);
	//}
	//// �޵�
	//else if (m_IsLeftDown && m_IsBackwardDown && !m_IsRightDown && !m_IsForwardDown)
	//{
	//	GetAnimationInstance()->ChangeAnimMontage(WarriorBlockAnimation::DashBackwardLeft);
	//}
}

void AWarriorCharacter::SkillQ()
{
	if (true == IsAttack() || JudgeFunc())
	{
		return;
	}

	if (m_IsQSkill)
	{
		CharacterSoundPlay(m_Sound.CoolTimeVoice, 0.2f, 1.f);
		return;
	}

	if (m_MP < m_SkillQConsumedMP)
	{
		CharacterSoundPlay(m_Sound.NoMPVoice, 0.2f, 1.f);
		return;
	}

	for (int32 i = 0; i < m_ArrayRimLightMat.Num(); ++i)
	{
		GetMesh()->SetMaterial(i, m_ArrayRimLightMat[i]);
	}

	m_IsQSkill = true;
	m_AttackSpeed = 1.3f;
	SetBerserkRateScale();

	GetAnimationInstance()->ChangeAnimMontage(WarriorAnimation::SkillQ);
	AddPostProcessGlowMat();

	//m_GlowSphere = GetWorld()->SpawnActor<AUR_GlowEffectActor>(AUR_GlowEffectActor::StaticClass(), GetActorTransform());
	//m_GlowSphere->AttachToComponent(m_CameraComponent, FAttachmentTransformRules::KeepWorldTransform);
	////m_GlowSphere->SetPivotOffset(FVector(-500.0, 0.0, 0.0));


	m_MP -= m_SkillQConsumedMP;

	m_MPPercent = m_MP / m_PlayerInfo->MaxMP;
}

void AWarriorCharacter::SkillE()
{
	if (true == IsAttack() || JudgeFunc())
	{
		return;
	}

	if (m_IsESkillAttacking)
	{
		CharacterSoundPlay(m_Sound.CoolTimeVoice, 0.2f, 1.f);
		return;
	}

	if (m_MP < m_SkillEConsumedMP)
	{
		CharacterSoundPlay(m_Sound.NoMPVoice, 0.2f, 1.f);
		return;
	}

	m_IsESkillAttacking = true;

	//AttackOn();
	GetAnimationInstance()->ChangeAnimMontage(WarriorAnimation::SkillELoop);
	m_MP -= m_SkillEConsumedMP;

	m_MPPercent = m_MP / m_PlayerInfo->MaxMP;
}

void AWarriorCharacter::SkillR()
{
	if (true == IsAttack())
	{
		return;
	}

	if (m_MP < m_SkillRConsumedMP)
	{
		CharacterSoundPlay(m_Sound.NoMPVoice, 0.2f, 1.f);
		return;
	}

	m_UltimateTargetMonster = GetSphereTraceHitActor(100.f, 800.f);

	if (m_UltimateTargetMonster.IsEmpty())
	{
		return;
	}

	AttackOn();
	m_IsInvincibility = true;
	m_UltimateAttackCount = 14;

	m_CameraSpringArmComponent->bUsePawnControlRotation = false;
	m_CameraSpringArmComponent->bInheritPitch = false;


	m_CameraSpringArmComponent->TargetArmLength = 1500;


	if (!m_UltimateTargetMonster.IsEmpty())
		m_UltimateCameraTarget = m_UltimateTargetMonster[0];

	m_UltimateTargetCount = m_UltimateTargetMonster.Num();

	m_PrevZ = GetActorLocation().Z;

	GetAnimationInstance()->ChangeAnimMontage(WarriorAnimation::SkillRStart);
	m_MP -= m_SkillRConsumedMP;

	m_MPPercent = m_MP / m_PlayerInfo->MaxMP;
}

void AWarriorCharacter::ForwardKeyEnd()
{
	m_IsForwardDown = false;
	m_BlendAlpha = 1.f;

	if (FirstJudgeFunc())
	{
		return;
	}

	if (JudgeFunc())
	{
		return;
	}
	
	if (!m_IsBlocking)
	{
		if (!m_IsCombating)
		{
			GetAnimationInstance()->ChangeAnimMontage(DefaultAnimation::Idle);
		}
		else
		{
			GetAnimationInstance()->ChangeAnimMontage(WarriorCombatAnimation::CombatIdle);
		}
	}
	else
	{
		GetAnimationInstance()->ChangeAnimMontage(WarriorBlockAnimation::BlockLoop);
	}
}

void AWarriorCharacter::BackwardKeyEnd()
{
	m_IsBackwardDown = false;
	m_BlendAlpha = 1.f;

	if (FirstJudgeFunc())
	{
		return;
	}

	if (JudgeFunc())
	{
		return;
	}

	if (!m_IsBlocking)
	{
		if (!m_IsCombating)
		{
			GetAnimationInstance()->ChangeAnimMontage(DefaultAnimation::Idle);
		}
		else
		{
			GetAnimationInstance()->ChangeAnimMontage(WarriorCombatAnimation::CombatIdle);
		}
	}
	else
	{
		GetAnimationInstance()->ChangeAnimMontage(WarriorBlockAnimation::BlockLoop);
	}
}

void AWarriorCharacter::LeftKeyEnd()
{
	m_IsLeftDown = false;
	m_BlendAlpha = 1.f;

	if (FirstJudgeFunc())
	{
		return;
	}

	if (JudgeFunc())
	{
		return;
	}

	if (!m_IsBlocking)
	{
		if (!m_IsCombating)
		{
			GetAnimationInstance()->ChangeAnimMontage(DefaultAnimation::Idle);
		}
		else
		{
			GetAnimationInstance()->ChangeAnimMontage(WarriorCombatAnimation::CombatIdle);
		}
	}
	else
	{
		GetAnimationInstance()->ChangeAnimMontage(WarriorBlockAnimation::BlockLoop);
	}
}

void AWarriorCharacter::RightKeyEnd()
{
	m_IsRightDown = false;
	m_BlendAlpha = 1.f;

	if (FirstJudgeFunc())
	{
		return;
	}

	if (JudgeFunc())
	{
		return;
	}

	if (!m_IsBlocking)
	{
		if (!m_IsCombating)
		{
			GetAnimationInstance()->ChangeAnimMontage(DefaultAnimation::Idle);
		}
		else
		{
			GetAnimationInstance()->ChangeAnimMontage(WarriorCombatAnimation::CombatIdle);
		}
	}
	else
	{
		GetAnimationInstance()->ChangeAnimMontage(WarriorBlockAnimation::BlockLoop);
	}
}

void AWarriorCharacter::CtrlKeyDown()
{
	m_IsRun = true;
	GetCharacterMovement()->MaxWalkSpeed = 1000;

	
}

void AWarriorCharacter::CtrlKeyOn()
{
	m_IsRun = false;
	GetCharacterMovement()->MaxWalkSpeed = m_PlayerInfo->Speed;
}

void AWarriorCharacter::ShiftKeyDown()
{
	if (DashJudgeFunc())
	{
		return;
	}

	if (m_Stamina - 20.f < 0.f)
	{
		return;
	}

	AttackOff();
	m_IsComboAttack = false;
	m_Stamina -= 20.f;
	

	// �ڷ���Ʈ � �������� ���� �Ǵ��ϴ� �Լ��̴�
	DashToJudge();
}

void AWarriorCharacter::ShiftKeyOn()
{
}

void AWarriorCharacter::DamageOn(bool _IsKnockBack)
{
	if (m_IsQSkill)
	{
		return;
	}

	if (!m_IsQSkill)
	{
		m_DamageCollision->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	}
	TArray<FHitResult> Targets = CollisionCheck(m_DamageCollision->GetComponentLocation(),
		FName(TEXT("PlayerAttackTrace")), m_DamageCollision->GetCollisionShape());

	bool Check = false;

	for (size_t i = 0; i < Targets.Num(); ++i)
	{
		AURCharacter* Character = Cast<AURCharacter>(Targets[i].GetActor());

		if (Character)
		{
			if (m_IsKnockDown)
			{
				Character->SetHitType(EHitType::KnockDownHit);
			}
			else
			{
				Character->SetHitType(EHitType::NormalHit);
			}
			Check = true;
			Character->CallDamage(3.0, this);
			FActorSpawnParameters spawnParams;
			CreateParticleObject<AUR_NormalAttackHit>(Character);
			Character->CharacterSoundPlay(m_HitSound, Character->GetRootComponent(), FName(TEXT("root")), 0.3f, 1.f);
		}
	}

	FColor Color = FColor::Green;

	if (Check)
	{
		Color = FColor::Red;
	}

	/*DrawDebugSphere(GetWorld(), m_DamageCollision->GetComponentLocation(), m_DamageCollision->GetScaledSphereRadius(),
		15, Color, false, 0.1f);*/
	
	/*TArray<UActorComponent*> Array = GetDamageCollision();

	for (size_t i = 0; i < Array.Num(); ++i)
	{
		bool Check = false;

		USphereComponent* Sphere = Cast<USphereComponent>(Array[i]);

		if (!Sphere)
			return;

		TArray<FHitResult> Targets = CollisionCheck(Sphere->GetComponentLocation(),
			FName(TEXT("PlayerAttackTrace")), Sphere->GetCollisionShape());

		for (int MonsterCount = 0; MonsterCount < Targets.Num(); ++MonsterCount)
		{
			AURCharacter* Character = Cast<AURCharacter>(Targets[MonsterCount].GetActor());

			if (Character)
			{
				Check = true;
				Character->CallDamage(1.0);
			}
		}

		FColor Color = FColor::Green;

		if (Check)
		{
			Color = FColor::Red;
		}

		DrawDebugSphere(GetWorld(), Sphere->GetComponentLocation(), Sphere->GetScaledSphereRadius(),
			15, Color, false, 0.1f);
	}*/
}
void AWarriorCharacter::DamageOff()
{
	m_DamageCollision->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
}
void AWarriorCharacter::BeginPlay()
{
	FOnTimelineFloat onTimelineCallback;
	FOnTimelineEventStatic onTimelineFinishedCallback;

	Super::BeginPlay();

	if (FloatCurve != NULL)
	{
		MyTimeline = NewObject<UTimelineComponent>(this, FName("TimelineAnimation"));
		MyTimeline->CreationMethod = EComponentCreationMethod::UserConstructionScript; // Indicate it comes from a blueprint so it gets cleared when we rerun construction scripts
		this->BlueprintCreatedComponents.Add(MyTimeline); // Add to array so it gets saved
		MyTimeline->SetNetAddressable();    // This component has a stable name that can be referenced for replication

		MyTimeline->SetPropertySetObject(this); // Set which object the timeline should drive properties on
		MyTimeline->SetDirectionPropertyName(FName("TimelineDirection"));

		MyTimeline->SetLooping(false);
		MyTimeline->SetTimelineLength(5.f);
		MyTimeline->SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);

		MyTimeline->SetPlaybackPosition(0.0f, false);

		//Add the float curve to the timeline and connect it to your timelines's interpolation function
		onTimelineCallback.BindUFunction(this, FName{ TEXT("TimelineCallback") });
		onTimelineFinishedCallback.BindUFunction(this, FName{ TEXT("TimelineFinishedCallback") });
		MyTimeline->AddInterpFloat(FloatCurve, onTimelineCallback);
		MyTimeline->SetTimelineFinishedFunc(onTimelineFinishedCallback);

		MyTimeline->RegisterComponent();
	}

	m_PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (!m_PlayerController || !m_PlayerController->IsValidLowLevel())
	{
		return;
	}

	for (auto& Anim : m_PlayerAnimations)
	{
		GetAnimationInstance()->AddAnimMontage(static_cast<int>(Anim.Key), Anim.Value);
	}

	for (auto& Anim : m_PlayerCombatAnimations)
	{
		GetAnimationInstance()->AddAnimMontage(static_cast<int>(Anim.Key), Anim.Value);
	}

	for (auto& Anim : m_PlayerJumpAnimations)
	{
		GetAnimationInstance()->AddAnimMontage(static_cast<int>(Anim.Key), Anim.Value);
	}

	for (auto& Anim : m_PlayerHitAnimations)
	{
		GetAnimationInstance()->AddAnimMontage(static_cast<int>(Anim.Key), Anim.Value);
	}

	for (auto& Anim : m_PlayerBlockAnimations)
	{
		GetAnimationInstance()->AddAnimMontage(static_cast<int>(Anim.Key), Anim.Value);
	}


	m_PlayerInfo = GetWorld()->GetGameInstance<UURGameInstance>()->GetPlayerData(FName(TEXT("Player2")));

	SetDefaultData();
	CommandStructInit();

	m_CombatIdleMontage = GetAnimationInstance()->GetAnimation(WarriorCombatAnimation::CombatIdle);

	m_ArrayNormalMat = GetMesh()->GetMaterials();

	GetWorld()->GetGameInstance<UURGameInstance>()->QuestSaveFull(this);

	TArray<UUserWidget*> ArrayWidget;

	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), ArrayWidget, UUserWidget::StaticClass());


	for (int i = 0; i < ArrayWidget.Num(); ++i)
	{
		FString Name = ArrayWidget[i]->GetName();

		int Length = ArrayWidget[i]->GetName().Len();

		for (int j = Length - 1; j > 0; --j)
		{
			if (Name[j] == 'I')
			{
				// \ ��������(&Path[i + 1]) 0���� ������̴�.
				memset(&Name[j + 1], 0, sizeof(TCHAR) * (Length - j - 1));
				break;
			}
		}

		if ((Name == L"BP_QuestSideUI") &&
			m_IsQuesting)
		{
			ArrayWidget[i]->SetVisibility(ESlateVisibility::Visible);
			break;
		}
	}
	GetWorld();
}

void AWarriorCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (m_Shield)
	{
		m_Shield->SetWorldLocation(GetActorLocation());
	}

	if (MyTimeline != NULL)
	{
		MyTimeline->TickComponent(DeltaTime, ELevelTick::LEVELTICK_TimeOnly, NULL);
	}

	if (m_IsForwardDown || m_IsBackwardDown || m_IsLeftDown || m_IsRightDown)
	{
		if (m_IsRun && !m_IsAttack)
		{
			if (m_Stamina - DeltaTime * 15.f > 0.f)
			{
				m_Stamina -= DeltaTime * 7.f;
			}
			if (m_Stamina <= 0.f)
			{
				m_Stamina = 0.f;
				m_IsRun = false;
				GetCharacterMovement()->MaxWalkSpeed = 600;
			}

		}

		else
		{
			if (m_Stamina < m_PlayerInfo->MaxStamina)
			{
				m_Stamina += DeltaTime * 15.f;
			}
			else if (m_Stamina > m_PlayerInfo->MaxStamina)
			{
				m_Stamina = m_PlayerInfo->MaxStamina;
			}
		}
	}

	else
	{
		if (m_Stamina < m_PlayerInfo->MaxStamina)
		{
			m_Stamina += DeltaTime * 15.f;
		}
		else if (m_Stamina > m_PlayerInfo->MaxStamina)
		{
			m_Stamina = m_PlayerInfo->MaxStamina;
		}
	}

	BlockStaminaTick(DeltaTime);
	CombatTick(DeltaTime);
	CoolTimeTick(DeltaTime);

	JumpTrace();
	CommandTimeJudge(DeltaTime);

	if (m_IsUltimateAttack)
	{
		if (m_UltimateTargetMonster.Num() != 0)
		{
			if (m_UltimateCameraTarget)
			{
				FVector TargetPos = GetActorLocation();//m_UltimateCameraTarget->GetActorLocation() + FVector(0.0, 0.0, -1.0) * 1000.f;
				FRotator TargetRotator = UKismetMathLibrary::FindLookAtRotation(m_CameraComponent->GetComponentLocation(),
					TargetPos);

				FRotator Rot = m_CameraSpringArmComponent->GetComponentRotation();

				Rot = FMath::RInterpTo(Rot, TargetRotator,
					DeltaTime, 5.f);

				m_CameraSpringArmComponent->SetWorldRotation(Rot);
			}
		}
	}

	TickPostProcessDeleteFunc(DeltaTime);

	m_TestTime += DeltaTime;

	if (m_TestTime >= 2.f)
	{
		m_TestTime = 0.f;
		FVector Velocity = GetCharacterMovement()->Velocity;

		Velocity.Z = 0.f;
	}


}

void AWarriorCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MouseMoveX"), this, &AWarriorCharacter::MouseMoveX);
	PlayerInputComponent->BindAxis(TEXT("MouseMoveY"), this, &AWarriorCharacter::MouseMoveY);

	PlayerInputComponent->BindAxis(FName(TEXT("LeftMove")),
		this, &AWarriorCharacter::PlayerLeftMove);
	PlayerInputComponent->BindAxis(FName(TEXT("RightMove")), this, &AWarriorCharacter::PlayerRightMove);
	PlayerInputComponent->BindAxis(FName(TEXT("ForwardMove")), this, &AWarriorCharacter::PlayerForwardMove);
	PlayerInputComponent->BindAxis(FName(TEXT("BackwardMove")), this, &AWarriorCharacter::PlayerBackwardMove);
	PlayerInputComponent->BindAction(FName(TEXT("ForwardMove")), EInputEvent::IE_Released, this, &AWarriorCharacter::ForwardKeyEnd);
	PlayerInputComponent->BindAction(FName(TEXT("LeftMove")), EInputEvent::IE_Released, this, &AWarriorCharacter::LeftKeyEnd);
	PlayerInputComponent->BindAction(FName(TEXT("RightMove")), EInputEvent::IE_Released, this, &AWarriorCharacter::RightKeyEnd);
	PlayerInputComponent->BindAction(FName(TEXT("BackwardMove")), EInputEvent::IE_Released, this, &AWarriorCharacter::BackwardKeyEnd);

	PlayerInputComponent->BindAction(FName(TEXT("Jump")), IE_Pressed, this, &AWarriorCharacter::Jump);
	PlayerInputComponent->BindAction(FName(TEXT("Jump")), IE_Released, this, &AWarriorCharacter::StopJumping);

	PlayerInputComponent->BindAction(FName(TEXT("AttackClick")), EInputEvent::IE_Pressed, this, &AWarriorCharacter::LeftAttack);
	PlayerInputComponent->BindAction(FName(TEXT("AttackClick")), EInputEvent::IE_Released, this, &AWarriorCharacter::LeftAttackUp);
	PlayerInputComponent->BindAction(FName(TEXT("BlockClick")), EInputEvent::IE_Pressed, this, &AWarriorCharacter::RightBlock);
	PlayerInputComponent->BindAction(FName(TEXT("BlockClick")), EInputEvent::IE_Released, this, &AWarriorCharacter::RightBlockUp);
	PlayerInputComponent->BindAction(FName(TEXT("SkillQ")), EInputEvent::IE_Pressed, this, &AWarriorCharacter::SkillQ);
	PlayerInputComponent->BindAction(FName(TEXT("SkillE")), EInputEvent::IE_Pressed, this, &AWarriorCharacter::SkillE);
	PlayerInputComponent->BindAction(FName(TEXT("SkillR")), EInputEvent::IE_Pressed, this, &AWarriorCharacter::SkillR);

	PlayerInputComponent->BindAction(FName(TEXT("CtrlKey")), EInputEvent::IE_Pressed, this, &AWarriorCharacter::CtrlKeyDown);
	PlayerInputComponent->BindAction(FName(TEXT("CtrlKey")), EInputEvent::IE_Released, this, &AWarriorCharacter::CtrlKeyOn);

	PlayerInputComponent->BindAction(FName(TEXT("ShiftKey")), EInputEvent::IE_Pressed, this, &AWarriorCharacter::ShiftKeyDown);
	PlayerInputComponent->BindAction(FName(TEXT("ShiftKey")), EInputEvent::IE_Released, this, &AWarriorCharacter::ShiftKeyOn);


	PlayerInputComponent->BindAxis(FName(TEXT("WheelAxis")), this, &AWarriorCharacter::WheelKey);


}

void AWarriorCharacter::Jump()
{
	if (GetAnimationInstance()->IsAnimMontage(WarriorJumpAnimation::JumpCombatLoop) ||
		GetAnimationInstance()->IsAnimMontage(WarriorJumpAnimation::JumpStartBackward) ||
		GetAnimationInstance()->IsAnimMontage(WarriorJumpAnimation::JumpStartForward) ||
		GetAnimationInstance()->IsAnimMontage(WarriorJumpAnimation::JumpStartLeft) ||
		GetAnimationInstance()->IsAnimMontage(WarriorJumpAnimation::JumpStartRight) ||
		GetAnimationInstance()->IsAnimMontage(WarriorJumpAnimation::JumpCombatStartBackward) ||
		GetAnimationInstance()->IsAnimMontage(WarriorJumpAnimation::JumpCombatStartForward) ||
		GetAnimationInstance()->IsAnimMontage(WarriorJumpAnimation::JumpCombatStartLeft) ||
		GetAnimationInstance()->IsAnimMontage(WarriorJumpAnimation::JumpCombatStartRight) ||
		IsAttack() || m_IsESkillAttacking)
	{
 		return;
	}

	Super::Jump();

	m_IsJump = true;

	if (!m_IsCombating)
	{
		if (m_IsRightDown && !m_IsForwardDown && !m_IsLeftDown && !m_IsBackwardDown)
		{
			GetAnimationInstance()->ChangeAnimMontage(WarriorJumpAnimation::JumpStartRight);
			m_JumpType = EWarriorJumpType::Right;
		}
		else if (m_IsLeftDown && !m_IsForwardDown && !m_IsRightDown && !m_IsBackwardDown)
		{
			GetAnimationInstance()->ChangeAnimMontage(WarriorJumpAnimation::JumpStartLeft);
			m_JumpType = EWarriorJumpType::Left;
		}
		else if (m_IsForwardDown && !m_IsLeftDown && !m_IsRightDown && !m_IsBackwardDown)
		{
			GetAnimationInstance()->ChangeAnimMontage(WarriorJumpAnimation::JumpStartForward);
			m_JumpType = EWarriorJumpType::Forward;
		}
		else if (m_IsBackwardDown && !m_IsForwardDown && !m_IsRightDown && !m_IsLeftDown)
		{
			GetAnimationInstance()->ChangeAnimMontage(WarriorJumpAnimation::JumpStartBackward);
			m_JumpType = EWarriorJumpType::Backward;
		}
		else
		{
			GetAnimationInstance()->ChangeAnimMontage(WarriorJumpAnimation::JumpStartForward);
			m_JumpType = EWarriorJumpType::Forward;
		}
	}
	else
	{
		if (m_IsRightDown && !m_IsForwardDown && !m_IsLeftDown && !m_IsBackwardDown)
		{
			GetAnimationInstance()->ChangeAnimMontage(WarriorJumpAnimation::JumpCombatStartRight);
			m_JumpType = EWarriorJumpType::Right;
		}
		else if (m_IsLeftDown && !m_IsForwardDown && !m_IsRightDown && !m_IsBackwardDown)
		{
			GetAnimationInstance()->ChangeAnimMontage(WarriorJumpAnimation::JumpCombatStartLeft);
			m_JumpType = EWarriorJumpType::Left;
		}
		else if (m_IsForwardDown && !m_IsLeftDown && !m_IsRightDown && !m_IsBackwardDown)
		{
			GetAnimationInstance()->ChangeAnimMontage(WarriorJumpAnimation::JumpCombatStartForward);
			m_JumpType = EWarriorJumpType::Forward;
		}
		else if (m_IsBackwardDown && !m_IsForwardDown && !m_IsRightDown && !m_IsLeftDown)
		{
			GetAnimationInstance()->ChangeAnimMontage(WarriorJumpAnimation::JumpCombatStartBackward);
			m_JumpType = EWarriorJumpType::Backward;
		}
		else
		{
			GetAnimationInstance()->ChangeAnimMontage(WarriorJumpAnimation::JumpCombatStartForward);
			m_JumpType = EWarriorJumpType::Forward;
		}
	}
	
}

void AWarriorCharacter::StopJumping()
{
	Super::StopJumping();
}

void AWarriorCharacter::CallDamage(double _Damage, AActor* _Actor, bool _IsKnockBack, bool _IsCameraShake)
{
	if (m_IsInvincibility)
	{
		return;
	}

	Super::CallDamage(_Damage, _Actor, _IsKnockBack, _IsCameraShake);

	if (_IsCameraShake)
	{
		CameraShake(CameraShake_Type::HitShake);
	}

	m_HPPercent = GetHP() / m_PlayerInfo->MaxHP;

	AUR_CircleActor* BossCIrcle = nullptr;

	if (_Actor != nullptr)
	{
		//AttackOff();
		if (!IsAttack())
		{
			if (_IsKnockBack)
				HitAnimMontageJudge();
		}
	}
}

void AWarriorCharacter::CommandStructInit()
{
	//m_Command.IsWClicked = false;
	//m_Command.IsWDoubleClicked = false;
	//m_Command.WResetTime = 0.7f;
	//m_Command.WDoubleResetTime = 0.7f;
}

void AWarriorCharacter::SetDefaultData()
{
	// ���ݿ� ����Ǵ� ������� �̸� �����Ѵ�.
	// m_PlayerInfo�� �𸮾� ������ �ּҸ� �����ִ°�.
	SetHP(m_PlayerInfo->HP);
	m_MaxHP = m_PlayerInfo->MaxHP;
	m_MaxHP = 10000000.0;
	m_MPPercent = 1.0;
	m_HPPercent = 1.0;
	m_MP = 10000000.0;
	m_Stamina = m_PlayerInfo->MaxStamina;
	m_QSkillCurCoolTime = 0.f;
	m_ESkillCurCoolTime = 0.f;
	m_RSkillCurCoolTime = 0.f;
	m_QSkillMaxCoolTime = static_cast<int>(m_PlayerInfo->QSkillCollTime);
	m_ESkillMaxCoolTime = static_cast<int>(m_PlayerInfo->ESkillCollTime);
	m_RSkillMaxCoolTime = static_cast<int>(m_PlayerInfo->RSkillCollTime);
}

void AWarriorCharacter::DashToJudge()
{
	//FVector InputVec = GetCharacterMovement()->GetLastInputVector();

	//FHitResult HitResult;
	FVector InputVec;

	if (m_IsRightDown && !m_IsForwardDown && !m_IsLeftDown && !m_IsBackwardDown)
	{
		InputVec = GetActorRightVector();
		GetAnimationInstance()->ChangeAnimMontage(WarriorAnimation::DashRight);
	}
	else if (m_IsLeftDown && !m_IsForwardDown && !m_IsRightDown && !m_IsBackwardDown)
	{
		InputVec = -GetActorRightVector();
		GetAnimationInstance()->ChangeAnimMontage(WarriorAnimation::DashLeft);
	}
	else if (m_IsForwardDown && !m_IsLeftDown && !m_IsRightDown && !m_IsBackwardDown)
	{
		InputVec = GetActorForwardVector();
		GetAnimationInstance()->ChangeAnimMontage(WarriorAnimation::DashForward);
	}
	else if (m_IsBackwardDown && !m_IsForwardDown && !m_IsRightDown && !m_IsLeftDown)
	{
		InputVec = -GetActorForwardVector();
		GetAnimationInstance()->ChangeAnimMontage(WarriorAnimation::DashBackward);
	}
	else if (m_IsRightDown && m_IsForwardDown && !m_IsLeftDown && !m_IsBackwardDown)
	{
		InputVec = GetActorForwardVector() + GetActorRightVector();
		GetAnimationInstance()->ChangeAnimMontage(WarriorAnimation::DashForwardRight);
	}
	else if (m_IsLeftDown && m_IsForwardDown && !m_IsRightDown && !m_IsBackwardDown)
	{
		InputVec = GetActorForwardVector() - GetActorRightVector();
		GetAnimationInstance()->ChangeAnimMontage(WarriorAnimation::DashForwardLeft);
	}
	else if (m_IsRightDown && m_IsBackwardDown && !m_IsLeftDown && !m_IsForwardDown)
	{
		InputVec = -GetActorForwardVector() + GetActorRightVector();
		GetAnimationInstance()->ChangeAnimMontage(WarriorAnimation::DashBackwardRight);
	}
	else if (m_IsLeftDown && m_IsBackwardDown && !m_IsRightDown && !m_IsForwardDown)
	{
		InputVec = -GetActorForwardVector() - GetActorRightVector();
		GetAnimationInstance()->ChangeAnimMontage(WarriorAnimation::DashBackwardLeft);
	}
	if (!m_OnDash)
	{
		FVector EndVec = InputVec * 9000.f;
		EndVec.Z = 0.f;

		LaunchCharacter(EndVec, true, true);
		m_OnDash = true;
		GetWorldTimerManager().SetTimer(m_TimerHandle, this, &AWarriorCharacter::StopDashing, 0.1f, false);
	}

	// �� �����ϴ� �뽬 ����ϰ� ������ ��Ȳ�� �°� �����ؼ� ��밡��

	/*bool BlockingHit = false;
	bool InitialOverlap = false;
	float Time = 0.f;
	float Distance = 0.f;
	FVector Location;
	FVector ImpactPoint;
	FVector Normal;
	FVector ImpactNormal;
	UPhysicalMaterial* PhysMat = nullptr;
	AActor* HitActor = nullptr;
	UPrimitiveComponent* HitComponent = nullptr;
	FName HitBoneName;
	FName BoneName;
	int32 HitItem;
	int32 ElementIndex;
	int32 FaceIndex;
	FVector TraceStart;
	FVector TraceEnd;*/

	//if (UAIBlueprintHelperLibrary::IsValidAIDirection(InputVec))
	//{
	//	FVector EndVec = InputVec * 800.f + GetActorLocation();

	//	/*if (GetWorld()->LineTraceSingleByChannel(HitResult, GetActorLocation(), EndVec, ECollisionChannel::ECC_WorldStatic))
	//	{
	//		UGameplayStatics::BreakHitResult(HitResult, BlockingHit, InitialOverlap, Time, Distance, Location,
	//			ImpactPoint, Normal, ImpactNormal, PhysMat, HitActor, HitComponent, HitBoneName, BoneName, HitItem,
	//			ElementIndex, FaceIndex, TraceStart, TraceEnd);

	//		DashFunc((InputVec * 100.f) + Location, InputVec);
	//	}
	//	else
	//	{
	//		UGameplayStatics::BreakHitResult(HitResult, BlockingHit, InitialOverlap, Time, Distance, Location,
	//			ImpactPoint, Normal, ImpactNormal, PhysMat, HitActor, HitComponent, HitBoneName, BoneName, HitItem,
	//			ElementIndex, FaceIndex, TraceStart, TraceEnd);

	//		DashFunc(TraceEnd, InputVec);
	//	}*/
	//}
	//else
	//{
	//	FVector EndVec = GetActorForwardVector() * 800.f + GetActorLocation();


	//	if (GetWorld()->LineTraceSingleByChannel(HitResult, GetActorLocation(), EndVec, ECollisionChannel::ECC_WorldStatic))
	//	{
	//		UGameplayStatics::BreakHitResult(HitResult, BlockingHit, InitialOverlap, Time, Distance, Location,
	//			ImpactPoint, Normal, ImpactNormal, PhysMat, HitActor, HitComponent, HitBoneName, BoneName, HitItem,
	//			ElementIndex, FaceIndex, TraceStart, TraceEnd);

	//		DashFunc((InputVec * 800.f) + Location, InputVec);
	//	}
	//}
	
}
 
void AWarriorCharacter::CombatTick(float DeltaTime)
{
	if (m_IsCombating)
	{
		m_CombatTime -= DeltaTime;

		if ((m_CombatTime <= 0.f) && GetAnimationInstance()->IsAnimMontage(WarriorCombatAnimation::CombatIdle))
		{
			if (!GetAnimationInstance()->IsAnimMontage(WarriorAnimation::Execution1) &&
				!GetAnimationInstance()->IsAnimMontage(WarriorAnimation::Execution2))
			{
				m_CombatTime = 7.f;

				m_IsCombating = false;

				GetAnimationInstance()->ChangeAnimMontage(DefaultAnimation::Idle);
			}
		}
	}
}

void AWarriorCharacter::CoolTimeTick(float DeltaTime)
{
	if (m_IsQSkill)
	{
		m_QSkillCurCoolTime += DeltaTime;

		if (m_QSkillCurCoolTime >= m_QSkillMaxCoolTime)
		{
			m_QSkillCurCoolTime = 0.f;
			m_IsQSkill = false;
			m_AttackSpeed = 1.f;
			SetBerserkRateScale();
			DeletePostProcessGlowMat();

			for (int32 i = 0; i < m_ArrayNormalMat.Num(); ++i)
			{
				GetMesh()->SetMaterial(i, m_ArrayNormalMat[i]);
			}
		}
	}
	if (m_IsESkillAttacking)
	{
		m_ESkillCurCoolTime += DeltaTime;

		if (m_ESkillCurCoolTime >= m_ESkillMaxCoolTime)
		{
			m_ESkillCurCoolTime = 0.f;
			m_IsESkillAttacking = false;
			m_IsCombating = true;
			m_CombatTime = 7.f;

			GetAnimationInstance()->ChangeAnimMontage(WarriorAnimation::SkillEEnd);
		}
	}
	if (m_IsRSkillAttacking)
	{
		m_RSkillCurCoolTime += DeltaTime;

		if (m_RSkillCurCoolTime >= m_RSkillMaxCoolTime)
		{
			m_RSkillCurCoolTime = 0.f;
			m_IsRSkillAttacking = false;
		}
	}
}

bool AWarriorCharacter::FirstJudgeFunc()
{
	if (GetAnimationInstance()->IsAnimMontage(WarriorAnimation::SkillQ) ||
		GetAnimationInstance()->IsAnimMontage(WarriorHitAnimation::HitLargeToFallDown) ||
		GetAnimationInstance()->IsAnimMontage(WarriorHitAnimation::CombatHitLargeToFallDown) ||
		GetAnimationInstance()->IsAnimMontage(WarriorHitAnimation::GetUp) ||
		GetAnimationInstance()->IsAnimMontage(WarriorHitAnimation::CombatGetUp) ||
		m_IsJump)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool AWarriorCharacter::JudgeFunc()
{
	if (GetAnimationInstance()->IsAnimMontage(WarriorAnimation::DashLeft) ||
		GetAnimationInstance()->IsAnimMontage(WarriorAnimation::DashRight) ||
		GetAnimationInstance()->IsAnimMontage(WarriorAnimation::DashForward) ||
		GetAnimationInstance()->IsAnimMontage(WarriorAnimation::DashBackward) ||
		GetAnimationInstance()->IsAnimMontage(WarriorAnimation::DashForwardLeft) ||
		GetAnimationInstance()->IsAnimMontage(WarriorAnimation::DashForwardRight) ||
		GetAnimationInstance()->IsAnimMontage(WarriorAnimation::DashBackwardLeft) ||
		GetAnimationInstance()->IsAnimMontage(WarriorAnimation::DashBackwardRight) ||
		GetAnimationInstance()->IsAnimMontage(WarriorAnimation::ComboA1) ||
		GetAnimationInstance()->IsAnimMontage(WarriorAnimation::ComboA2) ||
		GetAnimationInstance()->IsAnimMontage(WarriorAnimation::ComboA3) ||
		GetAnimationInstance()->IsAnimMontage(DefaultAnimation::Attack) ||
		GetAnimationInstance()->IsAnimMontage(WarriorAnimation::ComboB1) ||
		GetAnimationInstance()->IsAnimMontage(WarriorAnimation::ComboB2) ||
		GetAnimationInstance()->IsAnimMontage(WarriorAnimation::ComboB3) ||
		GetAnimationInstance()->IsAnimMontage(WarriorAnimation::ComboB4) ||
		GetAnimationInstance()->IsAnimMontage(WarriorAnimation::ComboC1) ||
		GetAnimationInstance()->IsAnimMontage(WarriorAnimation::ComboC2) ||
		GetAnimationInstance()->IsAnimMontage(WarriorAnimation::ComboC3) ||
		GetAnimationInstance()->IsAnimMontage(WarriorAnimation::ComboC4) ||
		GetAnimationInstance()->IsAnimMontage(WarriorAnimation::SkillELoop) ||
		GetAnimationInstance()->IsAnimMontage(WarriorAnimation::CommandDashAttack) ||
		GetAnimationInstance()->IsAnimMontage(WarriorBlockAnimation::BlockHit) ||
		GetAnimationInstance()->IsAnimMontage(WarriorAnimation::SkillRStart) ||
		GetAnimationInstance()->IsAnimMontage(WarriorAnimation::SkillQ) ||
		m_IsJump || m_IsESkillAttacking)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool AWarriorCharacter::DashJudgeFunc()
{
	if (GetAnimationInstance()->IsAnimMontage(WarriorAnimation::DashLeft) ||
		GetAnimationInstance()->IsAnimMontage(WarriorAnimation::DashRight) ||
		GetAnimationInstance()->IsAnimMontage(WarriorAnimation::DashForward) ||
		GetAnimationInstance()->IsAnimMontage(WarriorAnimation::DashBackward) ||
		GetAnimationInstance()->IsAnimMontage(WarriorAnimation::DashForwardLeft) ||
		GetAnimationInstance()->IsAnimMontage(WarriorAnimation::DashForwardRight) ||
		GetAnimationInstance()->IsAnimMontage(WarriorAnimation::DashBackwardLeft) ||
		GetAnimationInstance()->IsAnimMontage(WarriorAnimation::DashBackwardRight) ||
		m_IsESkillAttacking)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void AWarriorCharacter::StopDashing()
{
	GetCharacterMovement()->StopMovementImmediately();
	GetWorldTimerManager().SetTimer(m_TimerHandle, this, &AWarriorCharacter::ResetDash, 0.2f, false);
	GetCharacterMovement()->BrakingFrictionFactor = 2.f;
}

void AWarriorCharacter::DashFunc(const FVector& DashDir, const FVector& DashVelocity)
{
	m_DashDir = DashDir;
	m_DashVelocity = DashVelocity;
	PlayTimeline();
}


void AWarriorCharacter::JumpTrace()
{
	/*if (!GetCharacterMovement()->IsFalling() && m_IsUltimateAttack)
	{
		GetAnimationInstance()->ChangeAnimMontage(WarriorAnimation::SkillREnd);
	}*/
	if (m_IsJump && !GetCharacterMovement()->IsFalling())
	{
		m_IsJump = false;

		if (!m_IsCombating)
		{
			switch (m_JumpType)
			{
			case EWarriorJumpType::Forward:
				GetAnimationInstance()->ChangeAnimMontage(WarriorJumpAnimation::JumpEndForward);
				break;
			case EWarriorJumpType::Backward:
				GetAnimationInstance()->ChangeAnimMontage(WarriorJumpAnimation::JumpEndBackward);
				break;
			case EWarriorJumpType::Left:
				GetAnimationInstance()->ChangeAnimMontage(WarriorJumpAnimation::JumpEndLeft);
				break;
			case EWarriorJumpType::Right:
				GetAnimationInstance()->ChangeAnimMontage(WarriorJumpAnimation::JumpEndRight);
				break;
			}
		}
		else
		{
			switch (m_JumpType)
			{
			case EWarriorJumpType::Forward:
				GetAnimationInstance()->ChangeAnimMontage(WarriorJumpAnimation::JumpCombatEndForward);
				break;
			case EWarriorJumpType::Backward:
				GetAnimationInstance()->ChangeAnimMontage(WarriorJumpAnimation::JumpCombatEndBackward);
				break;
			case EWarriorJumpType::Left:
				GetAnimationInstance()->ChangeAnimMontage(WarriorJumpAnimation::JumpCombatEndLeft);
				break;
			case EWarriorJumpType::Right:
				GetAnimationInstance()->ChangeAnimMontage(WarriorJumpAnimation::JumpCombatEndRight);
				break;
			}
		}
		
	}
}

void AWarriorCharacter::CommandTimeJudge(float DeltaTime)
{
	//if (*m_qCommand.Peek() == 'w')
	//{
	//	m_qCommandTime -= DeltaTime;

	//	if (0.f >= m_qCommandTime)
	//	{
	//		m_qCommandTime = 0.5f;

	//		// ���� ��������� (¥�� �Ѱ��� ����)
	//		m_qCommand.Empty();
	//	}
	//}
}

void AWarriorCharacter::HitAnimMontageJudge()
{
	if (IsAttack() || JudgeFunc())
	{
		return;
	}
	const int Random = static_cast<int>(m_Stream.FRandRange(1, 6));

	switch (m_HitType)
	{
	case EHitType::NormalHit:
	{
		switch (Random)
		{
		case 1:
		case 2:
		case 3:
		case 4:
			HitAnimation(false);
			break;
		case 5:
		case 6:
			DestroyBlockObj();
			HitAnimation(true);
			break;
		}
	}
		break;
	case EHitType::KnockDownHit:
	{
		DestroyBlockObj();
		HitAnimation(true, true);
	}
		break;
	}
	
}

void AWarriorCharacter::HitAnimation(bool IsLarge, bool IsKncokDown)
{
	if (!IsKncokDown)
	{
		if (!IsLarge)
		{
			if (!HitKnockDownAnimJudge())
			{
				switch (m_HitDir)
				{
				case EHitDir::Forward:
					// ���鿡�� �������� ���ŷ�� �Ǳ⶧���� Forward���� ���ŷ ó��
					if (!m_IsBlocking)
					{
						if (!m_IsCombating)
							GetAnimationInstance()->ChangeAnimMontage(WarriorHitAnimation::HitForward);
						else
							GetAnimationInstance()->ChangeAnimMontage(WarriorHitAnimation::CombatHitForward);
					}
					else
					{
						GetAnimationInstance()->ChangeAnimMontage(WarriorBlockAnimation::BlockHit);
					}
					break;
				case EHitDir::Backward:
					if (!m_IsCombating)
						GetAnimationInstance()->ChangeAnimMontage(WarriorHitAnimation::HitBackward);
					else
						GetAnimationInstance()->ChangeAnimMontage(WarriorHitAnimation::CombatHitBackward);
					break;
				case EHitDir::Left:
					if (!m_IsCombating)
						GetAnimationInstance()->ChangeAnimMontage(WarriorHitAnimation::HitLeft);
					else
						GetAnimationInstance()->ChangeAnimMontage(WarriorHitAnimation::CombatHitLeft);
					break;
				case EHitDir::Right:
					if (!m_IsCombating)
						GetAnimationInstance()->ChangeAnimMontage(WarriorHitAnimation::HitRight);
					else
						GetAnimationInstance()->ChangeAnimMontage(WarriorHitAnimation::CombatHitRight);
					break;
				}
			}
		}
		else
		{
			switch (m_HitDir)
			{
			case EHitDir::Forward:
				if (!m_IsCombating)
					GetAnimationInstance()->ChangeAnimMontage(WarriorHitAnimation::HitLargeForward);
				else
					GetAnimationInstance()->ChangeAnimMontage(WarriorHitAnimation::CombatHitLargeForward);
				break;
			case EHitDir::Backward:
				if (!m_IsCombating)
					GetAnimationInstance()->ChangeAnimMontage(WarriorHitAnimation::HitLargeBackward);
				else
					GetAnimationInstance()->ChangeAnimMontage(WarriorHitAnimation::CombatHitLargeBackward);
				break;
			case EHitDir::Left:
				if (!m_IsCombating)
					GetAnimationInstance()->ChangeAnimMontage(WarriorHitAnimation::HitLargeLeft);
				else
					GetAnimationInstance()->ChangeAnimMontage(WarriorHitAnimation::CombatHitLargeLeft);
				break;
			case EHitDir::Right:
				if (!m_IsCombating)
					GetAnimationInstance()->ChangeAnimMontage(WarriorHitAnimation::HitLargeRight);
				else
					GetAnimationInstance()->ChangeAnimMontage(WarriorHitAnimation::CombatHitLargeRight);
				break;
			}
		}
	}
	else
	{
		if (!m_IsCombating)
			GetAnimationInstance()->ChangeAnimMontage(WarriorHitAnimation::HitLargeToFallDown);
		else
			GetAnimationInstance()->ChangeAnimMontage(WarriorHitAnimation::CombatHitLargeToFallDown);
	}
}

bool AWarriorCharacter::HitKnockDownAnimJudge()
{
	if (GetAnimationInstance()->IsAnimMontage(WarriorHitAnimation::HitLargeForward) ||
		GetAnimationInstance()->IsAnimMontage(WarriorHitAnimation::HitLargeBackward) ||
		GetAnimationInstance()->IsAnimMontage(WarriorHitAnimation::HitLargeLeft) ||
		GetAnimationInstance()->IsAnimMontage(WarriorHitAnimation::HitLargeRight) ||
		GetAnimationInstance()->IsAnimMontage(WarriorHitAnimation::CombatHitLargeForward) ||
		GetAnimationInstance()->IsAnimMontage(WarriorHitAnimation::CombatHitLargeBackward) ||
		GetAnimationInstance()->IsAnimMontage(WarriorHitAnimation::CombatHitLargeRight) ||
		GetAnimationInstance()->IsAnimMontage(WarriorHitAnimation::CombatHitLargeToFallDown) ||
		GetAnimationInstance()->IsAnimMontage(WarriorHitAnimation::GetUp) ||
		GetAnimationInstance()->IsAnimMontage(WarriorHitAnimation::HitLargeToFallDown) ||
		GetAnimationInstance()->IsAnimMontage(WarriorHitAnimation::CombatHitLargeToFallDown))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void AWarriorCharacter::BlockStaminaTick(float DeltaTime)
{
	if (m_IsBlocking)
	{
		if (m_MP - DeltaTime > 0.f)
		{
			m_MP -= DeltaTime;
		}
		if (m_MP <= 0.f)
		{
			m_MP = 0.f;
			m_IsBlocking = false;
		}
	}

	m_MPPercent = m_MP / m_PlayerInfo->MaxMP;
}

AActor* AWarriorCharacter::GetTargetActor()
{
	AActor* TargetMonster = nullptr;
	FVector StartPos = GetActorLocation();
	StartPos.Z -= 100.f;
	FVector EndPos = GetActorLocation();
	EndPos.Z += 100.f;
	
	TArray<TEnumAsByte<EObjectTypeQuery>> ArrayPawn;
	TEnumAsByte<EObjectTypeQuery> ObjectTypePawn = UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel2); // Monster Collision ä��
	ArrayPawn.Add(ObjectTypePawn);

	TArray<AActor*> IgnoreActors; // ������ ���͵�.

	TArray<FHitResult> HitResult; // ��Ʈ ��� �� ���� ����.

	if (UKismetSystemLibrary::SphereTraceMultiForObjects(GetWorld(), StartPos, EndPos, 2000.f, ArrayPawn, true, IgnoreActors,
		EDrawDebugTrace::None, HitResult, true))
	{
		// ���⼭ ������ ���ؼ� ���� �����Ѵ�.
		TArray<AActor*> TargetActor = CheckAttackTarget(HitResult);

		double DistValue = 100000.0;
		// ���⼭ �Ÿ��� ���� ���������� �Ǻ����Ѵ�..
		for (auto& Monster : TargetActor)
		{
			double Dist = (GetActorLocation() - Monster->GetActorLocation()).Size();

			if (Dist < DistValue)
			{
				DistValue = std::min(DistValue, Dist);
				TargetMonster = Monster;
			}
		}
	}

	return TargetMonster;
}

void AWarriorCharacter::CameraShake(CameraShake_Type _Type)
{
	if (m_CameraShakeMap.IsEmpty())
	{
		return;
	}

	if(!GetWorld() || !GetWorld()->GetFirstPlayerController() || !GetWorld()->GetFirstPlayerController()->PlayerCameraManager)
	{
		return;
	}

	switch (_Type)
	{
	case CameraShake_Type::HitShake:
		GetWorld()->GetFirstPlayerController()->PlayerCameraManager->PlayWorldCameraShake(GetWorld(),
			m_CameraShakeMap[_Type],
			m_CameraComponent->GetComponentLocation(),
			0.f, 500.f, 1.f);
		break;
	case CameraShake_Type::UltimateShake:
		GetWorld()->GetFirstPlayerController()->PlayerCameraManager->PlayWorldCameraShake(GetWorld(),
			m_CameraShakeMap[_Type],
			m_CameraComponent->GetComponentLocation(),
			0.f, 500.f, 1.f);
		break;
	case CameraShake_Type::BigShake:
		GetWorld()->GetFirstPlayerController()->PlayerCameraManager->PlayWorldCameraShake(GetWorld(),
			m_CameraShakeMap[_Type],
			m_CameraComponent->GetComponentLocation(),
			0.f, 500.f, 1.f);
		break;
	}
}

void AWarriorCharacter::AddPostProcessDrunkMat()
{
	if (m_DrunkPostProcessComponent->Settings.WeightedBlendables.Array.Num() == 0)
		m_DrunkPostProcessComponent->Settings.WeightedBlendables.Array.Add(m_DrunkWeightedBlend);
}

void AWarriorCharacter::DeletePostProcessDrunkMat()
{
	if (m_DrunkPostProcessComponent->Settings.WeightedBlendables.Array.Num() == 1)
		m_DrunkPostProcessComponent->Settings.WeightedBlendables.Array.RemoveAt(0);
}

void AWarriorCharacter::AddPostProcessGlowMat()
{
	if (m_GlowPostProcessComponent->Settings.WeightedBlendables.Array.Num() == 0)
		m_GlowPostProcessComponent->Settings.WeightedBlendables.Array.Add(m_GlowWeightedBlend);
}

void AWarriorCharacter::DeletePostProcessGlowMat()
{
	if (m_GlowPostProcessComponent->Settings.WeightedBlendables.Array.Num() == 1)
		m_GlowPostProcessComponent->Settings.WeightedBlendables.Array.RemoveAt(0);
}

TArray<AActor*> AWarriorCharacter::CheckAttackTarget(const TArray<FHitResult>& _HitResult)
{
	TArray<AActor*> TargetActor;

	for (auto& Hit : _HitResult)
	{
		// ���� ���� �༮�� �־���.
		if (Hit.GetActor()->GetInstigator<AURCharacter>()->IsDeath())
			continue;

		FVector MonsterDir = Hit.GetActor()->GetActorLocation() - GetActorLocation();
		MonsterDir = MonsterDir.GetSafeNormal();

		double Dot = FVector::DotProduct(MonsterDir, GetActorForwardVector());

		// �������� 0.7�̻��� �༮�鸸 ������
		if (Dot != 0.0)
		{
			TargetActor.Add(Hit.GetActor());
		}
	}

	return TargetActor;
}

void AWarriorCharacter::SetBerserkRateScale()
{
	GetAnimationInstance()->GetAnimation(DefaultAnimation::Attack)->RateScale = m_AttackSpeed;
	GetAnimationInstance()->GetAnimation(WarriorAnimation::ComboA1)->RateScale = m_AttackSpeed;
	GetAnimationInstance()->GetAnimation(WarriorAnimation::ComboA2)->RateScale = m_AttackSpeed;
	GetAnimationInstance()->GetAnimation(WarriorAnimation::ComboA3)->RateScale = m_AttackSpeed;
	GetAnimationInstance()->GetAnimation(WarriorAnimation::ComboB1)->RateScale = m_AttackSpeed;
	GetAnimationInstance()->GetAnimation(WarriorAnimation::ComboB2)->RateScale = m_AttackSpeed;
	GetAnimationInstance()->GetAnimation(WarriorAnimation::ComboB3)->RateScale = m_AttackSpeed;
	GetAnimationInstance()->GetAnimation(WarriorAnimation::ComboB4)->RateScale = m_AttackSpeed;
	GetAnimationInstance()->GetAnimation(WarriorAnimation::ComboC1)->RateScale = m_AttackSpeed;
	GetAnimationInstance()->GetAnimation(WarriorAnimation::ComboC2)->RateScale = m_AttackSpeed;
	GetAnimationInstance()->GetAnimation(WarriorAnimation::ComboC3)->RateScale = m_AttackSpeed;
	GetAnimationInstance()->GetAnimation(WarriorAnimation::ComboC4)->RateScale = m_AttackSpeed;
	GetAnimationInstance()->GetAnimation(WarriorAnimation::SkillELoop)->RateScale = m_AttackSpeed;
	
}

void AWarriorCharacter::TickPostProcessDeleteFunc(float DeltaTime)
{
	if (m_DrunkPostProcessComponent->Settings.WeightedBlendables.Array.Num() == 1)
	{
		m_PostProcessDeleteTime -= DeltaTime;

		if (m_PostProcessDeleteTime <= 0.f)
		{
			m_PostProcessDeleteTime = 5.f;
			m_DrunkPostProcessComponent->Settings.WeightedBlendables.Array.RemoveAt(0);
		}
	}
}

void AWarriorCharacter::DestroyBlockObj()
{
	m_IsBlocking = false;

	GetCharacterMovement()->MaxWalkSpeed = m_PlayerInfo->Speed;

	if (m_Shield != nullptr)
	{
		m_Shield->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		m_Shield->DestroyComponent();
	}
}


void AWarriorCharacter::TimelineCallback(float val)
{
	FVector NewLocation = UKismetMathLibrary::VLerp(GetActorLocation(), m_DashDir, val);

	SetActorLocation(NewLocation);
}

void AWarriorCharacter::TimelineFinishedCallback()
{
	GetCharacterMovement()->Velocity = m_DashVelocity * 500.f;
}

void AWarriorCharacter::PlayTimeline()
{
	if (MyTimeline != NULL)
	{
		MyTimeline->PlayFromStart();
	}
}

void AWarriorCharacter::TraceAttackMonster()
{
	AActor* TargetMonster = nullptr;
	FVector StartPos = GetActorLocation();
	FVector EndPos = GetActorLocation() + GetActorForwardVector() * 2500.f;


	TArray<TEnumAsByte<EObjectTypeQuery>> ArrayPawn;
	TEnumAsByte<EObjectTypeQuery> ObjectTypePawn = UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel2); // Monster Collision ä��
	ArrayPawn.Add(ObjectTypePawn);

	TArray<AActor*> IgnoreActors; // ������ ���͵�.

	TArray<FHitResult> HitResult; // ��Ʈ ��� �� ���� ����.

	if (UKismetSystemLibrary::LineTraceMultiForObjects(GetWorld(), StartPos, EndPos, ArrayPawn, true, IgnoreActors,
		EDrawDebugTrace::None, HitResult, true))
	{
		for (auto& Target : HitResult)
		{
			// ���� ���� �༮�� �־���.
			if (Target.GetActor()->GetInstigator<AURCharacter>()->IsDeath())
				continue;

			AURCharacter* Monster = Cast<AURCharacter>(Target.GetActor());
			Monster->CallDamage(m_PlayerInfo->MaxAttack, this);
			Monster->SetHitType(EHitType::NormalHit);
			CreateParticleObject<AUR_NormalAttackHit>(Monster);
		}

	}
}

TArray<AURCharacter*> AWarriorCharacter::GetSphereTraceHitActor(float _MinMax, float _Radius, ECollisionChannel _CollisionChannel)
{
	TArray<AURCharacter*> HitMonster;
	AActor* TargetMonster = nullptr;
	FVector StartPos = GetActorLocation();
	StartPos.Z -= _MinMax;
	FVector EndPos = GetActorLocation();
	EndPos.Z += _MinMax;

	TArray<TEnumAsByte<EObjectTypeQuery>> ArrayPawn;
	TEnumAsByte<EObjectTypeQuery> ObjectTypePawn = UEngineTypes::ConvertToObjectType(_CollisionChannel); // Monster Collision ä��
	ArrayPawn.Add(ObjectTypePawn);

	TArray<AActor*> IgnoreActors; // ������ ���͵�.

	TArray<FHitResult> HitResult; // ��Ʈ ��� �� ���� ����.

	if (UKismetSystemLibrary::SphereTraceMultiForObjects(GetWorld(), StartPos, EndPos, _Radius, ArrayPawn, true, IgnoreActors,
		EDrawDebugTrace::None, HitResult, true))
	{
		for (auto& Monster : HitResult)
		{
			// ���� ���� �༮�� �־���.
			if (Monster.GetActor()->GetInstigator<AURCharacter>()->IsDeath())
				continue;

			HitMonster.Add(Monster.GetActor()->GetInstigator<AURCharacter>());
		}
	}

	return HitMonster;
}

void AWarriorCharacter::UltimateAttack()
{
	if (m_UltimateAttackCount <= 0)
	{
		m_IsUltimateAttack = false;
		m_CameraSpringArmComponent->TargetArmLength = 800;
		return;
	}

	m_IsUltimateAttack = true;
	m_CameraSpringArmComponent->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	//
	m_CameraSpringArmComponent->SetWorldLocation(GetActorLocation() + (-GetActorRightVector() * 300.f) + (FVector(0.0, 0.0, 1.0) * 300.f));
	float WaitTime = 0.15f; //�ð��� �����ϰ�
	GetWorld()->GetTimerManager().SetTimer(
		m_TimerHandle,    // TimerHandle
		this,    // FTimerDelegate ��������Ʈ ���� ��ü
		&AWarriorCharacter::AdvanceTimer,    // FTimerDelegate (TimerManager.h Ȯ��)
		WaitTime,    // Interval
		true,    // �ݺ� ���� (false : 1ȸ��)
		0.f    // ���� Interval, �̰��� 0�̸� Interval�� ����
	);
}

void AWarriorCharacter::Finished()
{
	
}

void AWarriorCharacter::AdvanceTimer()
{
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	//LatentInfo.ExecutionFunction = FName("Finished");
	//LatentInfo.Linkage = 0;
	//LatentInfo.UUID = GetNextUUID();

	// ���� �迭������ ���Դµ� 0�̶�� Ÿ���� �� �׾��ٴ� �ǹ��̹Ƿ� �̴�� ������Ѿ��Ѵ�.
	if (m_UltimateTargetMonster.IsEmpty())
	{
		m_CameraSpringArmComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		m_CameraSpringArmComponent->SetWorldLocation(GetActorLocation() + FVector(0.0, 0.0, 1.0) * 272.0);
		m_CameraSpringArmComponent->bUsePawnControlRotation = true;
		m_CameraSpringArmComponent->bInheritPitch = true;
		GetWorldSettings()->SetTimeDilation(0.3f);
		GetAnimationInstance()->ChangeAnimMontage(WarriorAnimation::SkillRLoop);
		//m_UltimateCameraTarget = nullptr;
		AUR_UltimateCharge* Spline = GetCreateParticleObject<AUR_UltimateCharge>(this);
		CameraShake(CameraShake_Type::UltimateShake);

		m_CameraSpringArmComponent->TargetArmLength = 800;
		GetWorldTimerManager().ClearTimer(m_TimerHandle);
		return;
	}

	if (m_UltimateTargetCount <= 0)
	{
		m_UltimateTargetCount = m_UltimateTargetMonster.Num();
	}

	AURCharacter* Target = m_UltimateTargetMonster[m_UltimateTargetCount - 1];

	if (Target)
	{
		FVector Dir = Target->GetActorLocation() - GetActorLocation();
		Dir = Dir.GetSafeNormal();

		FVector EndPos;

		if (m_UltimateTargetMonster.Num() > 1)
		{
			if (m_UltimateAttackCount > 1)
			{
				EndPos = Target->GetActorLocation() + Dir * 250.f;
			}
			else
			{
				EndPos = Target->GetActorLocation() - Dir * 150.f;
			}
		}
		else
		{
			if (m_UltimateAttackCount > 1)
			{
				EndPos = Target->GetActorLocation() + Dir * 250.f + GetActorRightVector() * 400.f;
			}
			else
			{
				EndPos = Target->GetActorLocation() - Dir * 150.f;
			}
		}

		CreateParticleObject<AUR_NormalAttackHit>(Target);
		Target->SetHitType(EHitType::NormalHit);

		bool ReTarget = false;

		Target->CallDamage(m_PlayerInfo->MaxAttack, this, false);
		Target->CharacterSoundPlay(m_HitSound, Target->GetRootComponent(), FName(TEXT("root")), 0.3f, 1.f);

		GetAnimationInstance()->ChangeAnimMontage(WarriorAnimation::SkillRAttack);


		FRotator PlayerRot = Dir.Rotation();

		GetController()->SetControlRotation(PlayerRot);

		//AUR_UltimateSpline* Spline = GetCreateHitObject<AUR_UltimateSpline>(this);

		//Spline->SetActorRotation(GetActorRotation() + FRotator(90.0, 0.0, 0.0));

		UKismetSystemLibrary::MoveComponentTo(GetRootComponent(), EndPos, GetActorRotation(),
			false, false, 0.1f, true, EMoveComponentAction::Type::Move, LatentInfo);

		CharacterSoundPlay(m_Sound.UltimateDash, RootComponent, FName(TEXT("root")), 0.1f, 1.f);

		CameraShake(CameraShake_Type::UltimateShake);
	}
	else
	{

	}

	--m_UltimateAttackCount;
	--m_UltimateTargetCount;

	if (m_UltimateAttackCount <= 0)
	{
		m_CameraSpringArmComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		m_CameraSpringArmComponent->SetWorldLocation(GetActorLocation() + FVector(0.0, 0.0, 1.0) * 272.0);
		m_CameraSpringArmComponent->bUsePawnControlRotation = true;
		m_CameraSpringArmComponent->bInheritPitch = true;
		GetWorldSettings()->SetTimeDilation(0.3f);
		GetAnimationInstance()->ChangeAnimMontage(WarriorAnimation::SkillRLoop);
		//m_UltimateCameraTarget = nullptr;
		AUR_UltimateCharge* Spline = GetCreateParticleObject<AUR_UltimateCharge>(this);
		CameraShake(CameraShake_Type::UltimateShake);

		m_CameraSpringArmComponent->TargetArmLength = 800;
		// ī��Ʈ�ٿ��� �Ϸ�Ǹ� Ÿ�̸Ӹ� ����
		GetWorldTimerManager().ClearTimer(m_TimerHandle);
	}
}

void AWarriorCharacter::TurnFunc()
{
	FRotator PlayerRot = GetControlRotation();
	FRotator CameraRot = m_CameraComponent->GetComponentRotation();
	FRotator Rot = FMath::RInterpTo(GetControlRotation(), CameraRot,
		GetWorld()->GetDeltaSeconds(), 5.f);

	Rot.Roll = PlayerRot.Roll;
	Rot.Pitch = PlayerRot.Pitch;

	GetController()->SetControlRotation(Rot);
}