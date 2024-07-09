// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

namespace TestProject
{
	/**
	 *
	 */
	class TESTPROJECT_API Battery
	{
	public:
		Battery() = default;
		explicit Battery(float PercentIn);

		void Charge();
		void Discharge();

		float GetPercent() const;
		FColor GetColor() const;
		FString ToString() const;

		bool operator>=(const Battery& Rhs);
		bool operator==(const Battery& Rhs);
	private:
		void SetPercent(float PercentIn);
	private:
		float Percent{ 1.0f };
	};
}
