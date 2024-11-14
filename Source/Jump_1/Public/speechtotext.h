#include "YourClass.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Json.h"
#include "JsonUtilities.h"

// OpenAI API Key and URL
const FString ApiKey = TEXT("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");
const FString ApiUrl = TEXT("https://api.openai.com/v1/audio/transcriptions");

void UYourClass::TranscribeAudio(const FString& FilePath) {
    // Create the HTTP request
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->OnProcessRequestComplete().BindUObject(this, &UYourClass::OnTranscriptionResponseReceived);

    // Set up the request headers
    HttpRequest->SetURL(ApiUrl);
    HttpRequest->SetVerb("POST");
    HttpRequest->SetHeader("Authorization", "Bearer " + ApiKey);
    HttpRequest->SetHeader("Content-Type", "multipart/form-data; boundary=BoundaryString");

    // Prepare the multipart form-data body
    FString Boundary = "BoundaryString";
    FString ModelPart = "--" + Boundary + "\r\nContent-Disposition: form-data; name=\"model\"\r\n\r\nwhisper-1\r\n";
    FString LanguagePart = "--" + Boundary + "\r\nContent-Disposition: form-data; name=\"language\"\r\n\r\nen\r\n";
    FString FilePartHeader = "--" + Boundary + "\r\nContent-Disposition: form-data; name=\"file\"; filename=\"audio.wav\"\r\nContent-Type: application/octet-stream\r\n\r\n";
    FString BoundaryEnd = "\r\n--" + Boundary + "--\r\n";

    // Load the audio file
    TArray<uint8> AudioData;
    if (!FFileHelper::LoadFileToArray(AudioData, *FilePath)) {
        UE_LOG(LogTemp, Error, TEXT("Failed to load audio file: %s"), *FilePath);
        return;
    }

    // Construct the request body
    TArray<uint8> RequestBody;
    RequestBody.Append((uint8*)TCHAR_TO_UTF8(*ModelPart), ModelPart.Len());
    RequestBody.Append((uint8*)TCHAR_TO_UTF8(*LanguagePart), LanguagePart.Len());
    RequestBody.Append((uint8*)TCHAR_TO_UTF8(*FilePartHeader), FilePartHeader.Len());
    RequestBody.Append(AudioData);
    RequestBody.Append((uint8*)TCHAR_TO_UTF8(*BoundaryEnd), BoundaryEnd.Len());

    // Set the request content
    HttpRequest->SetContent(RequestBody);

    // Send the request
    HttpRequest->ProcessRequest();
}

void UYourClass::OnTranscriptionResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
    if (!bWasSuccessful || !Response.IsValid()) {
        UE_LOG(LogTemp, Error, TEXT("Failed to get transcription response"));
        return;
    }

    FString ResponseBody = Response->GetContentAsString();
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

    if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
        FString Transcription = JsonObject->GetStringField("text");
        UE_LOG(LogTemp, Log, TEXT("Transcription: %s"), *Transcription);
    } else {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON response"));
    }
}
