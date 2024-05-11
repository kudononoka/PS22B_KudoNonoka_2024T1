# include <Siv3D.hpp>

/*
	古き良き書き方での実装
	・安全性や利便性などは一切考えていない
*/

namespace constants{

	namespace ball {
		/// @brief ボールの速さ
		constexpr double BALL_SPEED = 480.0;
	}

	namespace bricks {
		/// @brief ブロックのサイズ
		constexpr Size BRICK_SIZE{ 40, 20 };

		/// @brief ブロックの数　縦
		constexpr int Y_COUNT = 5;

		/// @brief ブロックの数　横
		constexpr int X_COUNT = 20;

		/// @brief 合計ブロック数
		constexpr int MAX = Y_COUNT * X_COUNT;
	}

	namespace paddle {
		constexpr Size SIZE{ 40, 20 };
	}
}

class Ball {
public:
	Vec2 velocity;
	Circle model;

	Ball() :
		velocity(0, -constants::ball::BALL_SPEED),
		model{400,400,10} {}

	void Update() {
		model.moveBy(velocity * Scene::DeltaTime());
	}

	void Draw() {
		model.draw();
	}
};

class Bricks {
public:
	Rect bricksModel[constants::bricks::MAX];

	Bricks() {
		using namespace constants::bricks;
		for (int y = 0; y < Y_COUNT; ++y) {
			for (int x = 0; x < X_COUNT; ++x) {
				int index = y * X_COUNT + x;
				bricksModel[index] = Rect{
					x * BRICK_SIZE.x,
					60 + y * BRICK_SIZE.y,
					BRICK_SIZE
				};
			}
		}
	}

	void Draw() {
		for (int i = 0; i < constants::bricks::MAX; ++i) {
			bricksModel[i].stretched(-1).draw(HSV{ bricksModel[i].y - 40 });
	    }
	}

	void Intersects(Ball& ball) {
		for (int i = 0; i < constants::bricks::MAX; ++i) {
			// 参照で保持
			Rect& refBrick = bricksModel[i];

			// 衝突を検知
			if (refBrick.intersects(ball.model))
			{
				// ブロックの上辺、または底辺と交差
				if (refBrick.bottom().intersects(ball.model) || refBrick.top().intersects(ball.model))
				{
					ball.velocity.y *= -1;
				}
				else // ブロックの左辺または右辺と交差
				{
					ball.velocity.x *= -1;
				}

				// あたったブロックは画面外に出す
				refBrick.y -= 600;

				// 同一フレームでは複数のブロック衝突を検知しない
				break;
			}
		}
	}
};

class Paddle {
public:
	Rect model;

	Paddle() :
		model(Arg::center(Cursor::Pos().x, 500), 60, 10)
	{}

	void Update() {
		model.x = Cursor::Pos().x - (constants::paddle::SIZE.x / 2);
	}

	void Intersects(Ball& ball) {
		if ((0 < ball.velocity.y) && model.intersects(ball.model))
        {
			ball.velocity = Vec2{
		    (ball.model.x - model.center().x) * 10,
		    -ball.velocity.y
	        }.setLength(constants::ball::BALL_SPEED);
        }
	}

	void Draw() {
		model.rounded(3).draw();
	}
};

void Main()
{
	Ball ball;
	Bricks bricks;
	Paddle paddle;

	while (System::Update())
	{
		//更新
		ball.Update();
		paddle.Update();

		//当たり判定
		bricks.Intersects(ball);
		paddle.Intersects(ball);

		//描画
		ball.Draw();
		bricks.Draw();
		paddle.Draw();

		////==============================
		//// コリジョン
		////==============================

		//// 天井との衝突を検知
		//if ((ballModel.y < 0) && (ballVelocity.y < 0))
		//{
		//	ballVelocity.y *= -1;
		//}

		//// 壁との衝突を検知
		//if (((ballModel.x < 0) && (ballVelocity.x < 0))
		//	|| ((Scene::Width() < ballModel.x) && (0 < ballVelocity.x)))
		//{
		//	ballVelocity.x *= -1;
		//}
	}
}
