// Fill out your copyright notice in the Description page of Project Settings.


#include "WebGameMode.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/Actor.h"
#include "Json.h"
#include <iostream>
#include <vector>
#include "HttpModule.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"



using json = nlohmann::json;

AWebGameMode::AWebGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Blueprints/MyCharacter"));
	
	if (PlayerPawnClassFinder.Class != NULL) {
		DefaultPawnClass = PlayerPawnClassFinder.Class;
	}

	PopulateInitialMessageArray();
}

void AWebGameMode::StartPlay()
{
	Super::StartPlay();
}

void AWebGameMode::PostMessage()
{
	// Bind receiving function
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AWebGameMode::OnResponseReceived);

	// Set Header data
	Request->SetURL(Message_ApiUrl);
	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetHeader("Authorization", TEXT("Bearer ") + Message_ApiKey);
	
	// Set Request Body
	TSharedRef<FJsonObject> RequestObj = MakeShared<FJsonObject>();
	RequestObj->SetStringField("model", "gpt-3.5-turbo");
	RequestObj->SetArrayField("messages", messageString);

	// Serialize RequestObj into String
	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RequestObj, Writer);

	// Process RequestBody and submit post
	Request->SetContentAsString(RequestBody);
	Request->ProcessRequest();
}

void AWebGameMode::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccesfully)
{
	// Parse JSON format Response
	TSharedPtr<FJsonObject> ResponseObj;
	TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(Response->GetContentAsString());
	
	// Deseialize Response Body
	if (FJsonSerializer::Deserialize(Reader, ResponseObj))
	{
		// Check if the "choices" field exists and is not empty
		const TArray<TSharedPtr<FJsonValue>>* ChoicesArray;
		if (ResponseObj->TryGetArrayField("choices", ChoicesArray) && ChoicesArray->Num() > 0)
		{
			// Check if the first element contains "message"
			const TSharedPtr<FJsonObject> MessageObject = (*ChoicesArray)[0]->AsObject();
			if (MessageObject && MessageObject->HasField("message"))
			{
				// Check if the "message" field exists and is not empty
				TSharedPtr<FJsonObject> ContentObject = MessageObject->GetObjectField("message");

				// Get the "content" field
				if (ContentObject && ContentObject->HasField("content"))
				{
					// Set the response to variable CurrentResponse
					FString MessageResponse = ContentObject->GetStringField("content");
					CurrentResponse = MessageResponse;
					bMessageResponseReceived = true;
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, MessageResponse);

					// Add response to message array
					PopulateMessageArray("assistant", MessageResponse);
				}
			}
		}		
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to deserialize JSON response."));
	}
}

void AWebGameMode::TranscribeAudio()
{
	// Create the HTTP request
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &AWebGameMode::OnTranscriptionResponseReceived);

	// Load the audio file as binary data
	TArray<uint8> AudioData;
	if (!FFileHelper::LoadFileToArray(AudioData, *Audio_Filepath)) {
		UE_LOG(LogTemp, Error, TEXT("Failed to load audio file: %s"), *Audio_Filepath);
		return;
	}

	// Create the HTTP request Header with the multipart/form-data format
	HttpRequest->SetURL(Audio_ApiUrl);
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetHeader(TEXT("Authorization"), TEXT("Bearer ") + Audio_ApiKey);
	FString Boundary = TEXT("----WebKitFormBoundary7MA4YWxkTrZu0gW");
	HttpRequest->SetHeader(TEXT("Content-Type"), FString::Printf(TEXT("multipart/form-data; boundary=%s"), *Boundary));

	// Set up post content
	FString MultipartBody;
	TArray<uint8> CombinedContent;

	// Add model field
	MultipartBody += FString::Printf(TEXT("--%s\r\n"), *Boundary);
	MultipartBody += TEXT("Content-Disposition: form-data; name=\"model\"\r\n\r\n");
	MultipartBody += TEXT("whisper-1\r\n");

	// Set Language
	MultipartBody += FString::Printf(TEXT("--%s\r\n"), *Boundary);;
	MultipartBody += TEXT("Content-Disposition: form-data; name=\"language\"\r\n\r\n");
	MultipartBody += TEXT("en\r\n");

	// Add file field
	MultipartBody += FString::Printf(TEXT("--%s\r\n"), *Boundary);
	MultipartBody += TEXT("Content-Disposition: form-data; name=\"file\"; filename=\"audio.wav\"\r\n");
	MultipartBody += TEXT("Content-Type: audio/wav\r\n\r\n");

	CombinedContent.Append(_FStringToUint8(MultipartBody));

	// Add binary audio data to the body
	FString AudioDataString = FString(reinterpret_cast<const TCHAR*>(AudioData.GetData()), AudioData.Num());
	MultipartBody += AudioDataString;

	CombinedContent.Append(AudioData);

	// Add closing boundary
	MultipartBody += FString::Printf(TEXT("\r\n--%s--\r\n"), *Boundary);

	CombinedContent.Append(_FStringToUint8(FString::Printf(TEXT("\r\n--%s--\r\n"), *Boundary)));


	// Set the body content CHOOSE ONE
	// HttpRequest->SetContentAsString(MultipartBody);
	HttpRequest->SetContent(CombinedContent);
	HttpRequest->ProcessRequest();
}

void AWebGameMode::OnTranscriptionResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid()) {
		UE_LOG(LogTemp, Error, TEXT("@@@@@@@@@@@@@@@ Failed to get transcription response @@@@@@@@@@@@@@@@@@@"));
		return;
	}

	FString ResponseBody = Response->GetContentAsString();

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, ResponseBody);
	UE_LOG(LogTemp, Log, TEXT("@@@@@@@@@@@@@@ Response: %s"), *ResponseBody);
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

	if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
		FString Transcription = JsonObject->GetStringField("text");
		UE_LOG(LogTemp, Log, TEXT("!!!!!!!!!!!!!!!!!!!!!! Transcription: %s"), *Transcription);

		// Debugging
		CurrentAudioTranscription = Transcription;
		bAudioResponseReceived = true;

		// Send new Message
		PopulateMessageArray("user", Transcription);
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, Transcription);
		PostMessage();
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("!!!!!!!!!!!!!!!!!!!!!!!! Failed to parse JSON response"));
	}
}

void AWebGameMode::SpeechToText(FString inputText)
{
	// Bind receiving function
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AWebGameMode::OnSpeechToTextResponseReceived);

	// Set Header data
	Request->SetURL(Speech2Text_ApiUrl);
	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetHeader("Authorization", TEXT("Token ") + Speech2Text_ApiKey);

	// Set Request Body
	TSharedRef<FJsonObject> RequestObj = MakeShared<FJsonObject>();
	RequestObj->SetStringField("model", "aura-zeus-en");
	RequestObj->SetStringField("encoding", "linear16");
	RequestObj->SetStringField("container", "wav");
	RequestObj->SetStringField("text", inputText);

	// Serialize RequestObj into String
	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RequestObj, Writer);

	// Process RequestBody and submit post
	Request->SetContentAsString(RequestBody);
	Request->ProcessRequest();
}

void AWebGameMode::OnSpeechToTextResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccesfully)
{
	// Parse JSON format Response
	TSharedPtr<FJsonObject> ResponseObj;
	TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(Response->GetContentAsString());

	// Deseialize Response Body
	if (FJsonSerializer::Deserialize(Reader, ResponseObj))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Success"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to deserialize JSON response."));
	}
}

void AWebGameMode::PopulateInitialMessageArray()
{
	// Add messages to the array
	FString initialPrompt = " * *Task * *: You are simulating a patient who has been severely injured in a car accident.Respond naturally in complete sentences to questions from emergency medical personnel(EMT) assessing and treating your injuries.Your responses should reflect pain, disorientation, and limited ability to focus, with sentences that convey your injuries clearly but without unnecessary repetition.Aim for brief, varied responses that are clear and conversational. \
		** Context** : You are a man in your mid - 20s involved in a serious car accident.After colliding with a tree, you were found lying on the road a short distance from the wreck.You feel weak, disoriented, and in significant pain.The main injury is in your chest, which is bleeding heavily and makes breathing painful.You have minor scratches on your right leg and both arms, which feel numb and tingly, but are not the main source of pain or bleeding.You're showing symptoms of shock, such as feeling cold, shivering, and struggling to stay focused. \
		* *Persona * *: \
		-**Background * *: You are generally healthy, with no history of chronic health issues.You may not fully understand medical terms, so you describe your symptoms in basic, straightforward language. \
		- **Mental State * *: You feel dazed, confused, and somewhat detached from what's happening. Your memory of the accident is hazy, and it's hard to concentrate. \
		- **Physical Symptoms * *: \
		-**Breathing and Chest Pain * *: There is sharp, intense pain on the right side of your chest that makes breathing shallow and difficult.Blood is coming from this chest injury. \
		- **Leg and Arm Injuries * *: You have minor scratches on your right leg and both arms, which feel numb and tingly, likely due to shock or minor injuries. \
		- **Overall Condition * *: You feel cold, weak, and shaky, with dizziness and lightheadedness, likely due to blood loss and shock. \
		** Tone** : Use a weak, subdued tone with signs of confusion and pain.Respond in natural, complete sentences that directly address the EMT's questions. Avoid repetitive phrases; instead, vary your answers slightly to maintain a natural flow in the conversation. \
		* *Detailed Condition * *: \
		-**Chest Injury * *: Blood is seeping from the right side of your chest, causing sharp pain, especially with breathing.This injury is the main source of pain and bleeding. \
		- **Right Leg and Arm Injuries * *: Minor scratches on your right leg and arms are slightly uncomfortable but are not the main focus of pain. \
		- **Shock Symptoms * *: You feel cold and are shivering, regardless of the temperature.Lightheadedness and dizziness make it difficult to stay alert. \
		** Response Style** : Answer questions in natural, complete sentences that feel conversational and not overly repetitive.Adjust responses slightly based on context.For example : \
	-When asked, “What happened ? ” respond like : “I was in a car accident.I hit a tree, and my chest is bleeding pretty badly.”\
		- When asked, “How are you feeling ? ” respond with : “I feel really cold and weak.It's hard to breathe because of the pain in my chest.”\
		- When asked about specific pain, try : “My chest hurts the most, and it's bleeding. My leg and arms have some scratches, but they're not as bad.”\
		Focus on making responses sound natural, clear, and conversational.Reflect the seriousness of your injuries and answer in a way that varies slightly to avoid sounding mechanical.";

	PopulateMessageArray("system", initialPrompt);
}

void AWebGameMode::PopulateMessageArray(FString role, FString message)
{
	// Add messages to the array
	TSharedPtr<FJsonObject> populatedMessage = MakeShared<FJsonObject>();
	populatedMessage->SetStringField("role", role);
	populatedMessage->SetStringField("content", message);
	messageString.Add(MakeShareable(new FJsonValueObject(populatedMessage)));

}

// Helper Functions
TArray<uint8> AWebGameMode::_FStringToUint8(const FString& InString)
{
	TArray<uint8> OutBytes;

	// Handle empty strings
	if (InString.Len() > 0)
	{
		FTCHARToUTF8 Converted(*InString); // Convert to UTF8
		OutBytes.Append(reinterpret_cast<const uint8*>(Converted.Get()), Converted.Length());
	}

	return OutBytes;
}
