#pragma once
#include <Windows.h>
#include <iostream>
#include <cstdint>
#include <array>
#include <future>
#include <winternl.h>

uintptr_t GameBase = 0;

namespace Offsets
{
	// Global Arrays
	int GWorld = 0x117392A8;
	int GObjects = 0x11213D40;

	// Functions
	int StaticFindObject = 0xF361A0;
	int GetBoneMatrix = 0x10CDCC8;
	int GetWeaponStats = 0x151939C;
	int WeaponFireSingle = 0x27A428C;
	int Malloc = 0x101A5B8;
	int Free = 0x3C709A0;

	// Virtual Tables
	int PlayerVtable = 0x6F0;

	// Virtual Indexs
	int DrawTitleSafeArea = 0x380 / 0x8;
	int InputKey = 0xDA8 / 0x8;
	int ProcessEvent = 0x260 / 0x8;
	int GetMaterial = 0x2B0 / 0x8;
	int GetPlayerViewPoint = 0x7F0 / 0x8;
	int GetCameraViewPoint = 0x7F8 / 0x8;
	int GetTargetingTransform = 0x10 / 0x8;
	int FireSingle = 0x880 / 0x8;
	int TryToFire = 0x540 / 0x8;
	int GetReloadTime = 0x988 / 0x8;
	int GetFiringRate = 0x9D8 / 0x8;
	int GetProjectileSpeed = 0x9C8 / 0x8;
	int GetDamageStartLocation = 0x7C0 / 0x8;
}

#define SpoofCode 0x13041052

#include <Helper/Safety/SKCrypter.h>
#include <Helper/Custom/NoCRT.h>

#include <Helper/Custom/Map.h>
#include <Helper/Custom/Vector.h>
#include <Helper/PEB.h>
#include <Helper/SysCall.h>
#include <Helper/SpoofCall.h>

#include <Helper/Custom/Module.h>
#include <Helper/VTableHook.h>

#include <Fortnite/SDK/Structs.h>
#include <Fortnite/SDK/Classes.h>
#include <Fortnite/Settings.h>

namespace Variables
{
	UCanvas* Canvas = nullptr;
	APlayerController* PlayerController = nullptr;
	APlayerPawn_Athena_C* AcknowledgedPawn = nullptr;

	//Variables
	static FVector2D ScreenCenter = FVector2D();
	static FVector2D ScreenSize = FVector2D();

	//Camera
	static FVector CameraLocation = FVector();
	static FRotator CameraRotation = FRotator();
	static float FieldOfView = 0.f;

	// MISC
	static bool DrawMenu = true;
	static bool SetBackToMessage = true;
}

namespace Keys
{
	FKey LeftMouseButton;
	FKey RightMouseButton;
	FKey Insert;
	FKey F2;
	FKey F8;
	FKey W;
	FKey A;
	FKey S;
	FKey D;
	FKey SpaceBar;
	FKey LeftShift;
}

enum Bones : int {
	Root = 0,
	pelvis = 2,
	spine_01 = 3,
	spine_02 = 4,
	spine_03 = 5,
	spine_04 = 6,
	spine_05 = 7,
	clavicle_l = 8,
	upperarm_l = 9,
	lowerarm_l = 10,
	hand_l = 33,
	clavicle_r = 37,
	upperarm_r = 38,
	lowerarm_r = 39,
	hand_r = 62,
	neck_01 = 66,
	neck_02 = 67,
	head = 68,
	thigh_l = 71,
	calf_l = 72,
	calf_twist_01_l = 73,
	calf_twist_02_l = 74,
	foot_l = 76,
	thigh_r = 78,
	calf_r = 79,
	calf_twist_01_r = 80,
	calf_twist_02_r = 81,
	foot_r = 83,
	camera_root = 92,
	Camera = 93,
	attach_fp = 94,
};

#include <Fortnite/Hooks/DrawTitleSafeArea.h>