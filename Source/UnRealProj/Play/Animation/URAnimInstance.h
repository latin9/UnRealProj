// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "URAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJ_API UURAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UURAnimInstance();

	void AddAnimMontage(int _Key, UAnimMontage* _Montage);

	template<typename EnumType>
	void SetDefaultMontage(EnumType _Key)
	{
		SetDefaultMontage(static_cast<int>(_Key));
	}

	void SetDefaultMontage(int _Key);


	template<typename EnumType>
	void ChangeAnimMontage(EnumType _Key, bool Force = false)
	{
		ChangeAnimMontage(static_cast<int>(_Key), Force);
	}

	UFUNCTION(BlueprintCallable, Category = UR)
	void ChangeAnimMontage(int _Key, bool Force = false);

	template<typename EnumType>
	bool IsAnimMontage(EnumType _Key)	const
	{
		return IsAnimMontage(static_cast<int>(_Key));
	}

	bool IsAnimMontage(int _Key) const;

	template<typename EnumType>
	UAnimMontage* GetAnimation(EnumType _Key)
	{
		return GetAnimation(static_cast<int>(_Key));
	}

	UAnimMontage* GetAnimation(int _Key);

	UAnimMontage* GetCurrentMontage()
	{
		UAnimMontage** Anim = m_Animations.Find(m_CurrentAnimationKey);

		if (nullptr == Anim)
		{
			return nullptr;
		}

		return *Anim;
	}

	int FindMontageKey(UAnimMontage* _Montage);

	void SetCurrentKey(int _Key)
	{
		m_CurrentAnimationKey = _Key;
	}

private:
	// �𸮾��� ���� �ؽ�
	// �𸮾��� �� ���� ���̰�
	UPROPERTY(Category = "AnimationData", EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TMap<int, UAnimMontage*> m_Animations;

	int m_CurrentAnimationKey;

	int	m_DefaultKey;
	float m_Time;


protected:
	// �̳༮�� ��������� �̴ϼȶ����� �� ������ �ִٸ� �̳༮����
	void NativeBeginPlay() override;

	void NativeUpdateAnimation(float DeltaSeconds) override;
	
};
