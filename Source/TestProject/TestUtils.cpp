#include "Tests/TestUtils.h"

namespace TestProject
{
	void CallFuncByNameWithParams(UObject* Object, const FString& FuncName, const TArray<FString>& Params)
	{
		if (!Object) return;

		FString Command = FString::Printf(TEXT("%s"), *FuncName);
		for (const auto Param : Params)
		{
			Command.Append(" ").Append(Param);
		}
		FOutputDeviceNull OutputDeviceNull;
		Object->CallFunctionByNameWithArguments(*Command, OutputDeviceNull, nullptr, true);
	}

	UWorld* GetTestGameWorld()
	{
		const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
		for (const FWorldContext& Context : WorldContexts)
		{
			if ((Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game) && (Context.World() != nullptr))
			{
				return Context.World();
			}
		}
		return nullptr;
	}
}