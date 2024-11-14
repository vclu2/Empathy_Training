// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Http.h"
#include "custom_json.hpp"
#include "WebGameMode.generated.h"

/**
 * 
 */
UCLASS()
class JUMP_1_API AWebGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	AWebGameMode();

	virtual void StartPlay() override;

	UFUNCTION(BlueprintCallable, Category = "GPT Actions")
	void PostMessage();

	UFUNCTION(BlueprintCallable, Category = "GPT Actions")
	void TranscribeAudio();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GPT Response")
	FString CurrentResponse = "NULL";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GPT Response")
	bool bMessageResponseReceived = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GPT Response")
	FString CurrentAudioTranscription = "NULL";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GPT Response")
	bool bAudioResponseReceived = false;

private:

	const FString& Audio_ApiKey = TEXT("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");
	const FString& Audio_ApiUrl = TEXT("https://api.openai.com/v1/audio/transcriptions");

	const FString& Message_ApiKey = TEXT("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");
	const FString& Message_ApiUrl = TEXT("https://api.openai.com/v1/chat/completions");

	const FString& Speech2Text_ApiUrl = TEXT("https://api.deepgram.com/v1/listen/speak");
	const FString& Speech2Text_ApiKey = TEXT("...");

	// This is ABSOLUTE PATH!
	const FString& Audio_Filepath = TEXT("C:\\Users\\vrkhare2\\Documents\\Unreal Projects\\VICTORS_STUFF\\IFSI_BuildTest\\Saved\\BouncedWavFiles\\Aloha.wav");

	
	TArray<TSharedPtr<FJsonValue>> messageString;

	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccesfully);
	void OnTranscriptionResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	void SpeechToText(FString inputText);
	void OnSpeechToTextResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	void PopulateInitialMessageArray();
	void PopulateMessageArray(FString mode, FString message);

	TArray<uint8> _FStringToUint8(const FString& InString);

};
