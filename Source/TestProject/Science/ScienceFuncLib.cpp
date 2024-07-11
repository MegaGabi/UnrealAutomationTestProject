// Fill out your copyright notice in the Description page of Project Settings.


#include "Science/ScienceFuncLib.h"

int32 UScienceFuncLib::Fibonacci(int32 Value)
{
	//check(Value >= 0);
	if (Value < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid input for Fibonacci: %i"), Value);
	}
	return Value <= 1 ? Value : Fibonacci(Value - 1) + Fibonacci(Value - 2);
}

int32 UScienceFuncLib::Factorial(int32 Value)
{
	if (Value < 0) return -1;
	
	int32 Fac = 1;
	for (int32 i = 1; i <= Value; ++i)
	{
		Fac *= i;
	}

	return Fac;
}


