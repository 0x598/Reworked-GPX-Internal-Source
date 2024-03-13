#pragma once
#include "../DrawMenu.h"

void(*DrawTitleSafeAreaOriginal)(UGameViewportClient* Viewport, UCanvas* Canvas) = nullptr;
void DrawTitleSafeArea(UGameViewportClient* Viewport, UCanvas* Canvas)
{
	if (!Viewport || !Canvas)
		return DrawTitleSafeAreaOriginal(Viewport, Canvas);
	Variables::Canvas = Canvas;
	Variables::ScreenSize = FVector2D(double(Variables::Canvas->ClipX()), double(Variables::Canvas->ClipY()));
	Variables::ScreenCenter = FVector2D(Variables::ScreenSize.X / 2.0, Variables::ScreenSize.Y / 2.0);

	char UpdateMyTeam = char(1337);
	bool bInVehicle = false;

	do {
		UWorld* GWorld = UWorld::GetWorld();
		if (!GWorld) return DrawTitleSafeAreaOriginal(Viewport, Canvas);

		UGameInstance* Gameinstance = GWorld->OwningGameInstance();
		if (!Gameinstance) return DrawTitleSafeAreaOriginal(Viewport, Canvas);

		ULocalPlayer* LocalPlayer = Gameinstance->LocalPlayers()[0];
		if (!LocalPlayer) return DrawTitleSafeAreaOriginal(Viewport, Canvas);

		APlayerController* PlayerController = LocalPlayer->PlayerController();
		if (!PlayerController) return DrawTitleSafeAreaOriginal(Viewport, Canvas);
		Variables::PlayerController = PlayerController;

		APlayerCameraManager* PlayerCameraManager = PlayerController->PlayerCameraManager();
		if (!PlayerCameraManager) return DrawTitleSafeAreaOriginal(Viewport, Canvas);

		APlayerState* PlayerState = PlayerController->PlayerState();
		if (!PlayerState) return DrawTitleSafeAreaOriginal(Viewport, Canvas);

		APlayerPawn_Athena_C* AcknowledgedPawn = PlayerController->AcknowledgedPawn();
		Variables::AcknowledgedPawn = AcknowledgedPawn;

		Variables::FieldOfView = PlayerCameraManager->GetFOVAngle();
		Variables::CameraLocation = PlayerCameraManager->GetCameraLocation();
		Variables::CameraRotation = PlayerCameraManager->GetCameraRotation();

		static UObject* PlayerClass = nullptr;
		if (!PlayerClass) PlayerClass = UObject::FindObject(L"FortniteGame.FortPlayerPawnAthena");

		static UObject* PickupClass = nullptr;
		if (!PickupClass) PickupClass = UObject::FindObject(L"FortniteGame.FortPickup");

		static UObject* ContainerClass = nullptr;
		if (!ContainerClass) ContainerClass = UObject::FindObject(L"FortniteGame.BuildingContainer");

		static UObject* ProjectileClass = nullptr;
		if (!ProjectileClass) ProjectileClass = UObject::FindObject(L"FortniteGame.FortProjectileBase");

		static UObject* WeakspotClass = nullptr;
		if (!WeakspotClass) WeakspotClass = UObject::FindObject(L"FortniteGame.BuildingWeakSpot");

		static UObject* BuildingTrapClass = nullptr;
		if (!BuildingTrapClass) BuildingTrapClass = UObject::FindObject(L"FortniteGame.BuildingTrap");

		static UObject* VehicleClass = nullptr;
		if (!VehicleClass) VehicleClass = UObject::FindObject(L"FortniteGame.FortAthenaVehicle");

		static UObject* MarkerClass = nullptr;
		if (!MarkerClass) MarkerClass = UObject::FindObject(L"FortniteGame.FortPlayerMarkerBase");

		static UObject* SupplyDropClass = nullptr;
		if (!SupplyDropClass) SupplyDropClass = UObject::FindObject(L"FortniteGame.FortAthenaSupplyDrop");

		static UObject* RangedWeaponClass = nullptr;
		if (!RangedWeaponClass) RangedWeaponClass = UObject::FindObject(L"FortniteGame.FortWeaponRanged");

		static UObject* FortWeaponClass = nullptr;
		if (!FortWeaponClass) FortWeaponClass = UObject::FindObject(L"FortniteGame.FortWeapon");

		static UObject* PickaxeClass = nullptr;
		if (!PickaxeClass) PickaxeClass = UObject::FindObject(L"FortniteGame.FortWeaponPickaxeAthena");

		double SmallestDegrees = DBL_MAX;
		FVector SmallestDegreesOwnerWorldLocation = FVector();

		double ClosestDistanceToMyself = FLT_MAX;
		double ClosestDistanceToCenter = FLT_MAX;
		APlayerPawn_Athena_C* TargetPlayer = nullptr;

		double AimbotFOV = (Settings::Aimbot::FOV * Variables::ScreenSize.X / Variables::FieldOfView) / 2.0;

		if (AcknowledgedPawn)
		{
			UpdateMyTeam = AcknowledgedPawn->GetTeam();
			bInVehicle = AcknowledgedPawn->IsInVehicle();
		}

		TArray<UObject*> PlayerList = UGameplayStatics::GetAllActorsOfClass(GWorld, PlayerClass);
		for (int32_t i = 0; i < PlayerList.Count; i++)
		{
			APlayerPawn_Athena_C* Player = reinterpret_cast<APlayerPawn_Athena_C*>(PlayerList[i]);
			if (!Player)
				continue;

			bool PlayerCheck = UGameplayStatics::ObjectIsA(Player, PlayerClass);
			if (PlayerCheck)
			{
				if (Player == AcknowledgedPawn) continue;

				bool Wound = Player->IsDBNO();

				bool Dead = Player->IsDead();
				if (Dead) continue;

				char Team = Player->GetTeam();
				bool Teammate = UpdateMyTeam != char(1337) && UpdateMyTeam == Team;

				bool Visable = UFortKismetLibrary::CheckLineOfSightToActorWithChannel(Variables::CameraLocation, Player, ECollisionChannel::ECC_Visibility, nullptr);

				USkeletalMeshComponent* Mesh = Player->Mesh();
				if (!Mesh) continue;

				FLinearColor BoxColor = Settings::Colors::BoxInVisible;
				FLinearColor SkeletonColor = Settings::Colors::SkeletonInVisible;
				FLinearColor SnaplineColor = Settings::Colors::SkeletonInVisible;

				if (Visable)
				{
					BoxColor = Settings::Colors::BoxVisible;
					SkeletonColor = Settings::Colors::SkeletonVisible;
					SnaplineColor = Settings::Colors::SnaplineVisible;
				}

				if (Teammate)
				{
					BoxColor = Settings::Colors::TeammateColor;
					SkeletonColor = Settings::Colors::TeammateColor;
					SnaplineColor = Settings::Colors::TeammateColor;
				}

				FVector Root = Mesh->GetBoneLocation(Bones::Root);
				double Distance = UKismetMathLibrary::Vector_Distance(Variables::CameraLocation, Root) * 0.01;

				FVector Head = Mesh->GetBoneLocation(Bones::head);
				FVector2D HeadScreen = FVector2D();
				if (!PlayerController->ProjectWorldLocationToScreen(Head, &HeadScreen)) continue;

				if (HeadScreen.X < 5.0 || HeadScreen.X > Variables::ScreenSize.X - (5.0 * 2)) continue;
				if (HeadScreen.Y < 5.0 || HeadScreen.Y > Variables::ScreenSize.Y - (5.0 * 2)) continue;

				bool AimbotVisable = (Settings::Aimbot::VisableOnly && Visable) || !Settings::Aimbot::VisableOnly;
				bool AimbotKnocked = (Settings::Aimbot::SkipKnocked && !Wound) || !Settings::Aimbot::SkipKnocked;
				bool AimbotValid = (AimbotVisable && AimbotKnocked && !Teammate) && Distance < Settings::Aimbot::MaxDistance;
				if (AimbotValid) {
					double DistanceToCenter = UKismetMathLibrary::Vector_Distance2D(Variables::ScreenCenter, HeadScreen);
					if (DistanceToCenter < ClosestDistanceToCenter) {
						if (Distance < ClosestDistanceToMyself) {
							ClosestDistanceToMyself = Distance;
							ClosestDistanceToCenter = DistanceToCenter;
							TargetPlayer = Player;
						}
					}
				}

				if (Distance > Settings::Player::MaxDistance) continue;

				FVector2D RootScreen;
				if (!PlayerController->ProjectWorldLocationToScreen(Root, &RootScreen)) continue;

				FVector Head2 = FVector(Head.X, Head.Y, Head.Z + 16.0);
				FVector2D HeadScreen2;
				if (!PlayerController->ProjectWorldLocationToScreen(Head2, &HeadScreen2)) continue;

				FVector Neck = Mesh->GetBoneLocation(Bones::neck_01);
				FVector2D NeckScreen = FVector2D();
				if (!PlayerController->ProjectWorldLocationToScreen(Neck, &NeckScreen)) continue;

				FVector Chest = Mesh->GetBoneLocation(Bones::spine_05);
				FVector2D ChestScreen = FVector2D();
				if (!PlayerController->ProjectWorldLocationToScreen(Chest, &ChestScreen)) continue;

				FVector Chest2 = Mesh->GetBoneLocation(7);
				FVector2D ChestScreen2 = FVector2D();
				if (!PlayerController->ProjectWorldLocationToScreen(Chest2, &ChestScreen2)) continue;

				FVector Pelvis = Mesh->GetBoneLocation(Bones::pelvis);
				FVector2D PelvisScreen = FVector2D();
				if (!PlayerController->ProjectWorldLocationToScreen(Pelvis, &PelvisScreen)) continue;

				FVector RShoulder = Mesh->GetBoneLocation(Bones::upperarm_r);
				FVector2D RShoulderScreen = FVector2D();
				if (!PlayerController->ProjectWorldLocationToScreen(RShoulder, &RShoulderScreen)) continue;

				FVector RElbow = Mesh->GetBoneLocation(Bones::lowerarm_r);
				FVector2D RElbowScreen = FVector2D();
				if (!PlayerController->ProjectWorldLocationToScreen(RElbow, &RElbowScreen)) continue;

				FVector RHand = Mesh->GetBoneLocation(Bones::hand_r);
				FVector2D RHandScreen = FVector2D();
				if (!PlayerController->ProjectWorldLocationToScreen(RHand, &RHandScreen)) continue;

				FVector RThigh = Mesh->GetBoneLocation(Bones::thigh_r);
				FVector2D RThighScreen = FVector2D();
				if (!PlayerController->ProjectWorldLocationToScreen(RThigh, &RThighScreen)) continue;

				FVector RCalf = Mesh->GetBoneLocation(Bones::calf_r);
				FVector2D RCalfScreen = FVector2D();
				if (!PlayerController->ProjectWorldLocationToScreen(RCalf, &RCalfScreen)) continue;

				FVector RKnee = Mesh->GetBoneLocation(Bones::calf_twist_01_r);
				FVector2D RKneeScreen = FVector2D();
				if (!PlayerController->ProjectWorldLocationToScreen(RKnee, &RKneeScreen)) continue;

				FVector RFoot = Mesh->GetBoneLocation(Bones::foot_r);
				FVector2D RFootScreen = FVector2D();
				if (!PlayerController->ProjectWorldLocationToScreen(RFoot, &RFootScreen)) continue;

				FVector LShoulder = Mesh->GetBoneLocation(Bones::upperarm_l);
				FVector2D LShoulderScreen = FVector2D();
				if (!PlayerController->ProjectWorldLocationToScreen(LShoulder, &LShoulderScreen)) continue;

				FVector LElbow = Mesh->GetBoneLocation(Bones::lowerarm_l);
				FVector2D LElbowScreen = FVector2D();
				if (!PlayerController->ProjectWorldLocationToScreen(LElbow, &LElbowScreen)) continue;

				FVector LHand = Mesh->GetBoneLocation(Bones::hand_l);
				FVector2D LHandScreen = FVector2D();
				if (!PlayerController->ProjectWorldLocationToScreen(LHand, &LHandScreen)) continue;

				FVector LThigh = Mesh->GetBoneLocation(Bones::thigh_l);
				FVector2D LThighScreen = FVector2D();
				if (!PlayerController->ProjectWorldLocationToScreen(LThigh, &LThighScreen)) continue;

				FVector LCalf = Mesh->GetBoneLocation(Bones::calf_l);
				FVector2D LCalfScreen = FVector2D();
				if (!PlayerController->ProjectWorldLocationToScreen(LCalf, &LCalfScreen)) continue;

				FVector LKnee = Mesh->GetBoneLocation(Bones::calf_twist_01_l);
				FVector2D LKneeScreen = FVector2D();
				if (!PlayerController->ProjectWorldLocationToScreen(LKnee, &LKneeScreen)) continue;

				FVector LFoot = Mesh->GetBoneLocation(Bones::foot_l);
				FVector2D LFootScreen = FVector2D();
				if (!PlayerController->ProjectWorldLocationToScreen(LFoot, &LFootScreen)) continue;

				FVector2D BonesToCheck[] = { RootScreen, HeadScreen2, NeckScreen, ChestScreen, PelvisScreen, RShoulderScreen, RElbowScreen, RHandScreen, RCalfScreen, RKneeScreen, RFootScreen, LShoulderScreen, LElbowScreen, LHandScreen, LCalfScreen, LKneeScreen, LFootScreen };

				double MostLeft = DBL_MAX;
				double MostRight = DBL_MIN;
				double MostTop = DBL_MAX;
				double MostBottom = DBL_MIN;

				for (int ci = 0; ci < sizeof(BonesToCheck) / sizeof(FVector2D); ci++)
				{
					FVector2D CurrentBone = BonesToCheck[ci];

					if (CurrentBone.X < MostLeft)
						MostLeft = CurrentBone.X;

					if (CurrentBone.X > MostRight)
						MostRight = CurrentBone.X;

					if (CurrentBone.Y < MostTop)
						MostTop = CurrentBone.Y;

					if (CurrentBone.Y > MostBottom)
						MostBottom = CurrentBone.Y;
				}

				double ActorHeight = MostBottom - MostTop;
				double ActorWidth = MostRight - MostLeft;

				double DistanceDifference = 225.0 - Distance;
				double DistanceOffset = DistanceDifference * 0.025;
				double CornerWidth = ActorWidth / 4;
				double CornerHeight = ActorHeight / 3;
				double ThreeDimensionalWidth = ActorWidth / 3;

				double TopTextOffset = 22.0;
				double BottomTextOffset = 7.0;

				FVector2D BottomMiddle = FVector2D(MostLeft + (ActorWidth / 2.0), MostBottom);

				if (Settings::Player::Skeleton)
				{
					Wrapper::Line(HeadScreen, NeckScreen, SkeletonColor, 1.f);
					Wrapper::Line(NeckScreen, ChestScreen, SkeletonColor, 1.f);
					Wrapper::Line(ChestScreen, PelvisScreen, SkeletonColor, 1.f);

					Wrapper::Line(ChestScreen, RShoulderScreen, SkeletonColor, 1.f);
					Wrapper::Line(RShoulderScreen, RElbowScreen, SkeletonColor, 1.f);
					Wrapper::Line(RElbowScreen, RHandScreen, SkeletonColor, 1.f);

					Wrapper::Line(ChestScreen, LShoulderScreen, SkeletonColor, 1.f);
					Wrapper::Line(LShoulderScreen, LElbowScreen, SkeletonColor, 1.f);
					Wrapper::Line(LElbowScreen, LHandScreen, SkeletonColor, 1.f);

					Wrapper::Line(PelvisScreen, RThighScreen, SkeletonColor, 1.f);
					Wrapper::Line(RThighScreen, RCalfScreen, SkeletonColor, 1.f);
					Wrapper::Line(RCalfScreen, RKneeScreen, SkeletonColor, 1.f);
					Wrapper::Line(RKneeScreen, RFootScreen, SkeletonColor, 1.f);

					Wrapper::Line(PelvisScreen, LThighScreen, SkeletonColor, 1.f);
					Wrapper::Line(LThighScreen, LCalfScreen, SkeletonColor, 1.f);
					Wrapper::Line(LCalfScreen, LKneeScreen, SkeletonColor, 1.f);
					Wrapper::Line(LKneeScreen, LFootScreen, SkeletonColor, 1.f);
				}

				if (Settings::Player::BoxType == 1)
				{
					Wrapper::Line(FVector2D(MostLeft - DistanceOffset, MostTop - DistanceOffset), FVector2D(MostRight + DistanceOffset, MostTop - DistanceOffset), BoxColor, 1.5f);
					Wrapper::Line(FVector2D(MostLeft - DistanceOffset, MostBottom + DistanceOffset), FVector2D(MostRight + DistanceOffset, MostBottom + DistanceOffset), BoxColor, 1.5f);
					Wrapper::Line(FVector2D(MostLeft - DistanceOffset, MostBottom + DistanceOffset), FVector2D(MostLeft - DistanceOffset, MostTop - DistanceOffset), BoxColor, 1.5f);
					Wrapper::Line(FVector2D(MostRight + DistanceOffset, MostTop - DistanceOffset), FVector2D(MostRight + DistanceOffset, MostBottom + DistanceOffset), BoxColor, 1.5f);
				}
				else if (Settings::Player::BoxType == 2)
				{
					Wrapper::Line(FVector2D(MostLeft - DistanceOffset, MostTop - DistanceOffset), FVector2D(MostLeft - DistanceOffset + CornerWidth, MostTop - DistanceOffset), BoxColor, 2.f);
					Wrapper::Line(FVector2D(MostLeft - DistanceOffset, MostTop - DistanceOffset), FVector2D(MostLeft - DistanceOffset, MostTop - DistanceOffset + CornerHeight), BoxColor, 2.f);

					Wrapper::Line(FVector2D(MostRight + DistanceOffset, MostTop - DistanceOffset), FVector2D(MostRight + DistanceOffset - CornerWidth, MostTop - DistanceOffset), BoxColor, 2.f);
					Wrapper::Line(FVector2D(MostRight + DistanceOffset, MostTop - DistanceOffset), FVector2D(MostRight + DistanceOffset, MostTop - DistanceOffset + CornerHeight), BoxColor, 2.f);

					Wrapper::Line(FVector2D(MostLeft - DistanceOffset, MostBottom + DistanceOffset), FVector2D(MostLeft - DistanceOffset + CornerWidth, MostBottom + DistanceOffset), BoxColor, 2.f);
					Wrapper::Line(FVector2D(MostLeft - DistanceOffset, MostBottom + DistanceOffset), FVector2D(MostLeft - DistanceOffset, MostBottom + DistanceOffset - CornerHeight), BoxColor, 2.f);

					Wrapper::Line(FVector2D(MostRight + DistanceOffset, MostBottom + DistanceOffset), FVector2D(MostRight + DistanceOffset - CornerWidth, MostBottom + DistanceOffset), BoxColor, 2.f);
					Wrapper::Line(FVector2D(MostRight + DistanceOffset, MostBottom + DistanceOffset), FVector2D(MostRight + DistanceOffset, MostBottom + DistanceOffset - CornerHeight), BoxColor, 2.f);
				}
				else if (Settings::Player::BoxType == 3)
				{
					FVector2D bottom1 = FVector2D(MostLeft + DistanceOffset, MostBottom - DistanceOffset);
					FVector2D bottom2 = FVector2D(MostLeft - DistanceOffset, MostBottom - DistanceOffset);
					FVector2D bottom3 = FVector2D(MostRight - DistanceOffset, MostBottom + DistanceOffset);
					FVector2D bottom4 = FVector2D(MostRight + DistanceOffset, MostBottom + DistanceOffset);

					FVector2D top1 = FVector2D(MostLeft + DistanceOffset, MostTop - DistanceOffset);
					FVector2D top2 = FVector2D(MostLeft - DistanceOffset, MostTop - DistanceOffset);
					FVector2D top3 = FVector2D(MostRight - DistanceOffset, MostTop + DistanceOffset);
					FVector2D top4 = FVector2D(MostRight + DistanceOffset, MostTop + DistanceOffset);

					Wrapper::Line(FVector2D(bottom1.X, bottom1.Y), FVector2D(top1.X, top1.Y), BoxColor, 1.0f);
					Wrapper::Line(FVector2D(bottom2.X, bottom2.Y), FVector2D(top2.X, top2.Y), BoxColor, 1.0f);
					Wrapper::Line(FVector2D(bottom3.X, bottom3.Y), FVector2D(top3.X, top3.Y), BoxColor, 1.0f);
					Wrapper::Line(FVector2D(bottom4.X, bottom4.Y), FVector2D(top4.X, top4.Y), BoxColor, 1.0f);

					Wrapper::Line(FVector2D(bottom1.X, bottom1.Y), FVector2D(bottom2.X, bottom2.Y), BoxColor, 1.0f);
					Wrapper::Line(FVector2D(bottom2.X, bottom2.Y), FVector2D(bottom3.X, bottom3.Y), BoxColor, 1.0f);
					Wrapper::Line(FVector2D(bottom3.X, bottom3.Y), FVector2D(bottom4.X, bottom4.Y), BoxColor, 1.0f);
					Wrapper::Line(FVector2D(bottom4.X, bottom4.Y), FVector2D(bottom1.X, bottom1.Y), BoxColor, 1.0f);

					Wrapper::Line(FVector2D(top1.X, top1.Y), FVector2D(top2.X, top2.Y), BoxColor, 1.0f);
					Wrapper::Line(FVector2D(top2.X, top2.Y), FVector2D(top3.X, top3.Y), BoxColor, 1.0f);
					Wrapper::Line(FVector2D(top3.X, top3.Y), FVector2D(top4.X, top4.Y), BoxColor, 1.0f);
					Wrapper::Line(FVector2D(top4.X, top4.Y), FVector2D(top1.X, top1.Y), BoxColor, 1.0f);
				}

				if (Settings::Player::Lines)
				{
					Wrapper::Line(FVector2D(Variables::ScreenSize.X / 2, Variables::ScreenSize.Y), BottomMiddle, SnaplineColor, 1.0f);
				}

				if (Settings::Player::Distance)
				{
					FString ConvertedText = UKismetStringLibrary::BuildString_Int(L"", L"", int(Distance), L"m");
					Wrapper::Text(ConvertedText, FVector2D(BottomMiddle.X, MostTop - TopTextOffset), FLinearColor(1.f, 1.f, 1.f, 1.f), true, false, false);
					TopTextOffset += 14.0;
				}

				if (Settings::Player::Name)
				{
					if (APlayerState* PlayerState = Player->PlayerState())
					{
						if (FString Username = PlayerState->GetPlayerName())
						{
							FString ConvertedText = UKismetStringLibrary::BuildString_Int(Username, L" [", Team, L"]");
							Wrapper::Text(ConvertedText, FVector2D(BottomMiddle.X, MostTop - TopTextOffset), FLinearColor(1.f, 1.f, 0.f, 1.f), true, false, false);
							TopTextOffset += 14.0;
						}
					}
				}
			}
		}

		if (AcknowledgedPawn)
		{
			if (Settings::Aimbot::AimType)
			{
				if (Settings::Aimbot::DrawFOV)
				{
					Wrapper::Circle(Variables::ScreenCenter, FLinearColor(1.f, 0.f, 0.f, 1.f), AimbotFOV, 128.0);
				}

				FKey AimbotKey = Keys::RightMouseButton;

				if (Settings::Aimbot::KeyType == 1)
					AimbotKey = Keys::LeftMouseButton;
				else if (Settings::Aimbot::KeyType == 2)
					AimbotKey = Keys::LeftShift;

				if (APlayerPawn_Athena_C* Target = TargetPlayer)
				{
					if (Settings::Aimbot::Backtrack) *(float*)(Target + 0x68) = 1.f;

					if (USkeletalMeshComponent* Mesh = Target->Mesh())
					{
						if (AFortWeapon* CurrentWeapon = AcknowledgedPawn->CurrentWeapon())
						{
							bool IsHoldingWeapon = UGameplayStatics::ObjectIsA(CurrentWeapon, RangedWeaponClass);
							if (IsHoldingWeapon)
							{
								bool IsHoldingPickaxe = UGameplayStatics::ObjectIsA(CurrentWeapon, PickaxeClass);
								if (!IsHoldingPickaxe)
								{
									static Bones TargetBoneIndex = Bones::head;

									if (Settings::Aimbot::BoneType == 1)
										TargetBoneIndex = Bones::neck_02;
									else if (Settings::Aimbot::BoneType == 2)
										TargetBoneIndex = Bones::spine_05;

									FVector2D TargetScreen = FVector2D();
									FVector TargetWorld = Mesh->GetBoneLocation(TargetBoneIndex);
									if (PlayerController->ProjectWorldLocationToScreen(TargetWorld, &TargetScreen))
									{
										if (Custom::InCircle(Variables::ScreenCenter.X, Variables::ScreenCenter.Y, AimbotFOV, TargetScreen.X, TargetScreen.Y))
										{
											if (Settings::Aimbot::TargetLine)
											{
												FVector2D MuzzleScreen = FVector2D();
												FVector Muzzle = CurrentWeapon->GetMuzzleLocation();
												if (PlayerController->ProjectWorldLocationToScreen(Muzzle, &MuzzleScreen))
												{
													Wrapper::Line(MuzzleScreen, TargetScreen, Settings::Colors::TargetLine, 1.f);
												}
											}

											if (Settings::Aimbot::AimType == 1 && PlayerController->IsInputKeyDown(AimbotKey))
											{
												FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(Variables::CameraLocation, TargetWorld);

												Custom::AddInput(GWorld, PlayerController, Variables::CameraRotation, TargetRotation, Settings::Aimbot::Interpolate, Settings::Aimbot::Constant, Settings::Aimbot::Speed);
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	} while (false);

	if (Settings::Misc::CrosshairMode)
	{
		if (Settings::Misc::CrosshairMode == 1)
		{
			Wrapper::Line(FVector2D(Variables::ScreenCenter.X - 10, Variables::ScreenCenter.Y), FVector2D(Variables::ScreenCenter.X + 10, Variables::ScreenCenter.Y), FLinearColor(0.6f, 0.6f, 0.6f, 1.f), 1.f);
			Wrapper::Line(FVector2D(Variables::ScreenCenter.X, Variables::ScreenCenter.Y - 10), FVector2D(Variables::ScreenCenter.X, Variables::ScreenCenter.Y + 10), FLinearColor(0.6f, 0.6f, 0.6f, 1.f), 1.f);
		}
		else if (Settings::Misc::CrosshairMode == 2)
		{
			double crosshair_outer = double(14);
			double crosshair_inner = double(8);

			Wrapper::Line(FVector2D(Variables::ScreenCenter.X - crosshair_outer, Variables::ScreenCenter.Y), FVector2D(Variables::ScreenCenter.X - crosshair_inner, Variables::ScreenCenter.Y), FLinearColor(0.6f, 0.6f, 0.6f, 1.f), 2.f);
			Wrapper::Line(FVector2D(Variables::ScreenCenter.X + crosshair_inner, Variables::ScreenCenter.Y), FVector2D(Variables::ScreenCenter.X + crosshair_outer, Variables::ScreenCenter.Y), FLinearColor(0.6f, 0.6f, 0.6f, 1.f), 2.f);
			Wrapper::Line(FVector2D(Variables::ScreenCenter.X, Variables::ScreenCenter.Y - crosshair_outer), FVector2D(Variables::ScreenCenter.X, Variables::ScreenCenter.Y - crosshair_inner), FLinearColor(0.6f, 0.6f, 0.6f, 1.f), 2.f);
			Wrapper::Line(FVector2D(Variables::ScreenCenter.X, Variables::ScreenCenter.Y + crosshair_inner), FVector2D(Variables::ScreenCenter.X, Variables::ScreenCenter.Y + crosshair_outer), FLinearColor(0.6f, 0.6f, 0.6f, 1.f), 2.f);
		}
		else if (Settings::Misc::CrosshairMode == 3)
		{
			Wrapper::Line(FVector2D(Variables::ScreenCenter.X - 10, Variables::ScreenCenter.Y - 10), FVector2D(Variables::ScreenCenter.X + 10, Variables::ScreenCenter.Y + 10), FLinearColor(0.6f, 0.6f, 0.6f, 1.f), 1.f);
			Wrapper::Line(FVector2D(Variables::ScreenCenter.X - 10, Variables::ScreenCenter.Y + 10), FVector2D(Variables::ScreenCenter.X + 10, Variables::ScreenCenter.Y - 10), FLinearColor(0.6f, 0.6f, 0.6f, 1.f), 1.f);

			Wrapper::Line(FVector2D(Variables::ScreenCenter.X - 10, Variables::ScreenCenter.Y - 10), FVector2D(Variables::ScreenCenter.X - 5, Variables::ScreenCenter.Y - 5), FLinearColor(0.f, 0.f, 0.f, 1.f), 2.f);
			Wrapper::Line(FVector2D(Variables::ScreenCenter.X - 10, Variables::ScreenCenter.Y + 10), FVector2D(Variables::ScreenCenter.X - 5, Variables::ScreenCenter.Y + 5), FLinearColor(0.f, 0.f, 0.f, 1.f), 2.f);
			Wrapper::Line(FVector2D(Variables::ScreenCenter.X + 10, Variables::ScreenCenter.Y + 10), FVector2D(Variables::ScreenCenter.X + 5, Variables::ScreenCenter.Y + 5), FLinearColor(0.f, 0.f, 0.f, 1.f), 2.f);
			Wrapper::Line(FVector2D(Variables::ScreenCenter.X + 10, Variables::ScreenCenter.Y - 10), FVector2D(Variables::ScreenCenter.X + 5, Variables::ScreenCenter.Y - 5), FLinearColor(0.f, 0.f, 0.f, 1.f), 2.f);
		}
		else if (Settings::Misc::CrosshairMode == 4)
		{
			Wrapper::Circle(Variables::ScreenCenter, FLinearColor(0.6f, 0.6f, 0.6f, 1.f), 5.0, 38.0, true);
		}
	}

	if (Variables::PlayerController->WasInputKeyJustPressed(Keys::Insert)) Variables::DrawMenu = !Variables::DrawMenu;
	if (Variables::DrawMenu) Framework::DrawMenu();
	Wrapper::Render();
	Wrapper::ClearTargets();

	return DrawTitleSafeAreaOriginal(Viewport, Canvas);
}