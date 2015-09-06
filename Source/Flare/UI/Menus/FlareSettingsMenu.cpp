
#include "../../Flare.h"
#include "FlareSettingsMenu.h"
#include "../../Game/FlareGame.h"
#include "../../Game/FlareSaveGame.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"
#include "STextComboBox.h"


#define LOCTEXT_NAMESPACE "FlareSettingsMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareSettingsMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	Game = MenuManager->GetPC()->GetGame();
	UGameUserSettings* MyGameSettings = GEngine->GetGameUserSettings();

	// Current settings
	float CurrentTextureQualityRatio = MyGameSettings->ScalabilityQuality.TextureQuality / 3.f;
	float CurrentEffectsQualityRatio = MyGameSettings->ScalabilityQuality.EffectsQuality / 3.f;
	float CurrentAntiAliasingQualityRatio = MyGameSettings->ScalabilityQuality.AntiAliasingQuality / 3.f;
	float CurrentPostProcessQualityRatio = MyGameSettings->ScalabilityQuality.PostProcessQuality / 3.f;

	FLOGV("MyGameSettings->ScalabilityQuality.TextureQuality=%d CurrentTextureQualityRatio=%f", MyGameSettings->ScalabilityQuality.TextureQuality, CurrentTextureQualityRatio);
	FLOGV("MyGameSettings->ScalabilityQuality.EffectsQuality=%d CurrentEffectsQualityRatio=%f", MyGameSettings->ScalabilityQuality.EffectsQuality, CurrentEffectsQualityRatio);
	FLOGV("MyGameSettings->ScalabilityQuality.AntiAliasingQuality=%d CurrentAntiAliasingQualityRatio=%f", MyGameSettings->ScalabilityQuality.AntiAliasingQuality, CurrentAntiAliasingQualityRatio);
	FLOGV("MyGameSettings->ScalabilityQuality.PostProcessQuality=%d CurrentPostProcessQualityRatio=%f", MyGameSettings->ScalabilityQuality.PostProcessQuality, CurrentPostProcessQualityRatio);


	CreateBinds();

	// General data
	FLinearColor Color = Theme.NeutralColor;
	Color.A = Theme.DefaultAlpha;
	int32 LabelSize = 200;
	int32 ValueSize = 80;

	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		.Padding(Theme.ContentPadding)
		[
			SNew(SHorizontalBox)

			// Icon
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SImage).Image(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Main))
			]

			// Title
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(Theme.ContentPadding)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.TitleFont)
				.Text(LOCTEXT("Settings", "SETTINGS"))
			]

			// Close
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(Theme.TitleButtonPadding)
			.AutoWidth()
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("Back", "Back"))
				.HelpText(LOCTEXT("BackInfo", "Go back to the main menu"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Exit, true))
				.OnClicked(this, &SFlareSettingsMenu::OnExit)
			]
		]

		// Separator
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(200, 20))
		[
			SNew(SImage).Image(&Theme.SeparatorBrush)
		]

		// Main form
		+ SVerticalBox::Slot()
		.Padding(Theme.ContentPadding)
		.HAlign(HAlign_Center)
		[
			SNew(SScrollBox)
			.Style(&Theme.ScrollBoxStyle)
			.ScrollBarStyle(&Theme.ScrollBarStyle)

			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)

				// Info
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("GraphicsSettingsHint", "Graphics"))
					.TextStyle(&Theme.SubTitleFont)
				]

				// Subtitle
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(LOCTEXT("AutoApply", "Changes will be applied and saved automatically."))
				]

				// Graphic form
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				.HAlign(HAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(Theme.ContentWidth)
					.HAlign(HAlign_Fill)
					[
						SNew(SVerticalBox)

						// Resolution
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SAssignNew(ResolutionSelector, SComboBox<TSharedPtr<FScreenResolutionRHI>>)
							.OptionsSource(&ResolutionList)
							.OnGenerateWidget(this, &SFlareSettingsMenu::OnGenerateResolutionComboLine)
							.OnSelectionChanged(this, &SFlareSettingsMenu::OnResolutionComboLineSelectionChanged)
							.ComboBoxStyle(&Theme.ComboBoxStyle)
							.ForegroundColor(FLinearColor::White)
							[
								SNew(STextBlock)
								.Text(this, &SFlareSettingsMenu::OnGetCurrentResolutionComboLine)
								.TextStyle(&Theme.TextFont)
							]
						]

						// Fullscreen
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						.HAlign(HAlign_Right)
						[
							SAssignNew(FullscreenButton, SFlareButton)
							.Text(LOCTEXT("Fullscreen", "Fullscreen"))
							.HelpText(LOCTEXT("FullscreenInfo", "Show the game in full screen"))
							.Toggle(true)
							.OnClicked(this, &SFlareSettingsMenu::OnFullscreenToggle)
						]

						// VSync
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						.HAlign(HAlign_Right)
						[
							SAssignNew(VSyncButton, SFlareButton)
							.Text(LOCTEXT("V-sync", "V-Sync"))
							.HelpText(LOCTEXT("VSyncInfo", "Vertical synchronization ensures that every image is consistent, even with low performance."))
							.Toggle(true)
							.OnClicked(this, &SFlareSettingsMenu::OnVSyncToggle)
						]

						// Texture quality box
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SNew(SHorizontalBox)

							// Texture quality text
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(Theme.ContentPadding)
							[
								SNew(SBox)
								.WidthOverride(LabelSize)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("TextureLabel", "Texture quality"))
									.TextStyle(&Theme.TextFont)
								]
							]

							// Texture quality slider
							+ SHorizontalBox::Slot()
							.VAlign(VAlign_Center)
							.Padding(Theme.ContentPadding)
							[
								SAssignNew(TextureQualitySlider, SSlider)
								.Value(CurrentTextureQualityRatio)
								.Style(&Theme.SliderStyle)
								.OnValueChanged(this, &SFlareSettingsMenu::OnTextureQualitySliderChanged)
							]

							// Texture quality label
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(Theme.ContentPadding)
							[
								SNew(SBox)
								.WidthOverride(ValueSize)
								[
									SAssignNew(TextureQualityLabel, STextBlock)
									.TextStyle(&Theme.TextFont)
									.Text(GetTextureQualityLabel(MyGameSettings->ScalabilityQuality.TextureQuality))
								]
							]
						]

						// Effets quality box
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SNew(SHorizontalBox)

							// Effects quality text
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(Theme.ContentPadding)
							[
								SNew(SBox)
								.WidthOverride(LabelSize)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("EffectsLabel", "Effects quality"))
									.TextStyle(&Theme.TextFont)
								]
							]

							// Effects quality slider
							+ SHorizontalBox::Slot()
							.VAlign(VAlign_Center)
							.Padding(Theme.ContentPadding)
							[
								SAssignNew(EffectsQualitySlider, SSlider)
								.Value(CurrentEffectsQualityRatio)
								.Style(&Theme.SliderStyle)
								.OnValueChanged(this, &SFlareSettingsMenu::OnEffectsQualitySliderChanged)
							]

							// Effects quality label
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(Theme.ContentPadding)
							[
								SNew(SBox)
								.WidthOverride(ValueSize)
								[
									SAssignNew(EffectsQualityLabel, STextBlock)
									.TextStyle(&Theme.TextFont)
									.Text(GetEffectsQualityLabel(MyGameSettings->ScalabilityQuality.EffectsQuality))
								]
							]
						]

						// AntiAliasing quality box
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SNew(SHorizontalBox)

							// Anti aliasing quality text
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(Theme.ContentPadding)
							[
								SNew(SBox)
								.WidthOverride(LabelSize)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("AntiAliasingLabel", "Anti-aliasing quality"))
									.TextStyle(&Theme.TextFont)
								]
							]

							// AntiAliasing quality slider
							+ SHorizontalBox::Slot()
							.VAlign(VAlign_Center)
							.Padding(Theme.ContentPadding)
							[
								SAssignNew(AntiAliasingQualitySlider, SSlider)
								.Value(CurrentAntiAliasingQualityRatio)
								.Style(&Theme.SliderStyle)
								.OnValueChanged(this, &SFlareSettingsMenu::OnAntiAliasingQualitySliderChanged)
							]

							// AntiAliasing quality label
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(Theme.ContentPadding)
							[
								SNew(SBox)
								.WidthOverride(ValueSize)
								[
									SAssignNew(AntiAliasingQualityLabel, STextBlock)
									.TextStyle(&Theme.TextFont)
									.Text(GetAntiAliasingQualityLabel(MyGameSettings->ScalabilityQuality.AntiAliasingQuality))
								]
							]
						]

						// PostProcess quality box
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SNew(SHorizontalBox)

							// PostProcess quality text
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(Theme.ContentPadding)
							[
								SNew(SBox)
								.WidthOverride(LabelSize)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("PostProcessLabel", "Post-processing quality"))
									.TextStyle(&Theme.TextFont)
								]
							]

							// PostProcess quality slider
							+ SHorizontalBox::Slot()
							.VAlign(VAlign_Center)
							.Padding(Theme.ContentPadding)
							[
								SAssignNew(PostProcessQualitySlider, SSlider)
								.Value(CurrentPostProcessQualityRatio)
								.Style(&Theme.SliderStyle)
								.OnValueChanged(this, &SFlareSettingsMenu::OnPostProcessQualitySliderChanged)
							]

							// PostProcess quality label
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(Theme.ContentPadding)
							[
								SNew(SBox)
								.WidthOverride(ValueSize)
								[
									SAssignNew(PostProcessQualityLabel, STextBlock)
									.TextStyle(&Theme.TextFont)
									.Text(GetPostProcessQualityLabel(MyGameSettings->ScalabilityQuality.PostProcessQuality))
								]
							]
						]

						// Supersampling
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						.HAlign(HAlign_Right)
						[
							SAssignNew(SupersamplingButton, SFlareButton)
							.Text(LOCTEXT("Supersampling", "Supersampling"))
							.HelpText(LOCTEXT("SupersamplingInfo", "Supersampling will render the game at double the resolution. This is a very demanding feature."))
							.Toggle(true)
							.OnClicked(this, &SFlareSettingsMenu::OnSupersamplingToggle)
						]
					]
				]


				// Controls Info
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ControlsSettingsHint", "Controls"))
					.TextStyle(&Theme.SubTitleFont)
				]

				// Controls form
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				.HAlign(HAlign_Center)
				[
					BuildKeyBindingBox()
				]

			]
		]
	];

	FullscreenButton->SetActive(MyGameSettings->GetFullscreenMode() == EWindowMode::Fullscreen);
	VSyncButton->SetActive(MyGameSettings->IsVSyncEnabled());
	SupersamplingButton->SetActive(MyGameSettings->ScalabilityQuality.ResolutionQuality > 100);
}


TSharedRef<SWidget> SFlareSettingsMenu::BuildKeyBindingBox()
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	TSharedPtr<SVerticalBox> KeyboardBox;
	SAssignNew(KeyboardBox, SVerticalBox)
	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(5.0f, 5.0f, 10.0f, 5.0f)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.Padding(10.0f, 0.0f, 10.0f, 0.0f)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.TextStyle(&Theme.SubTitleFont)
			.Text(LOCTEXT("ControlSettingsAction", "Action"))
		]
		+ SHorizontalBox::Slot()
		.Padding(10.0f, 0.0f, 10.0f, 0.0f)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.FillWidth(1.2f)
		[
			SNew(STextBlock)
			.TextStyle(&Theme.SubTitleFont)
			.Text(LOCTEXT("ControlSettingsKeyBinds", "Key"))
		]
		+ SHorizontalBox::Slot()
		.Padding(10.0f, 0.0f, 10.0f, 0.0f)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.FillWidth(1.2f)
		[
			SNew(STextBlock)
			.TextStyle(&Theme.SubTitleFont)
			.Text(LOCTEXT("ControlSettingsAlternateKeyBinds", "Alternate Key"))
		]
	]

	//Key bind list

	+SVerticalBox::Slot()
	.AutoHeight()
	.Padding(FMargin(10.0f, 10.0f, 10.0f, 5.0f))
	[
		SAssignNew(ControlList, SVerticalBox)
	];

	if (KeyboardBox.IsValid())
	{
		//Create the bind list
		for (const auto& Bind : Binds)
		{
			if (Bind->bHeader)
			{
				ControlList->AddSlot()
				.AutoHeight()
				.Padding(FMargin(10.0f, 15.0f, 10.0f, 5.0f))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(10.0f, 0.0f, 10.0f, 0.0f)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(FText::FromString(Bind->DisplayName))
					]
				];
			}
			else
			{
				ControlList->AddSlot()
				.AutoHeight()
				.Padding(FMargin(10.0f, 4.0f, 10.0f, 4.0f))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(10.0f, 0.0f, 10.0f, 0.0f)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(FText::FromString(Bind->DisplayName))
					]
					+ SHorizontalBox::Slot()
					.Padding(10.0f, 4.0f, 10.0f, 4.0f)
					[
						SAssignNew(Bind->KeyWidget, SFlareKeyBind)
						.Key(Bind->Key)
						.DefaultKey(Bind->DefaultKey)
						.OnKeyBindingChanged( this, &SFlareSettingsMenu::OnKeyBindingChanged, Bind, true)
					]
					+ SHorizontalBox::Slot()
					.Padding(10.0f, 4.0f, 10.0f, 4.0f)
					[
						SAssignNew(Bind->AltKeyWidget, SFlareKeyBind)
						.Key(Bind->AltKey)
						.DefaultKey(Bind->DefaultAltKey)
						.OnKeyBindingChanged( this, &SFlareSettingsMenu::OnKeyBindingChanged, Bind, false)
					]
				];
			}
		}
	}

	return KeyboardBox.ToSharedRef();
}



/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareSettingsMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);
}

void SFlareSettingsMenu::Enter()
{
	FLOG("SFlareSettingsMenu::Enter");

	SetEnabled(true);
	SetVisibility(EVisibility::Visible);
	
	FillResolutionList();
}

void SFlareSettingsMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

FText SFlareSettingsMenu::OnGetCurrentResolutionComboLine() const
{
	TSharedPtr<FScreenResolutionRHI> Item = ResolutionSelector->GetSelectedItem();
	return Item.IsValid() ? FText::FromString(FString::Printf(TEXT("%dx%d"), Item->Width, Item->Height)) : FText::FromString("");
}

TSharedRef<SWidget> SFlareSettingsMenu::OnGenerateResolutionComboLine(TSharedPtr<FScreenResolutionRHI> Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	return SNew(STextBlock)
	.Text(FText::FromString(FString::Printf(TEXT("%dx%d"), Item->Width, Item->Height)))
	.TextStyle(&Theme.TextFont);
}

void SFlareSettingsMenu::OnResolutionComboLineSelectionChanged(TSharedPtr<FScreenResolutionRHI> StringItem, ESelectInfo::Type SelectInfo)
{
	UpdateResolution();
}

void SFlareSettingsMenu::OnFullscreenToggle()
{
	UpdateResolution();
}

void SFlareSettingsMenu::OnTextureQualitySliderChanged(float Value)
{
	int32 Step = 3;
	int32 StepValue = FMath::RoundToInt(Step * Value);
	TextureQualitySlider->SetValue((float)StepValue / (float)Step);

	UGameUserSettings* MyGameSettings = GEngine->GetGameUserSettings();
	FLOGV("SFlareSettingsMenu::OnTextureQualitySliderChanged : Set Texture quality to %d (current is %d)", StepValue, MyGameSettings->ScalabilityQuality.TextureQuality);

	if (MyGameSettings->ScalabilityQuality.TextureQuality != StepValue)
	{
		MyGameSettings->ScalabilityQuality.TextureQuality = StepValue;
		MyGameSettings->ApplySettings(false);
		TextureQualityLabel->SetText(GetTextureQualityLabel(StepValue));
	}
}

void SFlareSettingsMenu::OnEffectsQualitySliderChanged(float Value)
{
	int32 Step = 3;
	int32 StepValue = FMath::RoundToInt(Step * Value);
	EffectsQualitySlider->SetValue((float)StepValue / (float)Step);

	UGameUserSettings* MyGameSettings = GEngine->GetGameUserSettings();
	FLOGV("SFlareSettingsMenu::OnEffectsQualitySliderChanged : Set Effects quality to %d (current is %d)", StepValue, MyGameSettings->ScalabilityQuality.EffectsQuality);

	if (MyGameSettings->ScalabilityQuality.EffectsQuality != StepValue)
	{
		MyGameSettings->ScalabilityQuality.EffectsQuality = StepValue;
		MyGameSettings->ApplySettings(false);
		EffectsQualityLabel->SetText(GetEffectsQualityLabel(StepValue));
	}
}

void SFlareSettingsMenu::OnAntiAliasingQualitySliderChanged(float Value)
{
	int32 Step = 3;
	int32 StepValue = FMath::RoundToInt(Step * Value);
	AntiAliasingQualitySlider->SetValue((float)StepValue / (float)Step);

	int32 AAValue = StepValue;

	UGameUserSettings* MyGameSettings = GEngine->GetGameUserSettings();
	FLOGV("SFlareSettingsMenu::OnAntiAliasingQualitySliderChanged : set AntiAliasing quality to %d (current is %d)", AAValue, MyGameSettings->ScalabilityQuality.AntiAliasingQuality);

	if (MyGameSettings->ScalabilityQuality.AntiAliasingQuality != AAValue)
	{
		MyGameSettings->ScalabilityQuality.AntiAliasingQuality = AAValue;
		MyGameSettings->ApplySettings(false);
		AntiAliasingQualityLabel->SetText(GetAntiAliasingQualityLabel(AAValue));
	}
}

void SFlareSettingsMenu::OnPostProcessQualitySliderChanged(float Value)
{
	int32 Step = 3;
	int32 StepValue = FMath::RoundToInt(Step * Value);
	PostProcessQualitySlider->SetValue((float)StepValue / (float)Step);

	UGameUserSettings* MyGameSettings = GEngine->GetGameUserSettings();
	FLOGV("SFlareSettingsMenu::OnAntiAliasingQualitySliderChanged : set PostProcess quality to %d (current is %d)", StepValue, MyGameSettings->ScalabilityQuality.PostProcessQuality);

	if (MyGameSettings->ScalabilityQuality.PostProcessQuality != StepValue)
	{
		MyGameSettings->ScalabilityQuality.PostProcessQuality = StepValue;
		MyGameSettings->ApplySettings(false);
		PostProcessQualityLabel->SetText(GetPostProcessQualityLabel(StepValue));
	}
}

void SFlareSettingsMenu::OnVSyncToggle()
{
	if (VSyncButton->IsActive())
	{
		FLOG("SFlareSettingsMenu::OnVSyncToggle : Enable vsync")
	}
	else
	{
		FLOG("SFlareSettingsMenu::OnVSyncToggle : Disable vsync")
	}

	UGameUserSettings* MyGameSettings = GEngine->GetGameUserSettings();
	MyGameSettings->SetVSyncEnabled(VSyncButton->IsActive());
	MyGameSettings->ApplySettings(false);
	GetAllActionKeyBindings();
}

void SFlareSettingsMenu::OnSupersamplingToggle()
{
	if (SupersamplingButton->IsActive())
	{
		FLOG("SFlareSettingsMenu::OnSupersamplingToggle : Enable supersampling")
	}
	else
	{
		FLOG("SFlareSettingsMenu::OnSupersamplingToggle : Disable supersampling")
	}

	UGameUserSettings* MyGameSettings = GEngine->GetGameUserSettings();
	IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage"));
	CVar->Set((SupersamplingButton->IsActive() ? 200 : 100), ECVF_SetByScalability);
	MyGameSettings->ApplySettings(false);

	//FLOGV("MyGameSettings->ScalabilityQuality.ResolutionQuality %d", MyGameSettings->ScalabilityQuality.ResolutionQuality);

}

void SFlareSettingsMenu::OnKeyBindingChanged( FKey PreviousKey, FKey NewKey, TSharedPtr<FSimpleBind> BindingThatChanged, bool bPrimaryKey )
{
	// Primary or Alt key changed to a valid state.
	// TODO Duplicate

	UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();
	BindingThatChanged->WriteBind();
	InputSettings->SaveConfig();
}

void SFlareSettingsMenu::OnExit()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_Main);
}


/*----------------------------------------------------
	Helpers
----------------------------------------------------*/

void SFlareSettingsMenu::FillResolutionList()
{
	// Get the current res
	UGameUserSettings* MyGameSettings = GEngine->GetGameUserSettings();
	FIntPoint Resolution = GEngine->GameViewport->Viewport->GetSizeXY();
	FLOGV("SFlareSettingsMenu::FillResolutionList : current resolution is %s", *Resolution.ToString());

	// Setup the list data
	ResolutionList.Empty();
	FScreenResolutionArray Resolutions;
	int CurrentResolutionIndex = -1;

	// Get all resolution settings
	if (RHIGetAvailableResolutions(Resolutions, true))
	{
		for (const FScreenResolutionRHI& EachResolution : Resolutions)
		{
			if (EachResolution.Width >= 1280 && EachResolution.Height>= 720)
			{
				ResolutionList.Insert(MakeShareable(new FScreenResolutionRHI(EachResolution)), 0);
				if (Resolution.X == EachResolution.Width && Resolution.Y == EachResolution.Height)
				{
					CurrentResolutionIndex = ResolutionList.Num() - 1;
				}
			}
		}
	}
	else
	{
		FLOG("SFlareSettingsMenu::FillResolutionList : screen resolutions could not be obtained");
	}

	// Didn't find our current res...
	if (CurrentResolutionIndex < 0)
	{
		TSharedPtr<FScreenResolutionRHI> InitialResolution = MakeShareable(new FScreenResolutionRHI());
		InitialResolution->Width = Resolution.X;
		InitialResolution->Height = Resolution.Y;

		ResolutionList.Insert(InitialResolution, 0);
		CurrentResolutionIndex = 0;
	}

	// Update the list
	ResolutionSelector->SetSelectedItem(ResolutionList[CurrentResolutionIndex]);
	ResolutionSelector->RefreshOptions();
}

void SFlareSettingsMenu::UpdateResolution()
{
	if (GEngine)
	{
		UGameUserSettings* MyGameSettings = GEngine->GetGameUserSettings();
		FIntPoint Resolution = GEngine->GameViewport->Viewport->GetSizeXY();
		FLOGV("SFlareSettingsMenu::UpdateResolution : current resolution is %s", *Resolution.ToString());

		TSharedPtr<FScreenResolutionRHI> Item = ResolutionSelector->GetSelectedItem();
		FLOGV("SFlareSettingsMenu::UpdateResolution : new resolution is %dx%d", Item->Width, Item->Height);

		MyGameSettings->SetScreenResolution(FIntPoint(Item->Width, Item->Height));
		MyGameSettings->SetFullscreenMode(FullscreenButton->IsActive() ? EWindowMode::Fullscreen : EWindowMode::Windowed);
		MyGameSettings->RequestResolutionChange(Item->Width, Item->Height, FullscreenButton->IsActive() ? EWindowMode::Fullscreen : EWindowMode::Windowed, false);

		MyGameSettings->ConfirmVideoMode();
		MyGameSettings->ApplySettings(false);
		/*MyGameSettings->ApplyNonResolutionSettings();
		MyGameSettings->SaveSettings();*/
	}
}

FText SFlareSettingsMenu::GetTextureQualityLabel(int32 Value) const
{
	switch(Value)
	{
		case 1:
			return LOCTEXT("TextureQualityLow", "Low");
		case 2:
			return LOCTEXT("TextureQualityMedium", "Medium");
		case 3:
			return LOCTEXT("TextureQualityHigh", "High");
		case 0:
		default:
			return LOCTEXT("TextureQualityVeryLow", "Very Low");
	}
}

FText SFlareSettingsMenu::GetEffectsQualityLabel(int32 Value) const
{
	switch(Value)
	{
		case 1:
			return LOCTEXT("EffectsQualityLow", "Low");
		case 2:
			return LOCTEXT("EffectsQualityMedium", "Medium");
		case 3:
			return LOCTEXT("EffectsQualityHigh", "High");
		case 0:
		default:
			return LOCTEXT("EffectsQualityVeryLow", "Very Low");
	}
}

FText SFlareSettingsMenu::GetAntiAliasingQualityLabel(int32 Value) const
{
	switch(Value)
	{
		case 1:
			return LOCTEXT("AntiAliasingQualityLow", "Low");
		case 2:
			return LOCTEXT("AntiAliasingQualityMedium", "Medium");
		case 3:
			return LOCTEXT("AntiAliasingQualityHigh", "High");
		case 0:
		default:
			return LOCTEXT("AntiAliasingQualityDisabled", "Disabled");		
	}
}

FText SFlareSettingsMenu::GetPostProcessQualityLabel(int32 Value) const
{
	switch(Value)
	{
		case 1:
			return LOCTEXT("PostProcessQualityLow", "Low");
		case 2:
			return LOCTEXT("PostProcessQualityMedium", "Medium");
		case 3:
			return LOCTEXT("PostProcessQualityHigh", "High");
		case 0:
		default:
			return LOCTEXT("PostProcessQualityVeryLow", "Very Low");
	}
}


void SFlareSettingsMenu::CreateBinds()
{
	//Piloting
	Binds.Add(MakeShareable((new FSimpleBind(NSLOCTEXT("KeyBinds", "Piloting", "Piloting")))->MakeHeader()));
	Binds.Add(MakeShareable((new FSimpleBind(NSLOCTEXT("KeyBinds", "Move Forward", "Move Forward")))
		->AddAxisMapping("Thrust", 1.0f)
		->AddDefaults(EKeys::LeftShift)));
	Binds.Add(MakeShareable((new FSimpleBind(NSLOCTEXT("KeyBinds", "Move Backward", "Move Backward")))
		->AddAxisMapping("Thrust", -1.0f)
		->AddDefaults(EKeys::LeftControl)));
	Binds.Add(MakeShareable((new FSimpleBind(NSLOCTEXT("KeyBinds", "Move Left", "Move Left")))
		->AddAxisMapping("MoveHorizontalInput", -1.0f)
		->AddDefaults(EKeys::A)));
	Binds.Add(MakeShareable((new FSimpleBind(NSLOCTEXT("KeyBinds", "Move Right", "Move Right")))
		->AddAxisMapping("MoveHorizontalInput", 1.0f)
		->AddDefaults(EKeys::D)));
	Binds.Add(MakeShareable((new FSimpleBind(NSLOCTEXT("KeyBinds", "Move Up", "Move Up")))
		->AddAxisMapping("MoveVerticalInput", 1.0f)
		->AddDefaults(EKeys::W)));
	Binds.Add(MakeShareable((new FSimpleBind(NSLOCTEXT("KeyBinds", "Move Down", "Move Down")))
		->AddAxisMapping("MoveVerticalInput", -1.0f)
		->AddDefaults(EKeys::S)));
	Binds.Add(MakeShareable((new FSimpleBind(NSLOCTEXT("KeyBinds", "Roll CW", "Roll CW")))
		->AddAxisMapping("RollInput", 1.0f)
		->AddDefaults(EKeys::E)));
	Binds.Add(MakeShareable((new FSimpleBind(NSLOCTEXT("KeyBinds", "Roll CCW", "Roll CCW")))
		->AddAxisMapping("RollInput", -1.0f)
		->AddDefaults(EKeys::Q)));
	Binds.Add(MakeShareable((new FSimpleBind(NSLOCTEXT("KeyBinds", "Boost", "Boost")))
		->AddActionMapping("Boost")
		->AddDefaults(EKeys::SpaceBar)));

	// Auto pilot
	Binds.Add(MakeShareable((new FSimpleBind(NSLOCTEXT("KeyBinds", "Autopilot", "Autopilot")))->MakeHeader()));
	Binds.Add(MakeShareable((new FSimpleBind(NSLOCTEXT("KeyBinds", "Align Prograde", "Align to Prograde")))
		->AddActionMapping("FaceForward")
		->AddDefaults(EKeys::X)));
	Binds.Add(MakeShareable((new FSimpleBind(NSLOCTEXT("KeyBinds", "Face Retrograde", "Align Retrograde")))
		->AddActionMapping("FaceBackward")
		->AddDefaults(EKeys::Z)));
	Binds.Add(MakeShareable((new FSimpleBind(NSLOCTEXT("KeyBinds", "Disengage Autopilot", "Disengage Autopilot")))
		->AddActionMapping("Manual")
		->AddDefaults(EKeys::M)));

	// Weapons
	Binds.Add(MakeShareable((new FSimpleBind(NSLOCTEXT("KeyBinds", "Weapons", "Weapons")))->MakeHeader()));
	Binds.Add(MakeShareable((new FSimpleBind(NSLOCTEXT("KeyBinds", "Deactivate Weapons", "Deactivate Weapons")))
		->AddActionMapping("DeactivateWeapon")
		->AddDefaults(EKeys::One)));
	Binds.Add(MakeShareable((new FSimpleBind(NSLOCTEXT("KeyBinds", "Activate primary weapons", "Activate primary weapons")))
		->AddActionMapping("WeaponGroup1")
		->AddDefaults(EKeys::Two)));
	Binds.Add(MakeShareable((new FSimpleBind(NSLOCTEXT("KeyBinds", "Activate secondary weapons", "Activate secondary weapons")))
		->AddActionMapping("WeaponGroup2")
		->AddDefaults(EKeys::Three)));
	Binds.Add(MakeShareable((new FSimpleBind(NSLOCTEXT("KeyBinds", "Activate tertiary weapons", "Activate tertiary weapons")))
		->AddActionMapping("WeaponGroup3")
		->AddDefaults(EKeys::Four)));
	Binds.Add(MakeShareable((new FSimpleBind(NSLOCTEXT("KeyBinds", "Toogle Weapons", "Toogle Weapons")))
		->AddActionMapping("ToggleCombat")
		->AddDefaults(EKeys::F)));

	//Misc
	Binds.Add(MakeShareable((new FSimpleBind(NSLOCTEXT("KeyBinds", "Misc", "Misc")))->MakeHeader()));
	Binds.Add(MakeShareable((new FSimpleBind(NSLOCTEXT("KeyBinds", "Quick Ship Switch", "Quick Ship Switch")))
		->AddActionMapping("QuickSwitch")
		->AddDefaults(EKeys::N)));
	Binds.Add(MakeShareable((new FSimpleBind(NSLOCTEXT("KeyBinds", "Toogle Camera", "Toogle Camera")))
		->AddActionMapping("ToggleCamera")
		->AddDefaults(EKeys::C)));
	Binds.Add(MakeShareable((new FSimpleBind(NSLOCTEXT("KeyBinds", "Toogle HUD", "Toogle HUD")))
		->AddActionMapping("ToggleHUD")
		->AddDefaults(EKeys::H)));
}

void SFlareSettingsMenu::GetAllActionKeyBindings()
{
	//Bindings.Empty();

	const UInputSettings* Settings = GetDefault<UInputSettings>();
	if(!Settings) return;

	const TArray<FInputActionKeyMapping>& Actions = Settings->ActionMappings;

	for(const FInputActionKeyMapping& Each : Actions)
	{
		FLOGV("ActionName=%s Key=%s Alt=%d Cmd=%d Ctrl=%d Shift=%d", *Each.ActionName.ToString(), *Each.Key.ToString(), Each.bAlt, Each.bCmd, Each.bCtrl, Each.bShift)
		//Bindings.Add(FVictoryInput(Each));
	}
}

/*----------------------------------------------------
	FSimpleBind
----------------------------------------------------*/


FSimpleBind::FSimpleBind(const FText& InDisplayName)
{
	DisplayName = InDisplayName.ToString();
	bHeader = false;
	Key = MakeShareable(new FKey());
	AltKey = MakeShareable(new FKey());
}
FSimpleBind* FSimpleBind::AddActionMapping(const FName& Mapping)
{
	UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();

	bool bFound = false;
	for (int32 i = 0; i < InputSettings->ActionMappings.Num(); i++)
	{
		FInputActionKeyMapping Action = InputSettings->ActionMappings[i];
		if (Mapping == Action.ActionName && !Action.Key.IsGamepadKey())
		{
			ActionMappings.Add(Action);

			//Fill in the first 2 keys we find from the ini
			if (*Key == FKey())
			{
				*Key = Action.Key;
			}
			else if (*AltKey == FKey() && Action.Key != *Key)
			{
				*AltKey = Action.Key;
			}
			bFound = true;
		}
	}

	if (!bFound)
	{
		FInputActionKeyMapping Action;
		Action.ActionName = Mapping;
		ActionMappings.Add(Action);
	}
	return this;
}

FSimpleBind* FSimpleBind::AddAxisMapping(const FName& Mapping, float Scale)
{
	UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();

	bool bFound = false;
	for (int32 i = 0; i < InputSettings->AxisMappings.Num(); i++)
	{
		FInputAxisKeyMapping Axis = InputSettings->AxisMappings[i];
		if (Mapping == Axis.AxisName && Axis.Scale == Scale && !Axis.Key.IsGamepadKey())
		{
			AxisMappings.Add(Axis);

			//Fill in the first 2 keys we find from the ini
			if (*Key == FKey())
			{
				*Key = Axis.Key;
			}
			else if (*AltKey == FKey() && Axis.Key != *Key)
			{
				*AltKey = Axis.Key;
			}
			bFound = true;
		}
	}

	if (!bFound)
	{
		FInputAxisKeyMapping Action;
		Action.AxisName = Mapping;
		Action.Scale = Scale;
		AxisMappings.Add(Action);
	}
	return this;
}


FSimpleBind* FSimpleBind::AddSpecialBinding(const FName& Mapping)
{
	if (Mapping == FName(TEXT("Console")))
	{
		UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();
		if (InputSettings->ConsoleKeys.IsValidIndex(0))
		{
			*Key = InputSettings->ConsoleKeys[0];
		}
		if (InputSettings->ConsoleKeys.IsValidIndex(1))
		{
			*AltKey = InputSettings->ConsoleKeys[1];
		}
	}

	SpecialBindings.Add(Mapping);

	return this;
}

void FSimpleBind::WriteBind()
{
	if (bHeader)
	{
		return;
	}
	UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();

	// Collapse the keys if the main key is missing.
	if (*Key == FKey() && *AltKey != FKey())
	{
		Key = AltKey;
		AltKey = MakeShareable(new FKey());
	}

	//Remove the original bindings
	for (auto& Bind : ActionMappings)
	{
		InputSettings->RemoveActionMapping(Bind);
	}
	for (auto& Bind : AxisMappings)
	{
		InputSettings->RemoveAxisMapping(Bind);
	}

	//Set our new keys and readd them
	for (auto Bind : ActionMappings)
	{
		Bind.Key = *Key;
		InputSettings->AddActionMapping(Bind);
		if (*AltKey != FKey())
		{
			Bind.Key = *AltKey;
			InputSettings->AddActionMapping(Bind);
		}
	}
	for (auto Bind : AxisMappings)
	{
		Bind.Key = *Key;
		InputSettings->AddAxisMapping(Bind);
		if (*AltKey != FKey())
		{
			Bind.Key = *AltKey;
			InputSettings->AddAxisMapping(Bind);
		}
	}

	for (FName Bind : SpecialBindings)
	{
		if (Bind == FName(TEXT("Console")))
		{
			InputSettings->ConsoleKeys.Empty();
			InputSettings->ConsoleKeys.Add(*Key);
			InputSettings->ConsoleKeys.Add(*AltKey);
		}
	}
}

#undef LOCTEXT_NAMESPACE
