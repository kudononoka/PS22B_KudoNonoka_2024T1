# include <Siv3D.hpp>
/*
	よりC++ライクな書き方
	・クラスベース
	・継承を行う
*/

//==============================
// 前方宣言
//==============================
class Ball;
class Bricks;
class Paddle;

/// @brief シーンの管理
//数字を代入できないようにした
//typedef enumは int型も代入できちゃう
enum class SceneState
{
	/// @brief タイトル
	TITLE,
	/// @brief インゲーム
	IN_GAME,
};

//==============================
// 定数
//==============================
namespace constants {
	namespace brick {
		/// @brief ブロックのサイズ
		constexpr Size SIZE{ 40, 20 };

		/// @brief ブロックの数　縦
		constexpr int Y_COUNT = 3;

		/// @brief ブロックの数　横
		constexpr int X_COUNT = 20;

		/// @brief 合計ブロック数
		constexpr int MAX = Y_COUNT * X_COUNT;
	}

	namespace ball {
		/// @brief ボールの速さ
		constexpr double SPEED = 480.0;
	}

	namespace paddle {
		/// @brief パドルのサイズ
		constexpr Size SIZE{ 280, 10 };
	}

	namespace reflect {
		/// @brief 縦方向ベクトル
		constexpr Vec2 VERTICAL{ 1, -1 };

		/// @brief 横方向ベクトル
		constexpr Vec2 HORIZONTAL{ -1,  1 };
	}
}



//==============================
// クラス宣言
//==============================
/// @brief ボール
class Ball final {
private:
	/// @brief 速度
	Vec2 velocity;

	/// @brief ボール
	Circle ball;

public:
	/// @brief コンストラクタ
	Ball() : velocity({ 0, -constants::ball::SPEED }), ball({ 400, 400, 8 }) {}

	/// @brief デストラクタ
	~Ball() {}

	/// @brief 更新
	void Update() {
		ball.moveBy(velocity * Scene::DeltaTime());
	}

	/// @brief 描画
	void Draw() const {
		ball.draw();
	}

	Circle GetCircle() const {

		return ball;
	}

	Vec2 GetVelocity() const {
		return velocity;
	}

	/// @brief 新しい移動速度を設定
	/// @param newVelocity 新しい移動速度
	void SetVelocity(Vec2 newVelocity) {
		using namespace constants::ball;
		velocity = newVelocity.setLength(SPEED);
	}

	/// @brief 反射
	/// @param reflectVec 反射ベクトル方向 
	void Reflect(const Vec2 reflectVec) {
		velocity *= reflectVec;
	}
};

class BallSpawner {
private:
	
	/// @brief 複数のボールのアドレスを確保する配列
	Ball* balls[3];
	/// @brief ボール増減最大数
	int ballMaxNum = 3;
	/// @brief 現在のボール数
	int currentBallCount = 0;
	/// @brief ボールを増やす時に必要な現在のブロックの数の配列
	int bricksForExtraBall[3] = {constants::brick::MAX, constants::brick::MAX / 2, constants::brick::MAX / 4};
	/// @brief ボールを増やす時に必要な現在のブロックの数の配列のIndex
	int bricksForExtraBallIndex = 0;
	/// @brief ボールのカウントダウン時の最大値
	float BallSpawnCountMax = 3;
	/// @brief 現在のボールのカウントダウン値
	float BallSpawnCount = BallSpawnCountMax;
	/// @brief ボールの生成準備を始めているかどうか
	bool IsBallSpawnPrepare = false;

public:
	/// @brief ボール生成
	void Generate() {
		if (currentBallCount == ballMaxNum) return;
		balls[currentBallCount] = new Ball();
		currentBallCount++;
		bricksForExtraBallIndex++;
	}

	/// @brief ボール生成カウントダウン
	void BallSpawnCountDown(float DeltaTime)
	{
		BallSpawnCount -= DeltaTime;

		if (BallSpawnCount <= 0)
		{
			Generate();
			BallSpawnCount = BallSpawnCountMax;
			IsBallSpawnPrepare = false;
		}
	}

	/// @brief ボール削除
	void Delete() {
		for (int i = 0; i < currentBallCount; i++) {
			if (balls[i] != nullptr)
			{
				delete balls[i];
				balls[i] = nullptr;
			}
		}
		currentBallCount = 0;
		bricksForExtraBallIndex = 0;
	}

	/// @brief ボール参照
	Ball* GetBalls(int index) {
		return balls[index];
	}

	/// @brief 現在のボール数参照
	int GetBallCount(){
		return currentBallCount;
	}

	/// @brief 全てのボールを出したかどうか
	/// @return 出し終わっていたらTrue
	bool IsCurrentBallMax()
	{
		if (currentBallCount == ballMaxNum)
		{
			return true;
		}
		return false;
	}

	/// @brief 次ボールを増やすのに必要な壊したブロック数
	int NextAddBallBrickCount()
	{
		return  bricksForExtraBall[bricksForExtraBallIndex];
	}

	/// @brief ボール生成の準備を始める
	void BallSpawnPrepareToStart()
	{
		IsBallSpawnPrepare = true;
	}

	/// @brief ボール生成中もしくは準備中かどうか
	bool GetIsBallSpawnPrepare()
	{
		return IsBallSpawnPrepare;
	}

	/// @brief ボール生成時のカウントの値を返す
	float GetBallSpawnCount()
	{
		return BallSpawnCount;
	}
};

class AudioProperty
{
public:
	enum SE
	{
		BrickBreak,
		Button,
	};

private:
	SE se;
	String filePath;

public:
	AudioProperty(SE seState, String path)
	{
		se = seState;
		filePath = path;
	}

	String GetFilePath()
	{
		return filePath;
	}
};

/// @brief 音再生用
class AudioManager
{
private:
	Audio audio;

	/// @brief 音のデータ
	AudioProperty sounds[2] =
	{
		AudioProperty(AudioProperty::SE::BrickBreak, U"example/shot.mp3"),
		AudioProperty(AudioProperty::SE::Button, U"example/shot.mp3"),
	};

public:
	/// @brief 音再生
	/// @param se 再生する音
	void SEPlay(AudioProperty::SE se)
	{
		int id = se;
		String path = sounds[id].GetFilePath();
		audio = Audio{ path };
		audio.playOneShot();
	}
};

/// @brief ブロック
class Bricks final {
private:
	/// @brief ブロックリスト
	Rect brickTable[constants::brick::MAX];
	/// @brief 現在あるブロック数
	int brickCount = constants::brick::MAX;

	AudioManager audioManager;

public:

	/// @brief デストラクタ
	~Bricks() {}

	/// @brief 衝突検知
	void Intersects(Ball* const target);

	/// @brief 描画
	void Draw() const {
		using namespace constants::brick;

		for (int i = 0; i < MAX; ++i) {
			brickTable[i].stretched(-1).draw(HSV{ brickTable[i].y - 40 });
		}
	}

	void Instance()
	{
		using namespace constants::brick;
		brickCount = MAX;
		for (int y = 0; y < Y_COUNT; ++y) {
			for (int x = 0; x < X_COUNT; ++x) {
				int index = y * X_COUNT + x;
				brickTable[index] = Rect{
					x * SIZE.x,
					60 + y * SIZE.y,
					SIZE
				};
			}
		}
	}

	int GetCurrentBrickCount()
	{
		return brickCount;
	}
};

/// @brief パドル
class Paddle final {
private:
	Rect paddle;

public:
	/// @brief コンストラクタ
	Paddle() : paddle(Rect(Arg::center(Cursor::Pos().x, 500), constants::paddle::SIZE)) {}

	/// @brief デストラクタ
	~Paddle() {}

	/// @brief 衝突検知
	void Intersects(Ball* const target) const;

	/// @brief 更新
	void Update() {
		paddle.x = Cursor::Pos().x - (constants::paddle::SIZE.x / 2);
	}

	/// @brief 描画
	void Draw() const {
		paddle.rounded(3).draw();
	}
};

/// @brief 壁
class Wall {
public:
	/// @brief 衝突検知
	static void Intersects(Ball* target) {
		using namespace constants;

		if (!target) {
			return;
		}

		auto velocity = target->GetVelocity();
		auto ball = target->GetCircle();

		// 天井との衝突を検知
		if ((ball.y < 0) && (velocity.y < 0))
		{
			target->Reflect(reflect::VERTICAL);
		}

		// 壁との衝突を検知
		if (((ball.x < 0) && (velocity.x < 0))
			|| ((Scene::Width() < ball.x) && (0 < velocity.x)))
		{
			target->Reflect(reflect::HORIZONTAL);
		}
	}
};

class GameManager
{
private:
	/// @brief 現在のシーン
	SceneState _currentScene;
	float SceneChangeTime = 3;
	float SceneChangeTimer = 0;
public:
	/// @brief シーン遷移
	/// @param state 遷移するシーンのenum
	void SceneChange(SceneState state)
	{
		_currentScene = state;
	}

	/// @brief シーン遷移前に少し待つ
	/// @param DeltaTime 
	/// @return 待ち時間が経過したらTrueを返す
	bool WaitSceneChange(float DeltaTime)
	{
		SceneChangeTimer += DeltaTime;
		if (SceneChangeTimer >= SceneChangeTime)
		{
			SceneChangeTimer = 0;
			return true;
		}
		return false;
	}

	/// @brief 現在のシーン参照
	SceneState GetCurrentScene()
	{
		return _currentScene;
	}
};


//==============================
// 定義
//==============================
void Bricks::Intersects(Ball* const target) {
	using namespace constants;
	using namespace constants::brick;

	if (!target) {
		return;
	}

	auto ball = target->GetCircle();

	for (int i = 0; i < MAX; ++i) {
		// 参照で保持
		Rect& refBrick = brickTable[i];

		// 衝突を検知
		if (refBrick.intersects(ball))
		{
			// ブロックの上辺、または底辺と交差
			if (refBrick.bottom().intersects(ball)
				|| refBrick.top().intersects(ball))
			{
				target->Reflect(reflect::VERTICAL);
			}
			else // ブロックの左辺または右辺と交差
			{
				target->Reflect(reflect::HORIZONTAL);
			}

			// あたったブロックは画面外に出す
			refBrick.y -= 600;

			//ブロックの数を減らす
			brickCount--;

			//音を出す
			audioManager.SEPlay(AudioProperty::SE::BrickBreak);

			// 同一フレームでは複数のブロック衝突を検知しない
			break;
		}
	}
}

void Paddle::Intersects(Ball* const target) const {
	if (!target) {
		return;
	}

	auto velocity = target->GetVelocity();
	auto ball = target->GetCircle();

	if ((0 < velocity.y) && paddle.intersects(ball))
	{
		target->SetVelocity(Vec2{
			(ball.x - paddle.center().x) * 10,
			-velocity.y
		});
	}
}

//==============================
// エントリー
//==============================
void Main()
{
	Bricks bricks;
	Paddle paddle;
	BallSpawner ballSpawner;
	GameManager gameManager;
	AudioManager audioManager;
	//タイトルシーンにする
	gameManager.SceneChange(SceneState::TITLE);

	const Font font60{ FontMethod::MSDF, 60 };
	const Font font30{ FontMethod::MSDF, 30 };
	const Font font80{ FontMethod::MSDF, 80 };
	const Font font100{ FontMethod::MSDF, 100 };

	int32 count = 0;

	while (System::Update())
	{
		switch (gameManager.GetCurrentScene())
		{

			//タイトルシーン
			case SceneState::TITLE:
				//タイトル表示
				font100(U"Breaking blocks").drawAt(Vec2{ Scene::Center().x, 200 },ColorF{ 0.2, 0.6, 0.9 });
				//概要説明
				font30(U"このブロック崩しは").drawAt(Vec2{ Scene::Center().x, 500 }, ColorF{ 0.2, 0.6, 0.9 });
				font30(U"ブロックを崩すとボールが増えていきます").drawAt(Vec2{ Scene::Center().x, 550 }, ColorF{ 0.2, 0.6, 0.9 });
				//Startボタン表示
				if (SimpleGUI::Button(U"START", Vec2{ Scene::Center().x - 50, Scene::Center().y + 80 }, 100))
				{
					//音を出す
					audioManager.SEPlay(AudioProperty::SE::Button);

					gameManager.SceneChange(SceneState::IN_GAME);
					//ブロックの初期化
					bricks.Instance();
					//ボール生成
					ballSpawner.Generate();
				}
				break;


			//ゲームシーン
			case SceneState::IN_GAME:

				// 更新
				paddle.Update();
				for (int i = 0; i < ballSpawner.GetBallCount(); i++) {
					ballSpawner.GetBalls(i)->Update();
				}

				// コリジョン
				for (int i = 0; i < ballSpawner.GetBallCount(); i++) {
					Ball* ball = ballSpawner.GetBalls(i);
					bricks.Intersects(ball);
					Wall::Intersects(ball);
					paddle.Intersects(ball);
				}

				// 描画
				bricks.Draw();
				paddle.Draw();
				for (int i = 0; i < ballSpawner.GetBallCount(); i++) {
					ballSpawner.GetBalls(i)->Draw();
				}

				//Text表示
				int brickCountDown = bricks.GetCurrentBrickCount() - ballSpawner.NextAddBallBrickCount();
				if (brickCountDown > 0)
				{
					font30(U"ボール増えるまで あと {} 個"_fmt(brickCountDown))
						.drawAt(Vec2{ Scene::Center().x, 550 }, ColorF{ 0.2, 0.6, 0.9 });
				}
				else if(bricks.GetCurrentBrickCount() != 0)
				{
					font30(U"頑張ってください").drawAt(Vec2{ Scene::Center().x, 550 }, ColorF{ 0.2, 0.6, 0.9 });
				}

				//ボール生成Trigger
				if (!ballSpawner.IsCurrentBallMax() && bricks.GetCurrentBrickCount() == ballSpawner.NextAddBallBrickCount())
				{
					//ボール生成カウントダウンを始める
					ballSpawner.BallSpawnPrepareToStart();
				}
				if (ballSpawner.GetIsBallSpawnPrepare())
				{
					//カウントダウン
					ballSpawner.BallSpawnCountDown(Scene::DeltaTime());
					font80(U"{:.0f}"_fmt(ballSpawner.GetBallSpawnCount())).drawAt(Scene::Center(), ColorF{ 0.2, 0.6, 0.9, 0.5 });
				}

				//弾が下に落ちたらTitleに戻る
				for (int i = 0; i < ballSpawner.GetBallCount(); i++) {
					Circle ball = ballSpawner.GetBalls(i)->GetCircle();
					if (ball.y > 800)
					{
						//ボールメモリ解放
						ballSpawner.Delete();
						//タイトルに戻る
						gameManager.SceneChange(SceneState::TITLE);
					}
				}

				//ブロック数が０になったら
				if (bricks.GetCurrentBrickCount() == 0)
				{
					font60(U"GameClear").drawAt(Scene::Center(), ColorF{ 0.2, 0.6, 0.9 });
					font30(U"おめでとうございます").drawAt(Vec2{ Scene::Center().x, 550 }, ColorF{ 0.2, 0.6, 0.9 });
					ballSpawner.Delete();
					//Titleに戻るまで一定時間待つ
					if (gameManager.WaitSceneChange(Scene::DeltaTime()))
					{
						//Titleに戻る
						gameManager.SceneChange(SceneState::TITLE);
					}
				}

				break;
		}
	}
	ballSpawner.Delete();
}

