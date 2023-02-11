#include "Engine.h"

bool Engine::Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height)
{
	//timer.Start();

	if (!this->render_window.Initialize(this, hInstance, window_title, window_class, width, height))
		return false;

	if (!gfx.Initialize(this->render_window.GetHWND(), width, height))
		return false;

	HKL defaultLanguageLayout = LoadKeyboardLayout(L"0x04090409", KLF_SUBSTITUTE_OK);
	DWORD dwThreadID = GetCurrentThreadId();
	HKL hCurKeyboard = GetKeyboardLayout(dwThreadID);
	if (hCurKeyboard != defaultLanguageLayout)
	{
		UINT i;
		HKL hklCurrent;
		UINT uLayouts;
		HKL * lpList;

		uLayouts = GetKeyboardLayoutList(0, NULL);
		lpList = (HKL*)malloc(uLayouts * sizeof(HKL));
		uLayouts = GetKeyboardLayoutList(uLayouts, lpList);

		for (i = 0; i < uLayouts; i++)
		{
			hklCurrent = *(lpList + i);
			if (hklCurrent == defaultLanguageLayout)
			{
				ActivateKeyboardLayout(hklCurrent, 0);
			}
		}
	}

	return true;
}

bool Engine::ProcessMessages()
{
	return this->render_window.ProcessMessages();
}

void Engine::Update(float dt)
{

	while (!keyboard.CharBufferIsEmpty())
	{
		unsigned char ch = keyboard.ReadChar();
	}

	while (!keyboard.KeyBufferIsEmpty())
	{
		KeyboardEvent kbe = keyboard.ReadKey();
		unsigned char keycode = kbe.GetKeyCode();
	}

	while (!mouse.EventBufferIsEmpty())
	{
		MouseEvent me = mouse.ReadEvent();
		if (mouse.IsRightDown())
		{
			if (me.GetType() == MouseEvent::EventType::RAW_MOVE)
			{
				this->gfx.camera.AdjustRotation((float)me.GetPosY() * 0.01f, (float)me.GetPosX() * 0.01f, 0);
			}
		}
	}

	const float cameraSpeed = 0.6f;

	if (keyboard.KeyIsPressed('W'))
	{
		this->gfx.camera.AdjustPosition(this->gfx.camera.GetForwardVector() * cameraSpeed * dt);
		//this->gfx.model->AdjustPosition(this->gfx.model->GetForwardVector()* cameraSpeed * dt);
	}
	if (keyboard.KeyIsPressed('S'))
	{
		this->gfx.camera.AdjustPosition(this->gfx.camera.GetBackwardVector() * cameraSpeed * dt);
		//this->gfx.model->AdjustPosition(this->gfx.model->GetBackwardVector()* cameraSpeed * dt);
	}
	if (keyboard.KeyIsPressed('A'))
	{
		this->gfx.camera.AdjustPosition(this->gfx.camera.GetLeftVector() * cameraSpeed * dt);
		//this->gfx.model->AdjustPosition(this->gfx.model->GetLeftVector()* cameraSpeed * dt);
	}
	if (keyboard.KeyIsPressed('D'))
	{
		this->gfx.camera.AdjustPosition(this->gfx.camera.GetRightVector() * cameraSpeed * dt);
		//this->gfx.model->AdjustPosition(this->gfx.model->GetRightVector()* cameraSpeed * dt);
	}
	if (keyboard.KeyIsPressed(VK_SPACE))
	{
		this->gfx.camera.AdjustPosition(0.0f, cameraSpeed * dt, 0.0f);
	}
	if (keyboard.KeyIsPressed('Z'))
	{
		this->gfx.camera.AdjustPosition(0.0f, -cameraSpeed * dt, 0.0f);
	}
	//gfx.model->GetFBXModel()->m_kcurrentAnimaionIndex = 0;
	gfx.model->Update(dt);

	//collision
	BoundingBox cp1 = gfx.model->collider->obb;  
	BoundingBox cp2 = gfx.quad->collider->obb;
	auto sclMatrix = XMMatrixScalingFromVector(gfx.model->sclVector);
	auto matrix = gfx.model->transfomrMatirx * sclMatrix * gfx.model->worldMatrix;
	cp1.Transform(cp1, matrix);

	sclMatrix = XMMatrixScalingFromVector(gfx.quad->sclVector);
	matrix = gfx.quad->transfomrMatirx * sclMatrix * gfx.quad->worldMatrix;
	cp2.Transform(cp2, matrix);

	gfx.inte = cp1.Intersects(cp2);

	auto quadpos = gfx.quad->GetPositionFloat3();
	if (quadpos.x > 100.0f)
		moveswitch = false;

	if (quadpos.x <= -100.0f)
		moveswitch = true;

	gfx.quad->AdjustRotation(XMFLOAT3(0.01f, 0.01f, 0.01f));
	if (moveswitch == true)
		gfx.quad->SetPosition(XMFLOAT3(quadpos.x + 1, quadpos.y, quadpos.z));
	else 
		gfx.quad->SetPosition(XMFLOAT3(quadpos.x - 1, quadpos.y, quadpos.z));

}

void Engine::RenderFrame(float dt)
{
	this->gfx.RenderFrame(dt);
}

