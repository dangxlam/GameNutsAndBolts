#include "Gui.h"
#include "Engine.h"
#include "Client.h"
#include "Function.h"
#include "PluginAd.h"
#include "SoundManager.h"
#include "GameNutAndBolt.h"

const int HOLE_TOUCH_OFFSET		= 40;
const int SCREW_MASK_ORIGIN		= 0b11111111111111111111111100000000;
const float ITEM_SCALE			= 0.67;
const float HOLE_RADIUS			= 50;
const float SHAPE_DENSITY		= 1;
const float SHAPE_RESTITUTION	= 0.5;
const float SHAPE_FRICTION		= 0;
const float BOARD_SCALE_X		= 0.96;

CGameNutAndBolt *pGameNutAndBolt;

class CEffectNutAndBolt : public CEffect
{
public:
	CEffect* CreateFirework()
	{
		// Particle
		int r = e::RandomInt(0, 360);
		for (int i=0; i<60; i++)
		{
			CParticle *p = NewParticle();
			p->Load(e::Format("gui/effect/firework_%i.png", e::RandomInt(1, 8)));
			p->SetScale(c::GUI_SCALE*e::RandomFloat(0.7, 1));
			p->AddAction(CSequence::Create(CAction::Delay(e::RandomFloat(1, 1.2)), CAction::FadeTo(0.2, 0)));
			p->AddAction(CSequence::Create(CAction::Delay(e::RandomFloat(1, 1.2)), CAction::ScaleTo(0.2, 0)));
			p->AddAction(CAction::RotateBy(0.8, e::RandomInt(-30, 30)));
			p->AddParticleAction(CParticleAction::Fire2D(c::GUI_SCALE*e::RandomInt(3200, 4000), CVec2(0, -1).Rotate( e::RandomInt(-8, 8)), c::GUI_SCALE*e::RandomInt(5000, 6000)));
			AddChild(p);
		}

		// ZDeep
		SetZDeep(30);

		// Done
		return this;
	}
};

void CWindowReviveNutAndBolt::Load()
{
	// Load
	CWindow::Load(900, 1290, c::GUI_SCALE, true, false); 

	// Image Clock
	ImageClock.Load("gui/game/clock.png");
	ImageClock.SetScale(c::GUI_SCALE*0.9);
	ImageClock.SetPosition(GetWidth()/2, ImageClock.GetHeight()/2 + c::GUI_SCALE * 253);
	AddChild(&ImageClock);

	// Text Clock
	TextClock.Create(110 * c::GUI_SCALE);
	TextClock.SetText("");
	TextClock.SetPosition(ImageClock.GetX(), ImageClock.GetY());
	AddChild(&TextClock);

	// Button Revive
	ButtonRevive.Load("gui/window/button_red_3_ad.png", 45, c::GUI_SCALE);
	ButtonRevive.SetText("Revive");
	ButtonRevive.SetTextPos(c::GUI_SCALE *45, -c::GUI_SCALE*12);
	ButtonRevive.SetPosition(ImageClock.GetX(), ImageClock.GetY() + ImageClock.GetHeight()/2 + ButtonRevive.GetHeight()/2 + c::GUI_SCALE * 60);
	AddChild(&ButtonRevive);

	// Button Restart
	ButtonRestart.Load("gui/window/button_blue_3.png", 45, c::GUI_SCALE);
	ButtonRestart.SetText("Restart");
	ButtonRestart.SetTextPos(0, -c::GUI_SCALE*12);
	ButtonRestart.SetPosition(ButtonRevive.GetX(),  ButtonRevive.GetY() + ButtonRestart.GetHeight() + c::GUI_SCALE*60);
	AddChild(&ButtonRestart);

	// Gui
	SetZDeep(50);
	SetTitle("Revive");

	// Data
	CloseWhenTouchOutside = false;
}

void CWindowReviveNutAndBolt::Show()
{
	// Show
	CWindow::Show();

	// Data
	ClockTime = 5;
	IsClockPause = false;
}

void CWindowReviveNutAndBolt::OnClickButton(CButton *pButton)
{
	// Window
	CWindow::OnClickButton(pButton);

	// Button Revive
	if (pButton == &ButtonRevive)
	{
		// Data
		IsClockPause = true;
		pGameNutAndBolt->RewardType = REWARD_REVIVE_2;

		// Show Ad
		PluginAd.Show(TAG_REWARDED);
	}

	// Button Restart
	if (pButton == &ButtonRestart)
	{
		pGameNutAndBolt->RestartGame();
		pGameNutAndBolt->ShowAdInterstitial();
	}
}

void CWindowReviveNutAndBolt::OnUpdate(float DeltaTime) 
{
	if (!IsClockPause && ClockTime > -1)
	{
		// Clock
		int OldTime = (int)floor(ClockTime);
		ClockTime -= DeltaTime;
		int NewTime = (int)floor(ClockTime);
		TextClock.SetText(e::Format("%i",(int)ceil(ClockTime)));

		// Tick
		if (NewTime != OldTime)
		{
			if (NewTime > -1) SoundManager.Play("clock_1");
			else if (NewTime > -2) SoundManager.Play("clock_2");
		}
		if (ClockTime <= -1)
		{
			pGameNutAndBolt->GameLose();
			pGameNutAndBolt->ShowAdInterstitial();
		}
	}
}

void CWindowWinLoseNutAndBolt::Load()
{
	// Load
	CWindow::Load(900, 1240, c::GUI_SCALE, true, false);

	// Image Emote
	ImageEmote.Load("gui/game/emote_win.png");
	ImageEmote.SetScale(c::GUI_SCALE*1.5);
	ImageEmote.SetAnchorPointScale(0.5, 1);
	ImageEmote.SetPosition(GetWidth()/2, c::GUI_SCALE * 555);
	AddChild(&ImageEmote);

	// Button Home
	ButtonHome.Load("gui/window/button_blue_3.png", 48, c::GUI_SCALE);
	ButtonHome.SetText("Home");
	ButtonHome.SetTextPos(0, -c::GUI_SCALE*12);
	ButtonHome.SetPosition(ImageEmote.GetX(), ImageEmote.GetY() + ButtonHome.GetHeight()/2 + c::GUI_SCALE*60);
	AddChild(&ButtonHome);

	// Button Restart
	ButtonRestart.Load("gui/window/button_blue_3.png", 48, c::GUI_SCALE);
	ButtonRestart.SetText("Restart");
	ButtonRestart.SetTextPos(0, -c::GUI_SCALE*12);
	ButtonRestart.SetPosition(ButtonHome.GetX(), ButtonHome.GetY() + ButtonRestart.GetHeight() + c::GUI_SCALE * 60);
	AddChild(&ButtonRestart);

	// Button Next
	ButtonNext.Load("gui/window/button_blue_3.png", 48, c::GUI_SCALE);
	ButtonNext.SetText("Next");
	ButtonNext.SetTextPos(0, -c::GUI_SCALE*12);
	ButtonNext.SetPosition(ButtonHome.GetX(), ButtonHome.GetY() + ButtonRestart.GetHeight() + c::GUI_SCALE * 60);
	AddChild(&ButtonNext);

	// Gui
	SetZDeep(50);

	// Data
	CloseWhenTouchOutside = false;
}

void CWindowWinLoseNutAndBolt::Show(bool IsWin)
{
	// Show
	CWindow::Show();

	// Gui
	TextTitle.SetText(e::Format("%s", IsWin?"You Win":"You Lose"));
	ImageEmote.Update((IsWin)? "gui/game/emote_win.png":"gui/game/emote_lose.png");
	ButtonNext.SetVisible(IsWin);
	ButtonRestart.SetVisible(!IsWin);
}

void CWindowWinLoseNutAndBolt::OnClickButton(CButton *pButton)
{
	// Window
	CWindow::OnClickButton(pButton);

	// Button Restart
	if (pButton == &ButtonRestart)
	{
		pGameNutAndBolt->RestartGame();
	}

	// Button Home
	if (pButton == &ButtonHome)
	{
		Game.pGame->Release();
	}

	// Button Next
	if (pButton == &ButtonNext)
	{
		pGameNutAndBolt->BeginLevel();
	}
}

CItemNutAndBolt::CItemNutAndBolt()
{
	pImage = 0;
	pTexture = 0;
}

CItemNutAndBolt::~CItemNutAndBolt()
{
	if (pImage) delete pImage;
}

void CItemNutAndBolt::Load(int ItemId, int Group1, int Group2, int Layer)
{
	// Create
	Create();

	// Load
	string Line;
	CFile File;
	File.Load(e::Format("game_nut_and_bolt/item/item_%i_%i.txt", Group1, Group2));
	pPhysicsBody = PhysicsBody::create();
	while (File.ReadLine(Line))
	{
		if (Line == "[size]")
		{
			File.ReadLine(Line);
			vector<string> WordList;
			e::SplitWord(Line, WordList);
			Width = stoi(WordList[0]);
			Height = stoi(WordList[1]);
		}
		if (Line == "[pivot]")
		{
			File.ReadLine(Line);
			vector<string> WordList;
			e::SplitWord(Line, WordList);
			ItemPivot = CVec2(stof(WordList[0]), stof(WordList[1]));
		}
		if (Line == "[polygon_begin]")
		{
			Vec2 PointPolyList[40];
			int PolyPointNum = 0;
			File.ReadLine(Line);
			while(Line != "[polygon_end]")
			{
				vector<string> WordList;
				e::SplitWord(Line, WordList);
				PointPolyList[PolyPointNum] =  Vec2(-ItemPivot.x, ItemPivot.y) - Vec2(-stoi(WordList[0]), stoi(WordList[1]));
				File.ReadLine(Line);
				PolyPointNum++;
			}
			auto Shape = PhysicsShapePolygon::create(PointPolyList, PolyPointNum , PhysicsMaterial(SHAPE_DENSITY, SHAPE_RESTITUTION, SHAPE_FRICTION));
			pPhysicsBody->addShape(Shape);
		}
		if (Line == "[shape_circle]")
		{
			float Radius = File.ReadFloat();
			auto Shape = PhysicsShapeCircle::create(Radius, PhysicsMaterial(SHAPE_DENSITY, SHAPE_RESTITUTION, SHAPE_FRICTION));
			pPhysicsBody->addShape(Shape);
		}
	}

	// Bitmask
	string BitArr = "00000000000000000000000000000000";
	BitArr[32 - Layer] = '1';
	BitArr[24 - ItemId] = '1';
	Mask = 0;
	for(int i = 31; i >= 0; i--)
	{
		if (BitArr[i] == '1') Mask += pow(2, 31 - i);
	}

	// Physic Body
	pPhysicsBody->setDynamic(false);
	pPhysicsBody->setCategoryBitmask(Mask);
	pPhysicsBody->setCollisionBitmask(Mask);
	pPhysicsBody->setContactTestBitmask(Mask);
	AddComponent(pPhysicsBody);

	// Image
	pImage = new Image();
	string ImageFile;
	string Color;
	if (Layer == 1) Color = "grey";
	else if (Layer == 2) Color = "purple";
	else if (Layer == 3) Color = "orange";
	else if (Layer == 4) Color = "red";
	else if (Layer == 5) Color = "blue";
	else if (Layer == 6) Color = "green";
	else if (Layer == 7) Color = "brown";
	else Color = "blue";
	if (Group1 == 1) ImageFile = e::Format("game_nut_and_bolt/item/item_%i_%i_%s.png", Group1, Group2, Color.c_str());
	else ImageFile = e::Format("game_nut_and_bolt/item/item_%i_%i.png", Group1, Group2);
	pImage->initWithImageFile(ImageFile);

	// Texture
	pTexture = new Texture2D();
	pTexture->initWithImage(pImage);

	// Image Item
	ImageItem.LoadTexture(pTexture);
	ImageItem.SetAnchorPoint(ItemPivot.x, ItemPivot.y);
	AddChild(&ImageItem);

	// Gui
	SetZDeep(Layer);
	SetScale(c::GUI_SCALE*ITEM_SCALE);
	SetRotation(Rotation);

	// Data
	this->Layer = Layer;
	this->Group1 = Group1;
	this->Group2 = Group2;
	this->IsStateChange = true;
}

void CItemNutAndBolt::SetColor4B(int x, int y, Color4B Color, unsigned char* Data)
{
	int index = (x + y * Width) << 2;
	Data[index] = Color.r;
	Data[index + 1] = Color.g;
	Data[index + 2] = Color.b;
	Data[index + 3] = Color.a;
}

CVec2 CItemNutAndBolt::CalculItemPosition(CVec2 *PointList, int Count, CVec2 OffsetCenter /* = CVec2(0, 0) */)
{
	// Data
	CVec2 Pos, Offset;
	CVec2 FirstPoint = PointList[0];
	CVec2 SecondPoint = PointList[1];

	// One Hole
	if (Count == 1)
	{
		// Hole
		Offset = CVec2(-OffsetCenter.x , OffsetCenter.y);
		HoleList.push_back(ItemPivot + (Offset));

		// Rotation
		Offset *= c::GUI_SCALE*ITEM_SCALE;
		Pos = PointList[0] - Offset;
	}
	// Multi Hole
	else
	{
		// Distance
		CVec2 Dis1 = PointList[1] - FirstPoint;
		for(int i = 2; i < Count; i++)
		{
			if ((PointList[i] - FirstPoint).Length() > Dis1.Length()) Dis1 = PointList[i] - FirstPoint;
		}
		float OffsetX2 = (Dis1.Length()/(c::GUI_SCALE * ITEM_SCALE))/2;
		Offset = CVec2(-OffsetX2 - OffsetCenter.x, OffsetCenter.y);
		Dis1 = PointList[1] - FirstPoint;
		CVec2 Dis2 = Offset;
		Dis2 *= c::GUI_SCALE*ITEM_SCALE;

		// Rotation
		CVec2 AxisAlign = CVec2(1,0); 
		float t = 0;
		t = Dis1.GetAngle(AxisAlign);
		SetRotation(t);

		// Hole
		HoleList.push_back(ItemPivot + Offset);
		for(int i = 1; i < Count; i++)
		{
			CVec2 Point = PointList[i];
			HoleList.push_back((ItemPivot + Offset) + ((Point - FirstPoint)/(c::GUI_SCALE*ITEM_SCALE)).Rotate(-t)); 
		}

		// Position
		Pos = FirstPoint - Dis2.Rotate(t);
	}

	// Texture
	for(int k = 0; k < HoleList.size(); k++)
	{
		CVec2 Hole = HoleList[k];
		for(int i = Hole.x - (HOLE_RADIUS + 10); i < Hole.x + (HOLE_RADIUS + 10); i++)
		{
			for(int j = Hole.y - (HOLE_RADIUS + 10); j < Hole.y + (HOLE_RADIUS + 10); j++)
			{
				if (i < 0) i = 0;
				if (j < 0) j = 0;
				if (i >= Width) continue;
				if (j >= Height) continue;
				CVec2 Temp(i, j);
				if ((Temp - Hole).Length() < HOLE_RADIUS - 2) SetColor4B(i, j, Color4B(0, 0, 0, 0), pImage->getData());
				else if ((Temp - Hole).Length() < HOLE_RADIUS + 0) SetColor4B(i, j, Color4B(0, 0, 0, 255), pImage->getData());
			}
		}
	}
	pTexture->updateWithData(pImage->getData(), 0, 0, Width, Height);

	// Done
	return Pos;
}

CVec2 CItemNutAndBolt::CalculItemPositionOffset(CVec2 *PointList, int Count, CVec2 *OffsetList /*= 0*/, int OffsetCount /*= 0*/, int OffsetId /*= 1*/, string Axis /*= "x"*/)
{
	// Data
	CVec2 Pos;
	CVec2 FirstPoint = PointList[0];
	CVec2 SecondPoint = PointList[1];
	CVec2 OffsetFinal = (OffsetList == 0)? ItemPivot : OffsetList[0];

	// One Hole
	if (Count == 1)
	{
		if (OffsetId == 1)
		{
			if (OffsetFinal == ItemPivot) OffsetFinal = CVec2(0 , 0);
			HoleList.push_back(ItemPivot + (OffsetFinal));
			float Rotation = 0;
			if (Axis == "x") Rotation = 0;
			else if (Axis == "y") Rotation = 90;
			else Rotation = stof(Axis);

			// Rotation
			SetRotation(Rotation);
			OffsetFinal *= c::GUI_SCALE*ITEM_SCALE;
			Pos = PointList[0] - OffsetFinal.Rotate(Rotation);
		}
	}
	// Multi Hole
	else
	{
		if (OffsetId == 1)
		{
			if (OffsetCount <= 1)
			{
				CVec2 Dis1 = PointList[1] - FirstPoint;
				for(int i = 2; i < Count; i++)
				{
					if ((PointList[i] - FirstPoint).Length() > Dis1.Length()) Dis1 = PointList[i] - FirstPoint;
				}
				float OffsetX = (Dis1.Length()/(c::GUI_SCALE * ITEM_SCALE))/2;
				if (OffsetFinal == ItemPivot) OffsetFinal = CVec2(-OffsetX, 0);
				Dis1 = PointList[1] - FirstPoint;
				CVec2 Dis2 = OffsetFinal;
				Dis2 *= c::GUI_SCALE*ITEM_SCALE;
				CVec2 AxisAlign = CVec2(1,0); 
				if (Axis == "x") AxisAlign = CVec2(1,0);
				else if (Axis == "y") AxisAlign = CVec2(0, 1);
				else if (Axis == "-x") AxisAlign = CVec2(-1, 0);
				else if (Axis == "-y") AxisAlign = CVec2(0, -1);
				float t = 0;
				t = Dis1.GetAngle(AxisAlign);
				SetRotation(t);
				HoleList.push_back(ItemPivot + OffsetFinal);
				for(int i = 1; i < Count; i++)
				{
					CVec2 Point = PointList[i];
					HoleList.push_back( (ItemPivot + OffsetFinal) + ((Point - FirstPoint) / (c::GUI_SCALE*ITEM_SCALE)).Rotate(-t)); //
				}
				Pos = FirstPoint - Dis2.Rotate(t);
			}
			else
			{
				for(int i = 0; i < OffsetCount; i++)
				{
					HoleList.push_back(ItemPivot + OffsetList[i]);
				}
				CVec2 Dis1 = HoleList[1] - HoleList[0];
				CVec2 Dis2 = ItemPivot - HoleList[0];
				float Rt = Dis2.GetAngle(Dis1);
				CVec2 Dis3 = PointList[1] - PointList[0];
				Dis3.Rotate(Rt);
				Dis3.Normalize(Dis2.Length()*c::GUI_SCALE*ITEM_SCALE);
				Pos = PointList[0] + Dis3;
				Dis3 = PointList[1] - PointList[0];
				float Rotation = Dis3.GetAngle(Dis1);
				SetRotation(Rotation);
			}
		}
	}

	// Texture
	for(int k = 0; k < HoleList.size(); k++)
	{
		CVec2 Hole = HoleList[k];
		for(int i = Hole.x - (HOLE_RADIUS + 10); i < Hole.x + (HOLE_RADIUS + 10); i++)
		{
			for(int j = Hole.y - (HOLE_RADIUS + 10); j < Hole.y + (HOLE_RADIUS + 10); j++)
			{
				if (i < 0) i = 0;
				if (j < 0) j = 0;
				if (i >= Width) continue;
				if (j >= Height) continue;
				CVec2 Temp(i, j);
				if ((Temp - Hole).Length() < HOLE_RADIUS - 2) SetColor4B(i, j, Color4B(0, 0, 0, 0), pImage->getData());
				else if ((Temp - Hole).Length() < HOLE_RADIUS + 0) SetColor4B(i, j, Color4B(0, 0, 0, 255), pImage->getData());
			}
		}
	}
	pTexture->updateWithData(pImage->getData(), 0, 0, Width, Height);

	// Done
	return Pos;
}

int CItemNutAndBolt::CountScrew()
{
	return pScrewList.size();
}

int CItemNutAndBolt::IsHole(CVec2 Pos)
{
	for(int i = 0; i < HoleList.size(); i++)
	{
		CVec2 Tmp2 = ((HoleList[i] - ItemPivot)*ITEM_SCALE*c::GUI_SCALE);
		float Rotation = pNode->getRotation();
		CVec2 Tmp = CVec2(GetX(), GetY()) + ((HoleList[i] - ItemPivot)*ITEM_SCALE*c::GUI_SCALE).Rotate(Rotation);
		if ((Tmp - Pos).Length() < 5) return i;
	}
	return -1;
}

void CItemNutAndBolt::RefreshState()
{
	// Data
	IsStateChange = false;

	// Multi Screw
	if (CountScrew() > 1)
	{
		auto pJoint = pPhysicsBody->getJoints();
		if (!pJoint.empty())
		{
			auto pDirector = Director::getInstance();
			auto pScene = pDirector->getRunningScene();
			pScene->getPhysicsWorld()->removeJoint(pJoint[0]);
		}
		pPhysicsBody->setDynamic(false);
	}
	// None/One Screw
	else
	{
		// Physic
		pPhysicsBody->setDynamic(true);

		// One Screw
		if (CountScrew() == 1)
		{
			auto pJointCheck = pPhysicsBody->getJoints();
			if (!pJointCheck.empty())
			{
				auto pDirector = Director::getInstance();
				auto pScene = pDirector->getRunningScene();
				pScene->getPhysicsWorld()->removeJoint(pJointCheck[0]);
			}
			auto pPhysicsScrew = pScrewList[0]->pNode->getPhysicsBody();
			CVec2 ScrewPos = CVec2(pScrewList[0]->GetX(), pScrewList[0]->GetY());
			int HoleId = 0;
			for(int j = 0; j < HoleList.size(); j++)
			{
				CVec2 Tmp2 = ((HoleList[j] - ItemPivot)*ITEM_SCALE*c::GUI_SCALE);
				float rt = pNode->getRotation();
				CVec2 Tmp = CVec2(GetX(), GetY()) + ((HoleList[j] - ItemPivot)*ITEM_SCALE*c::GUI_SCALE).Rotate(pNode->getRotation());
				CVec2 Dis = Tmp - ScrewPos;
				if (Dis.Length() < 5)
				{
					HoleId = j;
					break;
				}
			}
			PhysicsJointLimit* pJoint = PhysicsJointLimit::construct(pPhysicsScrew, pPhysicsBody, Point::ZERO, Point((HoleList[HoleId] - ItemPivot).x*c::GUI_SCALE*ITEM_SCALE, (ItemPivot - HoleList[HoleId]).y*c::GUI_SCALE*ITEM_SCALE), 0, 0);
			pJoint->setCollisionEnable(false);
			pJoint->createConstraints();
			auto pDirector = Director::getInstance();
			auto pScene = pDirector->getRunningScene();
			pScene->getPhysicsWorld()->addJoint(pJoint);
		}
		// None Screw
		else
		{
			auto pJoint = pNode->getPhysicsBody()->getJoints();
			if (!pJoint.empty())
			{
				auto pDirector = Director::getInstance();
				auto pScene = pDirector->getRunningScene();
				pScene->getPhysicsWorld()->removeJoint(pJoint[0]);
			}
		}
	}
}

void CScrewNutAndBolt::Load(int HoleId)
{
	// Create
	Create();

	// Button Screw
	ButtonScrew.Load("game_nut_and_bolt/screw.png", 40);
	ButtonScrew.SoundEnable = false;
	AddChild(&ButtonScrew);

	// Data
	this->HoleId = HoleId;

	// Gui
	SetZDeep(10);
	SetScale(c::GUI_SCALE*ITEM_SCALE);
}

void CScrewNutAndBolt::OnSelect()
{
	SetZDeep(11);
	ButtonScrew.SetPosition(CVec2(0, -100));
	ButtonScrew.BgActive.Update("game_nut_and_bolt/screw_remove.png");
}

void CScrewNutAndBolt::UnSelect()
{
	SetZDeep(10);
	ButtonScrew.SetPosition(CVec2(0, 0));
	ButtonScrew.BgActive.Update("game_nut_and_bolt/screw.png");
}

bool CScrewNutAndBolt::CheckValid()
{
	bool IsValid = true;
	for(int i = 0; i < pGameNutAndBolt->pLockList.size(); i++)
	{
		if (pGameNutAndBolt->pLockList[i]->HoleId == this->HoleId)
		{
			IsValid = false;
			break;
		}
	}
	return IsValid;
}

void CHoleNutAndBolt::Load(CVec2 Position, int Type /*= HOLE_NORMAL*/, bool HasScrew /*= false*/)
{
	// Create
	Create();

	// Data
	this->Type = Type;
	this->Position = Position;
	this->pScrew = 0;
	IsValid = true;
	IsRequestAd = false;

	// Gui
	SetZDeep(0);
	SetScale(ITEM_SCALE*c::GUI_SCALE);

	// Image Hole
	ImageHole.Load("game_nut_and_bolt/board_hole.png");
	AddChild(&ImageHole);
	if (Type == HOLE_AD)
	{
		ImageAd.Load("game_nut_and_bolt/ad.png");
		ImageAd.SetScale(1);
		ImageAd.SetPosition(ImageHole.GetWidth()/2 - ImageAd.GetWidth()/2, ImageHole.GetHeight()/2 - ImageAd.GetHeight()/2);
		AddChild(&ImageAd);
	}
}

void CHoleNutAndBolt::RequestHole()
{
	if (Type == HOLE_AD)
	{
		IsRequestAd = true;
		pGameNutAndBolt->RewardType = REWARD_UNLOCK_HOLE;
		pGameNutAndBolt->PluginAd.Show(TAG_REWARDED);
	}
}

void CHoleNutAndBolt::OnRequestSuccess()
{
	// Data
	Type = HOLE_NORMAL;

	// Image Ad
	if (ImageAd.IsCreate()) RemoveChild(&ImageAd);

	// Sound
	pGameNutAndBolt->SoundManager.Play("game_nut_and_bolt/lock_unlock");
}

bool CHoleNutAndBolt::CheckValid()
{
	if (Type == HOLE_NORMAL && pScrew == 0) return true;
	return false;
}

void CLockNutAndBolt::Load(int LockId, int HoleId)
{
	// Create
	Create();

	// Data
	IsUse = true;
	this->Id = LockId;
	this->HoleId = HoleId;

	// Gui
	SetZDeep(11);
	SetScale(c::GUI_SCALE*ITEM_SCALE);

	// Image Lock
	ImageLock.Load("game_nut_and_bolt/item/item_lock.png");
	AddChild(&ImageLock);
}

void CLockNutAndBolt::Unlock()
{
	if (IsUse)
	{
		IsUse = false;
		pGameNutAndBolt->SoundManager.Play("game_nut_and_bolt/lock_unlock");
	}
}

void CKeyNutAndBolt::Load(int Key)
{
	// Create
	Create();

	// Image Key
	ImageKey.Load("game_nut_and_bolt/item/item_key.png");
	AddChild(&ImageKey);

	// Physic
	int MaskBit = 0b11111111111111111111111100000000;
	auto pPhysicsbody = PhysicsBody::createCircle(HOLE_RADIUS*0.9, PhysicsMaterial(SHAPE_DENSITY, SHAPE_RESTITUTION, SHAPE_FRICTION));
	pPhysicsbody->setDynamic(false);
	pPhysicsbody->setCategoryBitmask(MaskBit);
	pPhysicsbody->setCollisionBitmask(0b00000000000000000000000000000000);
	pPhysicsbody->setContactTestBitmask(MaskBit);
	AddComponent(pPhysicsbody);

	// Data
	Id = Key;
	IsUse = true;
	IsActivate = false;

	// Gui
	SetZDeep(11);
	SetScale(c::GUI_SCALE*ITEM_SCALE);
}

void CKeyNutAndBolt::GoUnlock()
{
	// Data
	IsActivate = true;

	// Lock
	for(int i = 0;i < pGameNutAndBolt->pLockList.size(); i++)
	{
		if (pGameNutAndBolt->pLockList[i]->Id == this->Id)
		{
			Destination = pGameNutAndBolt->pHoleList[pGameNutAndBolt->pLockList[i]->HoleId]->Position;
		}
	}
}

CVec2 CKeyNutAndBolt::GetPosition()
{
	return CVec2(GetX(), GetY());
}

void CKeyNutAndBolt::OnUpdate(float DeltaTime)
{
	if (IsActivate && GetPosition() != Destination)
	{
		float Distance = 1200 * DeltaTime * c::GUI_SCALE;
		CVec2 Dir = Destination - GetPosition();
		if (Dir.Length() <= Distance)
		{
			this->SetPosition(Destination);
			for (int i = 0; i < pGameNutAndBolt->pLockList.size(); i++)
			{
				if (pGameNutAndBolt->pLockList[i]->IsUse && pGameNutAndBolt->pLockList[i]->Id == Id)
				{
					IsUse = false;
					pGameNutAndBolt->pLockList[i]->Unlock();
				}
			}
		}
		else
		{
			Dir.Normalize(Distance);
			this->SetPosition(GetPosition() + Dir);
		}
	}
}

void CGameNutAndBolt::ShowAdInterstitial()
{
	if (Level >= Client.AdLevelNutAndBolt && PluginAd.PlayTime >= Client.AdTimeNutAndBolt)
	{
		if (IsUse) PluginAd.Show(TAG_INTERSTITIAL);
		else Game.IsShowAd = true;
	}
}

void CGameNutAndBolt::OnAdShowComplete(int Tag, bool IsReward)
{
	if (Tag == TAG_REWARDED)
	{
		if (IsReward)
		{
			if (RewardType == REWARD_UNLOCK_HOLE)
			{
				for(int i = 0; i < pHoleList.size(); i++)
				{
					if (pHoleList[i]->IsRequestAd && pHoleList[i]->Type == HOLE_AD) pHoleList[i]->OnRequestSuccess();
				}
			}
			else if (RewardType == REWARD_REVIVE_2)
			{
				Revive();
			}
			else if (RewardType == REWARD_REMOVE_SCREW)
			{
				ChangeGameState();
			}
			RewardType = 0;
		}
		else
		{
			WindowRevive.IsClockPause = false;
		}
	}
}

CGameNutAndBolt::~CGameNutAndBolt()
{
	ResetData();
}

void CGameNutAndBolt::Init()
{
	// Create
	Create();

	// Data
	Level = 1;
	BoardScale = 0.95;
	SpeedX = SpeedY = 0;
	GameState = GS_NORMAL;
	TimeEndLevel = ClockTime = 0;
	Score = ReviveCount = RewardType = 0;
	IsLose = IsGameWin = IsRevive = IsScrewCollide = false;
	Center.Create(c::WINDOW_WIDTH/2, 0.5*c::WINDOW_HEIGHT);
	pGameNutAndBolt = this;

	// Level
	string s;
	Level = Function.GetSetting("NutAndBoltLevel", s)?stoi(s):1;

	// Image Bg
	CImage ImageBg;
	ImageBg.Load("game_nut_and_bolt/bg.png");
	ImageBg.SetAnchorPoint(0, 0);
	ImageBg.SetScale(c::WINDOW_WIDTH / ImageBg.GetWidth(), c::WINDOW_HEIGHT / ImageBg.GetHeight());
	AddChild(&ImageBg);

	// Image Bg Booster
	ImageBgBooster.Load("game_nut_and_bolt/bg_black.png");
	ImageBgBooster.SetAnchorPoint(0, 0);
	ImageBgBooster.SetScale(c::WINDOW_WIDTH / ImageBgBooster.GetWidth(), c::WINDOW_HEIGHT / ImageBgBooster.GetHeight());
	ImageBgBooster.SetZDeep(0.1);
	ImageBgBooster.SetVisible(false);
	AddChild(&ImageBgBooster);

	// Text Level
	TextLevel.Create(65 * c::GUI_SCALE);
	TextLevel.SetColor(0xffffff);
	TextLevel.SetPosition(c::WINDOW_WIDTH/2, c::MARGIN_TOP + c::GUI_SCALE*100 + TextLevel.GetHeight()/2);
	AddChild(&TextLevel);

	// Image Clock
	ImageClock.Load("game_nut_and_bolt/clock.png");
	ImageClock.SetScale(c::GUI_SCALE*0.8);
	ImageClock.SetPosition(TextLevel.GetX() - ImageClock.GetWidth()/2 - c::GUI_SCALE*30, TextLevel.GetY() + 100*c::GUI_SCALE);
	AddChild(&ImageClock);

	// Text Time
	TextTime.Create(55 * c::GUI_SCALE);
	TextTime.SetColor(0xfcca4d);
	TextTime.SetAlign(ALIGN_LEFT, ALIGN_CENTER);
	TextTime.SetPosition(ImageClock.GetX() + ImageClock.GetWidth()/2  + c::GUI_SCALE * 15, ImageClock.GetY());
	AddChild(&TextTime);

	// Frame Map
	FrameMap.Create();
	FrameMap.SetPosition(0,0);
	FrameMap.SetZDeep(1);
	AddChild(&FrameMap);

	// Button Unscrew
	ButtonUnscrew.Load("game_nut_and_bolt/button_unscrew.png", 40, c::GUI_SCALE);
	ButtonUnscrew.SetPosition(c::WINDOW_WIDTH - ButtonUnscrew.GetWidth()/ 2 - c::GUI_SCALE*70, c::WINDOW_HEIGHT - c::MARGIN_BOTTOM - c::GUI_SCALE*60 - ButtonUnscrew.GetHeight()/2);
	ButtonUnscrew.SetZDeep(10);
	AddChild(&ButtonUnscrew);

	// Button Restart
	ButtonRestart.Load("game_nut_and_bolt/button_restart.png", 40, c::GUI_SCALE);
	ButtonRestart.SetPosition(Gui.ButtonHome.GetX(),  Gui.ButtonHome.GetY() + Gui.ButtonHome.GetHeight()/2 + ButtonRestart.GetHeight()/2 + c::GUI_SCALE*30);
	ButtonRestart.SetZDeep(10);
	AddChild(&ButtonRestart);

	// Text Booster
	TextBooster.Create(65 * c::GUI_SCALE);
	TextBooster.SetColor(0xfcca4d);
	TextBooster.SetText("Tap a bolt\nto remove it");
	TextBooster.SetPosition(c::WINDOW_WIDTH/2, Gui.ButtonHome.GetY() + Gui.ButtonHome.GetHeight()/2 + c::GUI_SCALE*10);
	TextBooster.SetZDeep(0.2);
	TextBooster.SetVisible(false);
	AddChild(&TextBooster);

	// Sound
	SoundManager.Load("game_nut_and_bolt/level_begin");
	SoundManager.Load("game_nut_and_bolt/game_win");
	SoundManager.Load("game_nut_and_bolt/game_over");
	SoundManager.Load("game_nut_and_bolt/screw_in");
	SoundManager.Load("game_nut_and_bolt/screw_out");
	SoundManager.Load("game_nut_and_bolt/key_get");
	SoundManager.Load("game_nut_and_bolt/lock_unlock");
	SoundManager.Load("clock_1");
	SoundManager.Load("clock_2");

	// Window Revive
	WindowRevive.Load();
	AddChild(&WindowRevive);

	// Window Lose
	WindowWinLose.Load();
	AddChild(&WindowWinLose);

	// Event Contact
	auto pEventContact = EventListenerPhysicsContact::create();
	pEventContact->onContactBegin = CC_CALLBACK_1(CGameNutAndBolt::OnContactBegin, this);
	GetEventDispatcher()->addEventListenerWithSceneGraphPriority(pEventContact, pNode);

	// Begin
	BeginLevel();
}

void CGameNutAndBolt::BeginLevel()
{
	// Reset
	ResetAll();

	// Text Level
	TextLevel.SetText(e::Format("Level %i", Level));

	// Sound
	SoundManager.Play("game_nut_and_bolt/level_begin");

	// Image Board
	ImageBoard.Load("game_nut_and_bolt/board.png");
	ImageBoard.SetScale(c::GUI_SCALE*BoardScale*BOARD_SCALE_X, c::GUI_SCALE*BoardScale);
	ImageBoard.SetPosition(Center);
	ImageBoard.SetZDeep(-0.1);
	FrameMap.AddChild(&ImageBoard);
	while(ButtonUnscrew.GetY() - ButtonUnscrew.GetHeight()/2 - (Center.y + ImageBoard.GetHeight()/2) < 40 && BoardScale > 0.8)
	{
		BoardScale -= 0.005;
		ImageBoard.SetScale(c::GUI_SCALE*BoardScale*BOARD_SCALE_X, c::GUI_SCALE*BoardScale);
	}

	// Board Zero
	PointBoardZero = Center - CVec2(ImageBoard.GetWidth()/2, ImageBoard.GetHeight()/2);

	// File
	string Line;
	CFile File;
	int TempLevel = (Level > 150)? (50 + ((Level%150 > 100)? (Level%150 - 100) : Level%150 )): Level;
	File.Load(e::Format("game_nut_and_bolt/level/level_%s%i.txt", (TempLevel < 10)?"00":((TempLevel < 100)? "0" : ""), TempLevel));
	vector<int> ScrewIdList;
	bool IsAlignHole = false;
	CVec2 VecAlign = CVec2(0,0);
	CVec2 OffsetAlign = CVec2(0,0);
	float MinHoleScrewY =  3000, MinHoleEmptyY = 3000, MinHoleScrewX = 3000;
	float MaxHoleScrewY = 0, MaxHoleEmptyY = 0, MaxHoleScrewX = 0;
	while (File.ReadLine(Line))
	{
		// Clock
		if (Line == "[time]")
		{
			ClockTime = File.ReadInt();
		}

		// Hole
		if (Line == "[hole]")
		{
			File.ReadLine(Line);
			vector<string> WordList;
			e::SplitWord(Line, WordList);
			CVec2 Pos(stof(WordList[0]), stof(WordList[1]));
			Pos = NormalPosToBoardPos(Pos);
			CHoleNutAndBolt *pHole = new CHoleNutAndBolt();
			bool HasScrew = (WordList.size() > 2)? ((WordList[2] == "s")? true : false) : false;
			pHole->Load(Pos, (WordList.size() > 2)? ((WordList[2] == "a")? HOLE_AD : HOLE_NORMAL ): HOLE_NORMAL);
			pHole->SetPosition(Pos);
			FrameMap.AddChild(pHole);
			pHoleList.push_back(pHole);
			if (HasScrew)
			{
				ScrewIdList.push_back(pHoleList.size() - 1);
				if (Pos.x > MaxHoleScrewX) MaxHoleScrewX = Pos.x;
				if (Pos.x < MinHoleScrewX) MinHoleScrewX = Pos.x;
				if (Pos.y > MaxHoleScrewY) MaxHoleScrewY = Pos.y;
				if (Pos.y < MinHoleScrewY) MinHoleScrewY = Pos.y;
			}
			else
			{
				if (Pos.y > MaxHoleEmptyY) MaxHoleEmptyY = Pos.y;
				if (Pos.y < MinHoleEmptyY) MinHoleEmptyY = Pos.y;
			}
		}

		// Offset X
		if (Line == "[offset_x]")
		{
			OffsetAlign.x = File.ReadFloat();
			OffsetAlign.x *= (c::GUI_SCALE * ITEM_SCALE);
		} 

		// Offset Y
		if (Line == "[offset_y]")
		{
			OffsetAlign.y = File.ReadFloat();
			OffsetAlign.y *= (c::GUI_SCALE * ITEM_SCALE);
		}

		// Item
		if (Line == "[item]")
		{
			// Align Hole
			if (!IsAlignHole)
			{
				IsAlignHole = true;
				if (MinHoleEmptyY < MinHoleScrewY) MinHoleScrewY -= (MinHoleScrewY - MinHoleEmptyY)/2;
				if (MaxHoleEmptyY > MaxHoleScrewY) MaxHoleScrewY += (MaxHoleEmptyY - MaxHoleScrewY)/2;
				float MaxDisX = MaxHoleScrewX - MinHoleScrewX;
				float CenterX = c::WINDOW_WIDTH/2;
				float MaxDisY = MaxHoleScrewY - MinHoleScrewY;
				VecAlign.x = MaxDisX/2;
				VecAlign.x = ((CenterX - MinHoleScrewX) > ( MaxHoleScrewX - CenterX)? ((CenterX - MinHoleScrewX) - VecAlign.x): (VecAlign.x - (MaxHoleScrewX - CenterX))) ;
				VecAlign.y = (ImageBoard.GetHeight() - MaxDisY)/2;
				VecAlign.y -= (MinHoleScrewY - PointBoardZero.y);
				VecAlign += OffsetAlign;
				for(int i = 0; i < pHoleList.size(); i++)
				{
					pHoleList[i]->Position += VecAlign;
					pHoleList[i]->SetPosition(pHoleList[i]->Position);
				}
			}

			// Item Data
			vector<string> WordList;
			CVec2 TempHoleList[15];
			CVec2 TempOffsetList[15];
			int HoleCount = 0;
			File.ReadLine(Line);
			e::SplitWord(Line, WordList);
			int Group = stoi(WordList[0]);
			int STT =  stoi(WordList[1]);
			int Layer = File.ReadInt();

			// Create Item
			CItemNutAndBolt *pItem = new CItemNutAndBolt();
			pItem->Load(int(pItemList.size()) + 1, Group, STT, Layer);
			File.ReadLine(Line);
			e::SplitWord(Line, WordList);
			if (WordList[0] == "p")
			{
				CVec2 Pos = CVec2(stof(WordList[1]), stoi(WordList[2]));
				Pos = NormalPosToBoardPos(Pos);
				Pos += VecAlign;
				pItem->SetPosition(Pos);
			}
			else
			{
				bool IsAlign = false;
				for(int i = 1; i < WordList.size(); i++) 
				{
					if (WordList[i] == "c")
					{
						IsAlign = true;
						break;
					}
					TempHoleList[HoleCount] = pHoleList[stoi(WordList[i])]->Position;
					HoleCount++;
				}

				if (WordList[0] == "a")
				{
					if (IsAlign)
					{
						CVec2 OffsetCenter = CVec2(stof(WordList[HoleCount + 2]), (WordList.size() > HoleCount + 3)? stof(WordList[HoleCount + 3]) : 0);
						pItem->SetPosition(pItem->CalculItemPosition(TempHoleList, HoleCount, OffsetCenter));
					}
					else pItem->SetPosition(pItem->CalculItemPositionOffset(TempHoleList, HoleCount));
				}
				else if (WordList[0] == "m")
				{
					File.ReadLine(Line);
					e::SplitWord(Line, WordList);
					string Axis = "x";
					if (WordList.size() == 4) Axis = WordList[3];
					TempOffsetList[0] = CVec2(stof(WordList[0]),stof(WordList[1]));
					pItem->SetPosition(pItem->CalculItemPositionOffset(TempHoleList, HoleCount, TempOffsetList, 1, stoi(WordList[2]), Axis));
				}
				else if (WordList[0] == "mm")
				{
					for(int i = 0; i < HoleCount; i++)
					{
						File.ReadLine(Line);
						e::SplitWord(Line, WordList);
						TempOffsetList[i] = CVec2(stof(WordList[0]),stof(WordList[1]));;
					}
					pItem->SetPosition(pItem->CalculItemPositionOffset(TempHoleList, HoleCount, TempOffsetList, HoleCount));
				}
			}
			FrameMap.AddChild(pItem);
			pItemList.push_back(pItem);
		}

		// Lock
		if (Line == "[lock]")
		{
			int Id = File.ReadInt();
			int HoleId = File.ReadInt();

			CLockNutAndBolt *pLock = new CLockNutAndBolt();
			pLock->Load(Id, HoleId);
			pLock->SetPosition(pHoleList[HoleId]->Position);
			FrameMap.AddChild(pLock);
			pLockList.push_back(pLock);
		}

		// Key
		if (Line == "[key]")
		{
			int Id = File.ReadInt();
			vector<string> WordList;
			File.ReadLine(Line);
			e::SplitWord(Line, WordList);
			CVec2 Pos = CVec2(stof(WordList[0]), stof(WordList[1]));
			Pos = NormalPosToBoardPos(Pos);
			Pos += VecAlign;

			CKeyNutAndBolt *pKey = new CKeyNutAndBolt();
			pKey->Load(Id);
			pKey->SetPosition(Pos);
			FrameMap.AddChild(pKey);
			pKeyList.push_back(pKey);
		}
	}

	// Screw
	for(int i = 0; i < ScrewIdList.size(); i++)
	{
		AddScrew(pHoleList[ScrewIdList[i]]);
	}

	// Refresh
	RefreshItemState();

	// Debug
	if (pItemList.size() >= 24) Gui.Log("Too Many Items!");
}

void CGameNutAndBolt::SaveLevel()
{
	Function.SetSetting("NutAndBoltLevel", e::Format("%i", Level));
}

void CGameNutAndBolt::ChangeGameState()
{
	// Data
	if (GameState == GS_BOOSTER) GameState = GS_NORMAL;
	else GameState = GS_BOOSTER;

	// Gui
	TextTime.SetVisible((GameState == GS_BOOSTER)? false : true);
	TextLevel.SetVisible((GameState == GS_BOOSTER)? false : true);
	TextBooster.SetVisible((GameState == GS_BOOSTER)? true : false);
	ImageClock.SetVisible((GameState == GS_BOOSTER)? false : true);
	ImageBgBooster.SetVisible((GameState == GS_BOOSTER)? true : false);
}

CVec2 CGameNutAndBolt::NormalPosToBoardPos(CVec2 NormalPos)
{
	CVec2 BoardPos = NormalPos;
	BoardPos.x = ImageBoard.GetWidth()/2 + (BoardPos.x - 580) * ITEM_SCALE * c::GUI_SCALE;
	BoardPos.y *= (c::GUI_SCALE * ITEM_SCALE);
	BoardPos += PointBoardZero;
	return BoardPos;
}

bool CGameNutAndBolt::AddScrew(CHoleNutAndBolt *pHole)
{
	// Item
	vector<CItemNutAndBolt*> ItemFitList;
	CScrewNutAndBolt *pScrew = new CScrewNutAndBolt(); 

	// Check Collide
	for(int i = 0; i < pItemList.size(); i++)
	{
		for(int j = 0; j < pItemList[i]->HoleList.size(); j++)
		{
			CVec2 Tmp2 = ((pItemList[i]->HoleList[j] - pItemList[i]->ItemPivot)*ITEM_SCALE*c::GUI_SCALE);
			float rt = pItemList[i]->pNode->getRotation();
			CVec2 Tmp = CVec2(pItemList[i]->GetX(), pItemList[i]->GetY()) + ((pItemList[i]->HoleList[j] - pItemList[i]->ItemPivot)*ITEM_SCALE*c::GUI_SCALE).Rotate(pItemList[i]->pNode->getRotation());
			if (i == 2)
			{
				int tt = pItemList[i]->pNode->getRotation();
				CVec2 a = CVec2(pItemList[i]->GetX(), pItemList[i]->GetY());
				int dd = 1;
			}
			CVec2 Dis = Tmp - pHole->Position;
			if (Dis.Length() < 10)
			{
				ItemFitList.push_back(pItemList[i]);
				pItemList[i]->pScrewList.push_back(pScrew);
			}
		}
	}

	// Mask Bit
	int MaskBit = SCREW_MASK_ORIGIN;
	if (ItemFitList.size() > 0)
	{
		MaskBit = ItemFitList[0]->Mask;
		for(int i = 1; i < ItemFitList.size(); i++)
		{
			MaskBit = MaskBit | ItemFitList[i]->Mask;
		}
		int t = SCREW_MASK_ORIGIN;
		MaskBit = MaskBit&t;
		MaskBit = MaskBit^t;
	}
	auto pPhysicsbody = PhysicsBody::createCircle(HOLE_RADIUS, PhysicsMaterial(SHAPE_DENSITY, SHAPE_RESTITUTION, SHAPE_FRICTION));
	pPhysicsbody->setDynamic(false);
	pPhysicsbody->setCategoryBitmask(MaskBit);
	pPhysicsbody->setCollisionBitmask(MaskBit);
	
	// Hole Id
	int HoleId = 0;
	for(int k = 0; k < pHoleList.size(); k++) 
		if (pHoleList[k] == pHole) 
		{
			HoleId = k; 
			break;
		}

	// Screw
	pScrew->Load(HoleId);
	pScrew->SetPosition(pHole->Position);
	pScrew->AddComponent(pPhysicsbody);
	pScrew->ButtonScrew.TouchEffect = false;
	pScrew->ButtonScrew.TouchBoundary = false;
	pScrew->ButtonScrew.HandleEventCallback(this);
	FrameMap.AddChild(pScrew);
	pScrewList.push_back(pScrew);

	// Hole
	pHole->pScrew = pScrew;

	// Done
	return true;
}

bool CGameNutAndBolt::MoveScrew(CVec2 Pos, CScrewNutAndBolt *pScrew)
{
	vector<CItemNutAndBolt*> ItemCollideList;
	CVec2 OldPos = CVec2(pScrew->GetX(), pScrew->GetY());
	CHoleNutAndBolt *pOldHole;
	for(int k = 0; k < pHoleList.size(); k++)
	{
		if (pHoleList[k]->pScrew == pScrew)
		{
			pOldHole = pHoleList[k];
			break;
		}
	}
	for(int k = 0; k < pHoleList.size(); k++)
	{
		CVec2 Dis1 = pHoleList[k]->Position - Pos;
		if (pHoleList[k]->CheckValid() && Dis1.Length() < ((HOLE_RADIUS + HOLE_TOUCH_OFFSET)*c::GUI_SCALE*ITEM_SCALE))
		{
			// Fail
			if ((pHoleList[k]->Position - OldPos).Length() < 10) return false;
			if (!CheckHoleCollide(pHoleList[k]->Position)) return false;
			for(int i = 0; i < pLockList.size(); i++)
			{
				if (pLockList[i]->HoleId == k) return false;
			}

			// Success
			pScrew->SetPosition(pHoleList[k]->Position);
			pSelectScrew->UnSelect();
			pOldHole->pScrew = 0;
			pHoleList[k]->pScrew = pScrew;

			// Old Item
			for(int i = 0; i < pItemList.size(); i++)
			{
				for(int m = 0; m < pItemList[i]->pScrewList.size(); m++)
				{
					if (pItemList[i]->pScrewList[m] == pScrew)
					{
						pItemList[i]->IsStateChange = true;
						pItemList[i]->pScrewList.erase(pItemList[i]->pScrewList.begin() + m);
						m--;
					}
				}
			}

			// New Item
			for(int i = 0; i < pItemList.size(); i++)
			{
				for(int j = 0; j < pItemList[i]->HoleList.size(); j++)
				{
					float Rotation = pItemList[i]->pNode->getRotation();
					CVec2 Tmp = CVec2(pItemList[i]->GetX(), pItemList[i]->GetY()) + ((pItemList[i]->HoleList[j] - pItemList[i]->ItemPivot)*ITEM_SCALE*c::GUI_SCALE).Rotate(Rotation);
					CVec2 Dis = Tmp - pHoleList[k]->Position;
					if (Dis.Length() < 10*c::GUI_SCALE)
					{
						pItemList[i]->IsStateChange = true;
						pItemList[i]->pScrewList.push_back(pScrew);
						ItemCollideList.push_back(pItemList[i]);
					}
				}
			}

			// Refresh
			RefreshItemState();

			// Mask Bit
			int MaskBit = SCREW_MASK_ORIGIN;
			if (ItemCollideList.size() > 0)
			{
				MaskBit = ItemCollideList[0]->Mask;
				for(int i = 1; i < ItemCollideList.size(); i++)
				{
					MaskBit = MaskBit | ItemCollideList[i]->Mask;
				}
				int t = SCREW_MASK_ORIGIN;
				MaskBit = MaskBit&t;
				MaskBit = MaskBit^t;
			}
			auto pPhysicsbody = pScrew->pNode->getPhysicsBody();
			pPhysicsbody->setCategoryBitmask(MaskBit);
			pPhysicsbody->setCollisionBitmask(MaskBit);
			return true;
		}
	}
	return false;
}

void CGameNutAndBolt::RefreshItemState()
{
	for(int i = 0; i < pItemList.size(); i++)
	{
		if (pItemList[i]->IsStateChange) pItemList[i]->RefreshState();
	}
}

void CGameNutAndBolt::ResetData()
{
	// Data
	GameState = GS_NORMAL;
	IsLose = IsGameWin = IsRevive  = false;
	Score  = ReviveCount = RewardType = 0;
	SelectTouchId = SelectHoleId = -1;
	SpeedX = SpeedY = 0;
	TimeEndLevel = ClockTime = 0;
	pSelectScrew = 0;

	// Joint
	auto pDirector = Director::getInstance();
	auto pScene = pDirector->getRunningScene();
	pScene->getPhysicsWorld()->removeAllJoints();

	// Screw
	for(int i = 0; i < pScrewList.size(); i++) delete pScrewList[i];
	pScrewList.clear();

	// Item
	for(int i = 0; i < pItemList.size(); i++) delete pItemList[i];
	pItemList.clear();

	// Key
	for(int i = 0; i < pItemList.size(); i++) delete pKeyList[i];
	pKeyList.clear();

	// Lock
	for(int i = 0; i < pLockList.size(); i++) delete pLockList[i];
	pLockList.clear();

	// Hole
	pHoleList.clear();
}

void CGameNutAndBolt::ResetAll()
{
	// Gui
	FrameMap.RemoveAllChild();
	TextTime.SetVisible(true);
	TextLevel.SetVisible(true);
	TextBooster.SetVisible(false);
	ImageClock.SetVisible(true);
	ImageBgBooster.SetVisible(false);
	WindowRevive.SetVisible(false);
	WindowWinLose.SetVisible(false);

	// Data
	ResetData();
}

void CGameNutAndBolt::RestartGame()
{
	// Gui
	WindowRevive.SetVisible(false);
	WindowWinLose.SetVisible(false);

	// Begin
	BeginLevel();
}

void CGameNutAndBolt::Revive() 
{
	// Data
	ClockTime = 90;
	IsLose = false;

	// Gui
	WindowRevive.SetVisible(false);
	WindowWinLose.SetVisible(false);
}

bool CGameNutAndBolt::OnQueryPoint(PhysicsWorld& pPhysicsWorld, PhysicsShape& pPhysicsShape, void* UserData)
{
	auto pPhysicsBodyShape = pPhysicsShape.getBody();
	for(int i = 0; i < pItemList.size(); i++)
	{
		auto pPhysicsBodyItem = pItemList[i]->pNode->getPhysicsBody();
		if ( pPhysicsBodyItem == pPhysicsBodyShape)
		{
			if (pItemList[i]->IsHole(HoleToCheck) < 0)
				IsScrewCollide = true;
		}
	}
	return true;
}

bool CGameNutAndBolt::CheckHoleCollide(CVec2 HolePos)
{
	// Check
	IsScrewCollide = false;
	HoleToCheck = HolePos;
	auto pDirector = Director::getInstance();
	auto pScene = pDirector->getRunningScene();
	auto pPhysicsWorld = pScene->getPhysicsWorld();
	auto func = CC_CALLBACK_3(CGameNutAndBolt::OnQueryPoint, this);
	for(int i = 0; i < 36; i++)
	{
		CVec2 PointTemp = HolePos - (CVec2(0, HOLE_RADIUS)*c::GUI_SCALE*ITEM_SCALE - CVec2(0,4)*c::GUI_SCALE*ITEM_SCALE).Rotate(i * 10);
		pPhysicsWorld->queryPoint(func, Point(PointTemp.x, c::WINDOW_HEIGHT - PointTemp.y), nullptr);
		if (IsScrewCollide) return false;
	}

	// Done
	return !IsScrewCollide;
}

void CGameNutAndBolt::OnKeyDown(EventKeyboard::KeyCode Key)
{
	if (Key == EventKeyboard::KeyCode::KEY_0)
	{
		Level = 1;
		SaveLevel();
		BeginLevel();
	}
	else if (Key == EventKeyboard::KeyCode::KEY_1 )
	{
		Level++;
		SaveLevel();
		BeginLevel();
	}
	else if (Key == EventKeyboard::KeyCode::KEY_2)
	{
		Level--;
		if (Level < 1) Level = 1;
		SaveLevel();
		BeginLevel();
	}
	else if (Key == EventKeyboard::KeyCode::KEY_3)
	{
		Level += 10;
		SaveLevel();
		BeginLevel();
	}
	else if (Key == EventKeyboard::KeyCode::KEY_4)
	{
		Level -= 10;
		if (Level < 1) Level = 1;
		SaveLevel();
		BeginLevel();
	}
}

void CGameNutAndBolt::OnClickButton(CButton *pButton)
{
	// Button Home
	if (pButton == &Gui.ButtonHome)
	{
#ifdef TEST_MODE
		Level--;
		if (Level < 1) Level = 1;
		SaveLevel();
		BeginLevel();
#else
		Release();
		ShowAdInterstitial();
#endif
	}

	// Button Setting
	if (pButton == &Gui.ButtonSetting)
	{
		Level++;
		SaveLevel();
		BeginLevel();
	}

	// Button Restart
	if (pButton == &ButtonRestart)
	{
		RestartGame();
		ShowAdInterstitial();
	}

	// Button Unscrew
	if (pButton == &ButtonUnscrew && GameState != GS_BOOSTER)
	{
		RewardType = REWARD_REMOVE_SCREW;
		PluginAd.Show(TAG_REWARDED);
		if (pSelectScrew)
		{
			pSelectScrew->UnSelect();
			pSelectScrew = 0;
		}
	}

	// Button Screw
	for(int i = 0; i < pScrewList.size(); i++)
	{
		if (pButton == &pScrewList[i]->ButtonScrew && pScrewList[i]->CheckValid())
		{
			// Normal
			if (GameState == GS_NORMAL)
			{
				if (pSelectScrew) pSelectScrew->UnSelect();
				if (pSelectScrew != pScrewList[i])
				{
					pSelectScrew = pScrewList[i];
					pSelectScrew->OnSelect();
					SoundManager.Play("game_nut_and_bolt/screw_out");
					SoundManager.Haptic();
				}
				else
				{
					pSelectScrew = 0;
				}
				break;
			}
			// Unscrew
			else if (GameState == GS_BOOSTER)
			{
				// State
				ChangeGameState();

				// Hole
				for(int j = 0; j < pHoleList.size(); j++)
					if (pHoleList[j]->pScrew == pScrewList[i])
					{
						pHoleList[j]->pScrew = 0;
						break;
					}

				// Item
				for(int m = 0; m < pItemList.size(); m++)
				{
					CItemNutAndBolt *pItem = pItemList[m];
					for(int n = 0; n < pItem->pScrewList.size(); n++)
					{
						if (pItem->pScrewList[n] == pScrewList[i])
						{
							pItem->IsStateChange = true;
							pItem->pScrewList.erase(pItem->pScrewList.begin() + n);
							break;
						}
					}
				}
				
				// Remove
				FrameMap.RemoveChild(pScrewList[i]);
				delete pScrewList[i];
				pScrewList.erase(pScrewList.begin() + i);

				// Refresh
				RefreshItemState();

				// Sound
				SoundManager.Haptic();
				SoundManager.Play("game_nut_and_bolt/screw_out");

				// Done
				break;
			}
		}
	}
}

bool CGameNutAndBolt::OnTouchBegin(int TouchId, float TouchX, float TouchY)
{
	// Super
	if (CFrame::OnTouchBegin(TouchId, TouchX, TouchY)) return true;

	// Data
	SelectTouchId = TouchId;

	// Check Hole
	for(int k = 0; k < pHoleList.size(); k++)
	{
		CVec2 Dis1 = pHoleList[k]->Position - CVec2(TouchX, TouchY);
		if (pHoleList[k]->Type == HOLE_AD && Dis1.Length() < (HOLE_RADIUS + HOLE_TOUCH_OFFSET)*c::GUI_SCALE*ITEM_SCALE)
		{
			SelectHoleId = k;
		}
	}

	// Done
	return false;
}

void CGameNutAndBolt::OnTouchEnd(int TouchId, float TouchX, float TouchY)
{
	// Frame
	CFrame::OnTouchEnd(TouchId, TouchX, TouchY);

	// Select
	if (SelectTouchId == TouchId)
	{
		// Unlock Hole
		for(int k = 0; k < pHoleList.size(); k++)
		{
			CVec2 Dis1 = pHoleList[k]->Position - CVec2(TouchX, TouchY);
			if (pHoleList[k]->Type == HOLE_AD && Dis1.Length() < (HOLE_RADIUS + HOLE_TOUCH_OFFSET)*c::GUI_SCALE*ITEM_SCALE && k == SelectHoleId)
			{
				SelectHoleId = -1;
				SelectTouchId = -1;
				pHoleList[k]->RequestHole();
				return;
			}
		}

		// Move Screw
		if (pSelectScrew)
		{
			if (MoveScrew(CVec2(TouchX, TouchY), pSelectScrew)) 
			{
				SoundManager.Play("game_nut_and_bolt/screw_in");
				SoundManager.Haptic();
			}
			else
			{
				pSelectScrew->UnSelect();
			}
			pSelectScrew = 0;
		}

		// Reset
		SelectTouchId = -1;
	}
}

void CGameNutAndBolt::GameLose()
{
	// Gui
	WindowRevive.SetVisible(false);
	WindowWinLose.Show(false);

	// Sound
	SoundManager.Play("game_nut_and_bolt/game_over");
}

bool CGameNutAndBolt::OnContactBegin(PhysicsContact& Contact)
{
	// Key
	for(int i = 0; i < pKeyList.size(); i++)
	{
		auto pNode = pKeyList[i]->pNode;
		if (!pKeyList[i]->IsActivate && Contact.getShapeA()->getBody()->getOwner() == pNode || Contact.getShapeB()->getBody()->getOwner() == pNode)
		{
			pKeyList[i]->GoUnlock();
			SoundManager.Play("game_nut_and_bolt/key_get");
		}
	}

	// Done
	return true;
}

void CGameNutAndBolt::OnUpdate(float DeltaTime)
{
	// Play Time
	if (!PluginAd.IsShow && !WindowWinLose.GetVisible() && !WindowRevive.GetVisible()) PluginAd.PlayTime += DeltaTime;

	// Clock Time
	if (ClockTime > 0 && !PluginAd.IsShow) 
	{
		// Gui
		int Min = ClockTime/60;
		int Sec = ClockTime - Min*60;
		TextTime.SetText(e::Format("%i:%s%i", Min, (Sec < 10)? "0":"", Sec));

		// Clock
		if (pItemList.size() > 0) ClockTime -= DeltaTime;
		if (ClockTime <= 0)
		{
			ClockTime = 0;
			IsLose = true;
			TimeEndLevel = e::GetTime();
		}	
	}

	// Window Revive
	if (WindowRevive.GetVisible()) WindowRevive.OnUpdate(DeltaTime);

	// Game End
	if (!IsLose)
	{
		// Item
		for(int i = 0; i < pItemList.size(); i++)
		{
			if (pItemList[i]->GetY() > c::WINDOW_HEIGHT + 200*c::GUI_SCALE)
			{
				FrameMap.RemoveChild(&pItemList[i]->ImageItem);
				delete pItemList[i];
				pItemList.erase(pItemList.begin() + i);
				i--;
			}
		}

		// Key
		for (int i=0; i<pKeyList.size(); i++)
		{
			if (pKeyList[i]->IsUse)
			{
				pKeyList[i]->OnUpdate(DeltaTime);
			}
			else
			{
				FrameMap.RemoveChild(pKeyList[i]);
				delete pKeyList[i];
				pKeyList.erase(pKeyList.begin() + i);
				i--;
			}
		}

		// Lock Hole
		for (int i=0; i<pLockList.size(); i++)
		{
			if (!pLockList[i]->IsUse)
			{
				FrameMap.RemoveChild(pLockList[i]);
				delete pLockList[i];
				pLockList.erase(pLockList.begin() + i);
				i--;
			}
		}

		// Win
		if (TimeEndLevel == 0 && !IsGameWin && pItemList.empty())
		{
			// Data
			IsGameWin = true;
			TimeEndLevel = e::GetTime();

			// Sound
			SoundManager.Play("game_nut_and_bolt/game_win");

			// Effect
			Game.EffectManager.AddEffect(c::WINDOW_WIDTH/2, c::WINDOW_HEIGHT * 0.8, (new CEffectNutAndBolt())->CreateFirework());
		}
	}

	// End Level
	if (TimeEndLevel > 0)
	{
		// Lose
		if (IsLose && e::GetTime() - TimeEndLevel > 500)
		{
			TimeEndLevel = 0;
			WindowRevive.Show();
		}
		// Win
		else if (IsGameWin && e::GetTime() - TimeEndLevel > 1300)
		{ 
			// Data
			TimeEndLevel = 0;

			// Window
			WindowWinLose.Show(true);

			// Show Ad
			ShowAdInterstitial();

			// Level
			Level++;
			SaveLevel();
		}
	}
}
