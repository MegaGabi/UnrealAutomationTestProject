#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/Blueprint.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Misc/OutputDeviceNull.h"

namespace TestProject
{
	template<typename Type1, typename Type2>
	struct TestPayload
	{
		Type1 TestValue;
		Type2 ExpectedValue;
	};

	void CallFuncByNameWithParams(UObject* Object, const FString& FuncName, const TArray<FString>& Params);

	UWorld* GetTestGameWorld();

	class LevelScope
	{
	public:
		LevelScope(const FString& MapName)
		{
			AutomationOpenMap(MapName);
		}
		~LevelScope()
		{
			ADD_LATENT_AUTOMATION_COMMAND(FExitGameCommand);
		}
	};

	class FCustomUntilCommand : public IAutomationLatentCommand
	{
	public:
		FCustomUntilCommand(TFunction<bool()> InCallback, float InTimeout = 5.0f)
			: Callback(MoveTemp(InCallback))
			, Timeout(InTimeout)
		{}

		virtual bool Update() override
		{
			Callback();
			const double NewTime = FPlatformTime::Seconds();
			if (NewTime - StartTime >= Timeout)
			{
				return true;
			}

			return false;
		}

	private:
		TFunction<bool()> Callback;
		float Timeout;
	};
}