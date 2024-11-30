#pragma once

#include <D2DWorld.h>

class InvaderWorld : public D2DWorld
{
protected:
	const COLORREF m_textColour = RGB(0, 255, 0);
	const FLOAT m_textHeight = 18.0f;

	// dimensions
	const int m_screenWidth = 1000;
	const int m_screenHeight = 1080;
	const FLOAT m_scoreY = m_textHeight * 3;
	const FLOAT m_shipY = 50.0f;

	const FLOAT m_bulletWidth = 5.0f;
	const FLOAT m_bulletHeight = 30.0f;
	const COLORREF m_bulletColour = RGB(255, 255, 255);

	const FLOAT m_playerWidth = 80.0f;
	const FLOAT m_playerHeight = 30.0f;
	const Point2F m_playerStart = Point2F(0, 1000.0f);
	const FLOAT m_playerSpeed = 10.0f;
	const FLOAT m_playerbulletSpeed = 20.0f;
	const int m_playerResetTime = 5000;
	const COLORREF m_playerColour = RGB(255, 255, 255);

	const int m_playerLives = 3;
	const Point2F m_playerLivesStart = Point2F(20.0F, 1050.0f);
	const FLOAT m_playerIndicatorSize = 0.6F;

	const FLOAT m_barrierWidth = 100.0f;
	const FLOAT m_barrierHeight = 50.0f;
	const int m_barrierCount = 4;
	const FLOAT m_barrierY = 900.0f;
	const COLORREF m_barrierColour = RGB(0, 255, 0);
	const int m_barrierDividerX = 8;
	const int m_barrierDividerY = 6;

	const FLOAT m_invaderWidth = 60.0f;
	const FLOAT m_invaderHeight = 40.0f;
	const FLOAT m_invaderBorder = 10.0f;
	const FLOAT m_invaderbulletSpeed = 15.0f;
	const int m_invaderBulletChanceStart = 60;
	const int m_invaderCols = 10;
	const int m_invaderRows = 5;
	const int m_invaderMoveDelayStart = 1000;	// in ms
	const FLOAT m_invaderSpeed = 25.0f;		// per move delay
	const COLORREF m_invaderColour = RGB(0, 255, 0);
	const int score_invader_hit = 30;

	const FLOAT m_shipWidth = 70.0f;
	const FLOAT m_shipHeight = 20.0f;
	const FLOAT m_shipSpeed = 4.0f;
	const int m_shipSpawnTime = 10000;
	const COLORREF m_shipColour = RGB(255, 0, 255);
	const int score_ship_hit = 100;

	const COLORREF m_gameOverColour = RGB(255, 255, 255);

public:
	InvaderWorld(Notifier& notifier) :
		m_notifier(notifier),
		// shapes
		m_player(m_playerStart, m_playerWidth, m_playerHeight, 0, 0, m_playerColour),
		m_playerBullet(Point2F(0, 0), m_bulletWidth, m_bulletHeight, 0, 0, m_bulletColour),
		m_ship(NULL),

		// tickers
		m_tdInvaderMove(m_invaderMoveDelayStart),
		m_tdPlayerReset(m_playerResetTime, false),
		m_tdShip(m_shipSpawnTime),
		m_tdShipScore(2000),

		// Texts
		m_textScore(L"", Point2F(0.0f, m_textHeight * 1.5f), 200.0f, m_textHeight, 0.0f, 0, m_textColour, DWRITE_TEXT_ALIGNMENT_CENTER),
		m_textScoreLabel(L"Score", Point2F(0.0f, m_textHeight * 0.5f), 200.0f, m_textHeight, 0.0f, 0, m_textColour, DWRITE_TEXT_ALIGNMENT_CENTER),
		m_textShipScore(L"100", Point2F(0.0f, 0.0f), m_shipWidth, m_textHeight, 0.0f, 0, m_shipColour, DWRITE_TEXT_ALIGNMENT_CENTER),
		m_textGameOver(L"Game Over", Point2F(0.0f, m_screenHeight / 2.0F), (FLOAT)m_screenWidth, (FLOAT)m_screenHeight / 10.0F, 0.0f, 0, m_gameOverColour, DWRITE_TEXT_ALIGNMENT_CENTER)
	{
	}

	~InvaderWorld() {
	}

	w32Size D2DGetScreenSize() override {
		return w32Size(m_screenWidth, m_screenHeight);
	}

	bool D2DCreateResources(IDWriteFactory* pDWriteFactory, ID2D1HwndRenderTarget* pRenderTarget, IWICImagingFactory* pIWICFactory, D2DRectScaler* pRS) override {
		m_bitmap1.LoadFromFile(pRenderTarget, pIWICFactory, L"octopusClosed.png", (UINT)m_invaderWidth, (UINT)m_invaderHeight);
		m_bitmap2.LoadFromFile(pRenderTarget, pIWICFactory, L"octopusOpen.png", (UINT)m_invaderWidth, (UINT)m_invaderHeight);
		m_bitmap3.LoadFromFile(pRenderTarget, pIWICFactory, L"crabClosed.png", (UINT)m_invaderWidth, (UINT)m_invaderHeight);
		m_bitmap4.LoadFromFile(pRenderTarget, pIWICFactory, L"crabOpen.png", (UINT)m_invaderWidth, (UINT)m_invaderHeight);
		m_bitmap5.LoadFromFile(pRenderTarget, pIWICFactory, L"squidClosed.png", (UINT)m_invaderWidth, (UINT)m_invaderHeight);
		m_bitmap6.LoadFromFile(pRenderTarget, pIWICFactory, L"squidOpen.png", (UINT)m_invaderWidth, (UINT)m_invaderHeight);
		m_bitmapUFO.LoadFromFile(pRenderTarget, pIWICFactory, L"UFO.png", (UINT)m_invaderWidth, (UINT)m_invaderHeight);
		return true;
	}

	bool Init() {
		// Initialise Game Variables
		m_score = 0;
		m_livesRemaining = m_playerLives;
		m_player.SetPos(m_playerStart);
		m_frame = false;
		m_invaderBulletChance = m_invaderBulletChanceStart;

		// Create Game Objects
		m_ship = new MovingBitmap(&m_bitmapUFO, Point2F(0, 0), m_shipWidth, m_shipHeight, m_shipSpeed, 90, m_shipColour);

		// Create the invaders
		for (FLOAT x = 0; x < m_invaderCols; x++) {
			for (int y = 0; y < m_invaderRows; y++) {
				int invaderType = 0;
				d2dBitmap* p = &m_bitmap5;
				if ((y == 1) || (y == 2)) {
					p = &m_bitmap3;
					invaderType = 1;
				}
				else if ((y == 3) || (y == 4)) {
					p = &m_bitmap1;
					invaderType = 2;
				}

				MovingBitmap* pInvader = new MovingBitmap(p,
					Point2F(m_invaderBorder + x * (m_invaderWidth + m_invaderBorder),
						m_invaderBorder + y * (m_invaderHeight + m_invaderBorder) + m_scoreY + m_shipY),
					m_invaderWidth, m_invaderHeight,
					m_invaderSpeed, 90,
					m_invaderColour
				);
				pInvader->SetUserData(invaderType);
				m_invaders.push_back(pInvader);
				QueueShape(pInvader);
			}
		}

		// Create the barriers as grids of destructable rectangles
		FLOAT step = (FLOAT)m_screenWidth / (FLOAT)m_barrierCount;
		for (int i = 0; i < m_barrierCount; i++) {
			FLOAT divBarrierWidth = m_barrierWidth / m_barrierDividerX;
			FLOAT divBarrierHeight = m_barrierHeight / m_barrierDividerY;
			FLOAT startX = step / 2.0f + step * i - m_barrierWidth / 2;
			FLOAT startY = m_barrierY;

			for (int x = 0; x < m_barrierDividerX; x++) {
				for (int y = 0; y < m_barrierDividerY; y++) {
					auto barrier = new MovingRectangle(
						Point2F(startX + x * divBarrierWidth, startY + y * divBarrierHeight),
						divBarrierWidth + 1.0F, divBarrierHeight + 1.0F, 0.0F, 0, m_barrierColour);

					m_barriers.push_back(barrier);
					QueueShape(barrier);
				}
			}
		}

		// Create the player life indicators
		Point2F playerLifePt = m_playerLivesStart;
		for (int i = 0; i < m_playerLives - 1; i++) {
			FLOAT piHeight = m_playerHeight * m_playerIndicatorSize;
			FLOAT piWidth = m_playerWidth * m_playerIndicatorSize;
			auto life = new MovingRectangle(playerLifePt, piWidth, piHeight, 0, 0, m_playerColour);
			auto bullet = new MovingRectangle(Point2F(0, 0), m_bulletWidth * m_playerIndicatorSize, m_bulletHeight * m_playerIndicatorSize, 0, 0, m_bulletColour);

			auto pos = life->GetPos();
			pos += Point2F((piWidth - m_bulletWidth) / 2.0f, -piHeight / 2.0f);
			bullet->SetPos(pos);

			playerLifePt.x += piWidth + 10.0F;
			QueueShape(life);
			m_playerLifeIndicators.push_back(life);
			QueueShape(bullet);
			m_playerLifeIndicators.push_back(bullet);
		}

		QueueShape(&m_player);
		QueueShape(&m_playerBullet);
		QueueShape(m_ship, false);
		QueueShape(&m_textScore);
		QueueShape(&m_textScoreLabel);
		QueueShape(&m_textShipScore, false);
		QueueShape(&m_textGameOver, false);

		UpdateIndicators();

		AddScore(0);

		return true;
	}

	void DeInit() {
		delete m_ship;
		m_ship = NULL;

		for (auto p : m_invaders)
			delete p;
		m_invaders.clear();

		for (auto p : m_invaderBullets)
			delete p;
		m_invaderBullets.clear();

		for (auto p : m_barriers)
			delete p;
		m_barriers.clear();

		for (auto p : m_playerLifeIndicators) {
			delete p;
		}
		m_playerLifeIndicators.clear();
	}

	bool D2DUpdate(ULONGLONG tick, const Point2F& mouse, std::queue<WindowEvent>& events) override {
		UpdateCheckPlayerKeys();

		UpdateMoveInvaders(tick);

		UpdateMovePlayer();

		UpdateMovePlayerBullet();

		UpdateMoveInvaderBullets();

		// Game over? Don't do anything else.
		if (m_livesRemaining == 0) {
			return true;
		}

		UpdateFireInvaderBullets();

		if (m_ship) {
			UpdateShip(tick);
		}

		// Does the player need to respawn?
		if (m_tdPlayerReset.Elapsed(tick)) {	// respawn?
			UpdateIndicators();
			m_player.SetPos(m_playerStart);
			m_player.SetActive(true);
			m_playerBullet.SetActive(true);
			m_tdPlayerReset.SetActive(false);
		}

		while (!events.empty()) {
			auto& ev = events.front();
			if (ev.m_msg == WM_KEYDOWN) {
				if (ev.m_wParam == VK_ESCAPE) {
					// quit back to menu
					m_notifier.Notify(m_amQuit);
				}
			}
			events.pop();
		}

		return true;
	}

protected:
	void UpdateCheckPlayerKeys() {
		// Left/right?
		if (KeyDown(VK_LEFT)) {
			m_player.SetDirectionInDeg(-90);
			m_player.SetSpeed(m_playerSpeed);
		}
		else if (KeyDown(VK_RIGHT)) {
			m_player.SetDirectionInDeg(90);
			m_player.SetSpeed(m_playerSpeed);
		}
		else {
			m_player.SetSpeed(0.0f);
		}

		// Fire?
		if (KeyPressed(VK_LCONTROL)) {
			if (!m_playerBullet.GetUserData()) {
				// Create a player bullet
				m_playerBullet.SetDirectionInDeg(0);
				m_playerBullet.SetSpeed(m_playerbulletSpeed);
				m_playerBullet.SetUserData(1);
			}
		}
	}

	void UpdateIndicators() {
		int lastIndicator = (m_livesRemaining - 1) * 2;
		for (int i = 0; i < m_playerLifeIndicators.size(); i++) {
			if (i < lastIndicator) {
				m_playerLifeIndicators[i]->SetActive(true);
			}
			else {
				m_playerLifeIndicators[i]->SetActive(false);
			}
		}
	}

	void UpdateMoveInvaders(ULONGLONG tick) {
		bool hitEnd = false;
		if (m_tdInvaderMove.Elapsed(tick)) {
			// Time for an invader move
			// Replace the bitmap
			for (auto* p : m_invaders) {
				if (p->GetUserData() == 0) {
					p->SetBitmap(m_frame ? &m_bitmap5 : &m_bitmap6);
				}
				else if (p->GetUserData() == 1) {
					p->SetBitmap(m_frame ? &m_bitmap3 : &m_bitmap4);
				}
				else {
					p->SetBitmap(m_frame ? &m_bitmap1 : &m_bitmap2);
				}
			}
			m_frame = !m_frame;

			// Check if they move, will any of them hit the end.
			for (auto* p : m_invaders) {
				if (p->IsActive()) {
					if (p->WillHitBounds(D2DGetScreenSize()) != Position::moveResult::ok) {
						hitEnd = true;
					}
				}
			}

			if (hitEnd) {
				// Move invaders down and send them back the other way
				for (auto inv : m_invaders) {
					if (inv->IsActive()) {
						inv->BounceX();
						inv->OffsetPos(Point2F(0.0f, 60.0f));
					}
				}
			}
			else {
				for (auto inv : m_invaders) {
					inv->Move();
				}
			}
		}
	}

	void UpdateMovePlayer() {
		// Move the player
		if (m_player.WillHitBounds(D2DGetScreenSize()) == Position::moveResult::ok) {
			m_player.Move();
		}
		else {
			m_player.BounceX();
		}
	}

	void UpdateMovePlayerBullet() {
		// Move the player bullet
		if (m_playerBullet.IsActive()) {
			if (m_playerBullet.WillHitBounds(D2DGetScreenSize()) != Position::moveResult::ok) {
				m_playerBullet.SetUserData(0);
			}
			else {
				m_playerBullet.Move();

				// Did we hit a barrier
				for (auto pBarrier : m_barriers) {
					if (pBarrier->IsActive()) {
						if (m_playerBullet.HitTestShape(*pBarrier)) {
							pBarrier->SetActive(false);
							m_playerBullet.SetUserData(0);	// Reset the player bullet
						}
					}
				}

				// Did we hit an invader?
				for (auto& inv : m_invaders) {
					if (inv->IsActive() && inv->HitTestShape(m_playerBullet)) {
						inv->SetActive(false);	// disable the invader
						m_playerBullet.SetUserData(0);
						AddScore(score_invader_hit);
						m_tdInvaderMove.AddTicks(-15);						// speed up the invaders a little
						m_invaderBulletChance -= 1; // increase bullet chance
						break;
					}
				}

				// Did we hit a ship at the top?
				if (m_ship && m_ship->IsActive() && m_ship->HitTestShape(m_playerBullet)) {
					m_ship->SetActive(false);
					m_playerBullet.SetUserData(0);

					// Display the score and set off a timer
					m_textShipScore.SetPos(m_ship->GetPos());
					m_textShipScore.SetActive(true);
					m_tdShipScore.SetActive(true);

					AddScore(score_ship_hit);
				}
			}
		}

		// Show the bullet on top of the player if it's not in flight
		if (!m_playerBullet.GetUserData()) {
			auto pos = m_player.GetPos();
			pos += Point2F((m_playerWidth - m_bulletWidth) / 2.0f, -m_playerHeight / 2.0f);
			m_playerBullet.SetPos(pos);
		}
	}

	void UpdateMoveInvaderBullets() {
		// Move (and destroy) invader bullets
		for (auto it = m_invaderBullets.begin(); it != m_invaderBullets.end();) {
			if ((*it)->IsActive()) {
				if ((*it)->WillHitBounds(D2DGetScreenSize()) != Position::moveResult::ok) {
					// we're out of bounds
					RemoveShape(*it);
					it = m_invaderBullets.erase(it);	// remove the bullet
					continue;
				}
				(*it)->Move();

				// did we hit a barrier?
				bool hitBarrier = false;
				for (auto pBarrier : m_barriers) {
					if (pBarrier->IsActive()) {
						if ((*it)->HitTestShape(*pBarrier)) {
							hitBarrier = true;
							pBarrier->SetActive(false);
						}
					}
				}

				// If so, destroy the bullet
				if (hitBarrier) {
					RemoveShape(*it);
					it = m_invaderBullets.erase(it);	// remove the bullet
					continue;
				}

				// did we hit the player?
				if (m_player.IsActive() && (*it)->HitTestShape(m_player)) {
					m_livesRemaining--;
					if (m_livesRemaining == 0) {
						// Go back to start screen/menu	-->>> GAME OVER
						m_textGameOver.SetActive(true);
						m_tdPlayerReset.SetActive(false);
						m_tdInvaderMove.SetActive(false);
					}

					m_tdPlayerReset.SetActive(true);
					m_player.SetActive(false);
					m_playerBullet.SetActive(false);
					RemoveShape(*it);
					it = m_invaderBullets.erase(it);	// remove the bullet
					continue;
				}
			}
			++it;
		}
	}

	void UpdateFireInvaderBullets() {
		// For each active invader, fire a bullet randomly
		// Count the remaining invaders
		int invaderCount = 0;
		for (auto p : m_invaders) {
			if (p->IsActive())
				invaderCount++;
		}

		for (auto p : m_invaders) {
			if (p->IsActive()) {
				if (!w32rand(m_invaderBulletChance * invaderCount)) {
					Point2F ptBullet = p->GetPos();
					ptBullet.x += (m_invaderWidth - m_bulletWidth) / 2;
					MovingRectangle* bullet = new MovingRectangle(
						ptBullet, m_bulletWidth, m_bulletHeight, m_invaderbulletSpeed, 180, m_bulletColour
					);

					m_invaderBullets.push_back(bullet);
					QueueShape(bullet);
				}
			}
		}
	}

	void UpdateShip(ULONGLONG tick) {
		if (m_ship->IsActive()) {
			// Move the ship along the top
			if (m_ship->WillHitBounds(D2DGetScreenSize()) == Position::moveResult::ok) {
				m_ship->Move();
			}
			else {
				m_ship->SetActive(false);
			}
		}

		if (m_tdShipScore.Elapsed(tick)) {
			m_textShipScore.SetActive(false);
		}

		// Spawn a ship along the top?
		if (m_tdShip.Elapsed(tick)) {
			m_ship->SetPos(Point2F(0.0f, m_scoreY + (m_shipY - m_shipHeight) / 2.0f));
			m_ship->SetActive(true);
		}
	}

	void AddScore(int n) {
		m_score += n;
		wchar_t buff[32];
		_snwprintf_s(buff, 32, L"%06d", m_score);
		m_textScore.SetText(buff);
	}

protected:
	MovingRectangle m_player;
	MovingRectangle m_playerBullet;
	std::vector<Shape*> m_playerLifeIndicators;
	std::vector<MovingBitmap*> m_invaders;
	std::vector<Shape*> m_invaderBullets;
	MovingBitmap* m_ship;
	MovingText m_textScore;
	MovingText m_textScoreLabel;
	std::vector<Shape*> m_barriers;

	TickDelta m_tdInvaderMove;
	TickDelta m_tdPlayerReset;
	TickDelta m_tdShip;
	TickDelta m_tdShipScore;
	MovingText m_textShipScore;
	MovingText m_textGameOver;

	d2dBitmap m_bitmap1;
	d2dBitmap m_bitmap2;
	d2dBitmap m_bitmap3;
	d2dBitmap m_bitmap4;
	d2dBitmap m_bitmap5;
	d2dBitmap m_bitmap6;
	d2dBitmap m_bitmapUFO;

	int m_invaderBulletChance;
	int m_score;
	int m_livesRemaining;

	bool m_frame;

	Notifier& m_notifier;
public:
	AppMessage m_amQuit;
};