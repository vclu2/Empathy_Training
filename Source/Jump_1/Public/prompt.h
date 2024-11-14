#include "YourClassName.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "JsonUtilities.h"

// Your OpenAI API key and endpoint
const FString ApiKey = TEXT("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");
const FString ApiUrl = TEXT("https://api.openai.com/v1/chat/completions");

void UYourClassName::BeginPlay()
{
    Super::BeginPlay();

    // Initial system message
    InitialMessage = R"(
        **Task**: You are simulating a patient who has been severely injured in a car accident. Respond naturally in complete sentences to questions from emergency medical personnel (EMT) assessing and treating your injuries. Your responses should reflect pain, disorientation, and limited ability to focus, with sentences that convey your injuries clearly but without unnecessary repetition. Aim for brief, varied responses that are clear and conversational.

        **Context**: You are a man in your mid-20s involved in a serious car accident. After colliding with a tree, you were found lying on the road a short distance from the wreck. You feel weak, disoriented, and in significant pain. The main injury is in your chest, which is bleeding heavily and makes breathing painful. You have minor scratches on your right leg and both arms, which feel numb and tingly, but are not the main source of pain or bleeding. You're showing symptoms of shock, such as feeling cold, shivering, and struggling to stay focused.

        **Persona**:
        - **Background**: You are generally healthy, with no history of chronic health issues. You may not fully understand medical terms, so you describe your symptoms in basic, straightforward language.
        - **Mental State**: You feel dazed, confused, and somewhat detached from what's happening. Your memory of the accident is hazy, and it's hard to concentrate.
        - **Physical Symptoms**:
            - **Breathing and Chest Pain**: There is sharp, intense pain on the right side of your chest that makes breathing shallow and difficult. Blood is coming from this chest injury.
            - **Leg and Arm Injuries**: You have minor scratches on your right leg and both arms, which feel numb and tingly, likely due to shock or minor injuries.
            - **Overall Condition**: You feel cold, weak, and shaky, with dizziness and lightheadedness, likely due to blood loss and shock.

        **Tone**: Use a weak, subdued tone with signs of confusion and pain. Respond in natural, complete sentences that directly address the EMT's questions. Avoid repetitive phrases; instead, vary your answers slightly to maintain a natural flow in the conversation.

        **Detailed Condition**:
        - **Chest Injury**: Blood is seeping from the right side of your chest, causing sharp pain, especially with breathing. This injury is the main source of pain and bleeding.
        - **Right Leg and Arm Injuries**: Minor scratches on your right leg and arms are slightly uncomfortable but are not the main focus of pain.
        - **Shock Symptoms**: You feel cold and are shivering, regardless of the temperature. Lightheadedness and dizziness make it difficult to stay alert.

        **Response Style**: Answer questions in natural, complete sentences that feel conversational and not overly repetitive. Adjust responses slightly based on context. For example:
        - When asked, “What happened?” respond like: “I was in a car accident. I hit a tree, and my chest is bleeding pretty badly.”
        - When asked, “How are you feeling?” respond with: “I feel really cold and weak. It's hard to breathe because of the pain in my chest.”
        - When asked about specific pain, try: “My chest hurts the most, and it's bleeding. My leg and arms have some scratches, but they're not as bad.”

        Focus on making responses sound natural, clear, and conversational. Reflect the seriousness of your injuries and answer in a way that varies slightly to avoid sounding mechanical.
    )";

    Messages.Add({ TEXT("role"), TEXT("system") });
    Messages.Add({ TEXT("content"), InitialMessage });

    UE_LOG(LogTemp, Log, TEXT("Car Crash Simulation started. Type 'exit' to end the conversation."));
}

void UYourClassName::SendMessageToAPI(const FString& UserInput)
{
    // Add the user input to the messages array
    TSharedPtr<FJsonObject> UserMessage = MakeShareable(new FJsonObject);
    UserMessage->SetStringField("role", "user");
    UserMessage->SetStringField("content", UserInput);
    MessagesArray.Add(UserMessage);

    // Prepare the OpenAI API request
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->OnProcessRequestComplete().BindUObject(this, &UYourClassName::OnResponseReceived);
    HttpRequest->SetURL(ApiUrl);
    HttpRequest->SetVerb("POST");
    HttpRequest->SetHeader("Authorization", "Bearer " + ApiKey);
    HttpRequest->SetHeader("Content-Type", "application/json");

    // Create JSON payload
    TSharedPtr<FJsonObject> RequestPayload = MakeShareable(new FJsonObject);
    RequestPayload->SetStringField("model", "gpt-3.5-turbo");
    RequestPayload->SetArrayField("messages", MessagesArray);

    FString ContentString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ContentString);
    FJsonSerializer::Serialize(RequestPayload.ToSharedRef(), Writer);
    HttpRequest->SetContentAsString(ContentString);

    // Send the request
    HttpRequest->ProcessRequest();
}

void UYourClassName::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (!bWasSuccessful || !Response.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get a response from the API."));
        return;
    }

    // Parse the JSON response
    FString ResponseContent = Response->GetContentAsString();
    TSharedPtr<FJsonObject> JsonResponse;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseContent);

    if (FJsonSerializer::Deserialize(Reader, JsonResponse) && JsonResponse.IsValid())
    {
        FString AssistantMessage;
        const TArray<TSharedPtr<FJsonValue>>* Choices;
        if (JsonResponse->TryGetArrayField("choices", Choices))
        {
            TSharedPtr<FJsonObject> Choice = (*Choices)[0]->AsObject();
            TSharedPtr<FJsonObject> MessageObj = Choice->GetObjectField("message");
            AssistantMessage = MessageObj->GetStringField("content");
            UE_LOG(LogTemp, Log, TEXT("Patient: %s"), *AssistantMessage);

            // Add assistant response to the conversation history
            TSharedPtr<FJsonObject> AssistantMessageJson = MakeShareable(new FJsonObject);
            AssistantMessageJson->SetStringField("role", "assistant");
            AssistantMessageJson->SetStringField("content", AssistantMessage);
            MessagesArray.Add(AssistantMessageJson);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON response"));
    }
}

void UYourClassName::HandleUserInput(const FString& UserInput)
{
    if (UserInput.ToLower() == "exit")
    {
        UE_LOG(LogTemp, Log, TEXT("Ending conversation."));
        return;
    }

    // Send user input to the API
    SendMessageToAPI(UserInput);
}
