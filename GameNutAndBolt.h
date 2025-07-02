#pragma once

#include "Game.h"

const int GS_NORMAL				= 1;
const int GS_BOOSTER			= 2;
const int HOLE_NORMAL			= 1;
const int HOLE_LOCK				= 2;
const int HOLE_AD				= 3;
const int REWARD_REVIVE_2		= 1;
const int REWARD_UNLOCK_HOLE	= 2;
const int REWARD_REMOVE_SCREW	= 3;

class CWindowReviveNutAndBolt : public CWindow
{
public:
	void	Load();
	void	Show();
	void	OnClickButton(CButton* pButton);
	void	OnUpdate(float DeltaTime);

public:
	bool	IsClockPause;
	float	ClockTime;
	CText	TextClock;
	CImage	ImageClock;
	CButton ButtonRevive;
	CButton ButtonRestart;
};

class CWindowWinLoseNutAndBolt : public CWindow
{
public:
	void	Load();
	void	Show(bool IsWin);
	void	OnClickButton(CButton* pButton);

public:
	CImage	ImageEmote;
	CButton ButtonRestart, ButtonNext, ButtonHome;
};

class CScrewNutAndBolt : public CFrame
{
public:
	void	Load(int HoleId);
	void	OnSelect();
	void	UnSelect();
	bool	CheckValid();
public:
	int		Mask, HoleId;
	CButton ButtonScrew;
};

class CItemNutAndBolt : public CNode
{
public:
	CItemNutAndBolt();
	~CItemNutAndBolt();

	int		IsHole(CVec2 Pos);
	int		CountScrew();
	void	RefreshState();
	void	Load(int ItemId, int Group1, int Group2, int Layer);
	void	SetColor4B(int x, int y, Color4B Color, unsigned char* Data);
	CVec2	CalculItemPosition(CVec2 *PointList, int Count, CVec2 OffsetCenter = CVec2(0, 0));
	CVec2	CalculItemPositionOffset(CVec2 *PointList, int Count, CVec2 *OffsetList = 0, int OffsetCount = 0, int OffsetId = 1, string Axis = "x");

public:
	int		Group1, Group2, Layer, Mask, Width, Height;
	bool	IsStateChange;
	float	Rotation;
	CVec2	ItemPivot;
	Image	*pImage;
	CImage	ImageItem;
	Texture2D *pTexture;
	PhysicsBody *pPhysicsBody;
	vector<CVec2> HoleList;
	vector<CScrewNutAndBolt*> pScrewList;
};

class CHoleNutAndBolt : public CNode
{
public:
	void	Load(CVec2 Position, int Type = HOLE_NORMAL, bool HasScrew = false);
	bool	CheckValid();
	void	RequestHole();
	void	OnRequestSuccess();

public:
	int		Type, IsRequestAd;
	bool	IsValid;
	CVec2	Position;
	CImage	ImageHole, ImageAd;
	CScrewNutAndBolt *pScrew;
};

class CKeyNutAndBolt : public CNode
{
public:
	void	Load(int Key);
	void	GoUnlock();
	void	OnUpdate(float DeltaTime);
	CVec2	GetPosition();

public:
	int		Id;
	bool	IsActivate, IsUse;
	CVec2	Destination;
	CImage	ImageKey;
};

class CLockNutAndBolt : public CNode
{
public:
	void	Load(int LockId, int HoleId);
	void	Unlock();

public:
	int		Id, HoleId;
	bool	IsUse;
	CImage	ImageLock;
};

class CGameNutAndBolt : public IGame
{
public:
	~CGameNutAndBolt();

	void	Init();
	void	BeginLevel();
	void	RestartGame();
	void	OnUpdate(float DeltaTime);
	void	OnKeyDown(EventKeyboard::KeyCode Key);
	void	OnTouchEnd(int TouchId, float TouchX, float TouchY);
	bool	OnTouchBegin(int TouchId, float TouchX, float TouchY);
	bool	OnQueryPoint(PhysicsWorld& pPhysicsWorld, PhysicsShape& pPhysicsShape, void* UserData);
	bool	OnContactBegin(PhysicsContact& Contact);
	void	Revive();
	void	GameLose();
	void	ShowAdInterstitial();
	void	OnAdShowComplete(int Tag, bool IsReward);
	void	ResetData();
	void	ResetAll();
	void	OnClickButton(CButton *pButton);
	bool	AddScrew(CHoleNutAndBolt *pHole);
	bool	MoveScrew(CVec2 Pos, CScrewNutAndBolt *pScrew);
	bool	CheckHoleCollide(CVec2 HolePos);
	void	RefreshItemState();
	void	ChangeGameState();
	void	SaveLevel();
	CVec2	NormalPosToBoardPos(CVec2 NormalPos);

public:
	int		Score, Level, GameState, ReviveCount, SelectTouchId, SelectHoleId, RewardType;
	bool	IsLose, IsGameWin, IsRevive, IsScrewCollide;
	float	ClockTime, SpeedX, SpeedY, BoardScale;
	CVec2	HoleToCheck, Center, PointBoardZero;
	bigint	TimeEndLevel;
	CText	TextLevel, TextTime, TextBooster;
	CFrame	FrameMap;
	CImage	ImageBoard, ImageBgBooster, ImageClock;
	CButton ButtonUnscrew, ButtonRestart;
	CScrewNutAndBolt *pSelectScrew;
	vector<CKeyNutAndBolt*> pKeyList;
	vector<CHoleNutAndBolt*> pHoleList;
	vector<CItemNutAndBolt*> pItemList;
	vector<CLockNutAndBolt*> pLockList;
	vector<CScrewNutAndBolt*> pScrewList;
	CWindowReviveNutAndBolt WindowRevive;
	CWindowWinLoseNutAndBolt WindowWinLose;
};
