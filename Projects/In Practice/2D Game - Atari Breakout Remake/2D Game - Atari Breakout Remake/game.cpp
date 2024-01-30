#include <algorithm>
#include <sstream>
#include <irrKlang.h>

#include "game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "game_object.h"
#include "ball_object.h"
#include "particle_generator.h"
#include "post_processor.h"
#include "text_renderer.h"

// Game-related State data
SpriteRenderer*    Renderer;
GameObject*		   Player;
BallObject*		   Ball;
ParticleGenerator* Particles;
PostProcessor*	   Effects;
irrklang::ISoundEngine* SoundEngine = irrklang::createIrrKlangDevice();
TextRenderer* Text;

// variables
float ShakeTime = 0.0f;

// Collision detection functions
bool CheckCollision(GameObject& one, GameObject& two);
Collision CheckCollision(BallObject& one, GameObject& two);
Direction VectorDirection(glm::vec2 target);

bool ShouldSpawn(uint32_t chance);
void ActivatePowerUP(PowerUp& powerUp);
bool isOtherPowerUpActive(std::vector<PowerUp>& powerUps, std::string type);

Game::Game(uint32_t width, uint32_t height)
	: State(GameState::GAME_MENU), Keys(), KeysProcessed(), Width(width), Height(height), Lives(3)
{ }

Game::~Game()
{
	delete Renderer;
	delete Player;
	delete Ball;
	delete Particles;
	delete Effects;
}

void Game::Init()
{
	// load shaders
	ResourceManager::LoadShader("./shaders/sprite.glsl", "sprite");
	ResourceManager::LoadShader("./shaders/particle_shader.glsl", "particle");
	ResourceManager::LoadShader("./shaders/postprocessing_shader.glsl", "postprocessing");
	// configure shaders
	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
	ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
	ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
	ResourceManager::GetShader("particle").Use().SetInteger("sprite", 0);
	ResourceManager::GetShader("particle").SetMatrix4("projection", projection);
	// load textures
	ResourceManager::LoadTexture("./resources/textures/background.jpg", false, "background");
	ResourceManager::LoadTexture("./resources/textures/block.png", false, "block");
	ResourceManager::LoadTexture("./resources/textures/block_solid.png", false, "block_solid");
	ResourceManager::LoadTexture("./resources/textures/paddle.png", true, "paddle");
	ResourceManager::LoadTexture("./resources/textures/awesomeface.png", true, "face");
	ResourceManager::LoadTexture("./resources/textures/particle.png", true, "particle");
	ResourceManager::LoadTexture("./resources/textures/powerup_speed.png", true, "powerup_speed");
	ResourceManager::LoadTexture("./resources/textures/powerup_sticky.png", true, "powerup_sticky");
	ResourceManager::LoadTexture("./resources/textures/powerup_increase.png", true, "powerup_increase");
	ResourceManager::LoadTexture("./resources/textures/powerup_confuse.png", true, "powerup_confuse");
	ResourceManager::LoadTexture("./resources/textures/powerup_chaos.png", true, "powerup_chaos");
	ResourceManager::LoadTexture("./resources/textures/powerup_passthrough.png", true, "powerup_passthrough");
	// set render-specific controls
	Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
	Particles = new ParticleGenerator(ResourceManager::GetShader("particle"), ResourceManager::GetTexture("particle"), 500);
	Effects = new PostProcessor(ResourceManager::GetShader("postprocessing"), this->Width, this->Height);
	// load levels
	GameLevel one; one.Load("./resources/levels/1.Standard.level", this->Width, this->Height / 2);
	GameLevel two; two.Load("./resources/levels/2.A_few_small_gaps.level", this->Width, this->Height / 2);
	GameLevel three; three.Load("./resources/levels/3.Space_invader.level", this->Width, this->Height / 2);
	GameLevel four; four.Load("./resources/levels/4.Bounce_galore.level", this->Width, this->Height / 2);
	this->Levels.push_back(one);
	this->Levels.push_back(two);
	this->Levels.push_back(three);
	this->Levels.push_back(four);
	this->Level = 0;
	// configure game objects
	glm::vec2 playerPos = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
	Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));
	// configure ball object
	glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -BALL_RADIUS * 2.0f);
	Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));
	// audio
	//SoundEngine->play2D("./resources/audio/breakout.mp3", true);
	// text
	Text = new TextRenderer(this->Width, this->Height);
	Text->Load("./resources/fonts/JOKERMAN.TTF", 24);
}

void Game::ProcessInput(float dt)
{
	if (this->State == GameState::GAME_MENU)
	{
		if (this->Keys[GLFW_KEY_ENTER] && !this->KeysProcessed[GLFW_KEY_ENTER])
		{
			this->State = GameState::GAME_ACTIVE;
			SoundEngine->play2D("./resources/audio/breakout.mp3", true);
			this->KeysProcessed[GLFW_KEY_ENTER] = true;
		}
		if (this->Keys[GLFW_KEY_W] && !this->KeysProcessed[GLFW_KEY_W])
		{
			this->Level = (this->Level + 1) % this->Levels.size();
			SoundEngine->play2D("./resources/audio/ball_hit_non-solid.mp3", false);
			this->KeysProcessed[GLFW_KEY_W] = true;
		}
		if (this->Keys[GLFW_KEY_S] && !this->KeysProcessed[GLFW_KEY_S])
		{
			if (this->Level > 0)
				--this->Level;
			else
				this->Level = this->Levels.size() - 1;
			SoundEngine->play2D("./resources/audio/ball_bounces_paddle.wav", false);
			this->KeysProcessed[GLFW_KEY_S] = true;
		}
	}
	if (this->State == GameState::GAME_ACTIVE)
	{
		float velocity = PLAYER_VELOCITY * dt;
		// move playerboard
		if (this->Keys[GLFW_KEY_A])
		{
			if (Player->Position.x >= 0.0f)
			{
				Player->Position.x -= velocity;
				if (Ball->Stuck)
					Ball->Position.x -= velocity;
			}
		}
		if (this->Keys[GLFW_KEY_D])
		{
			if (Player->Position.x + Player->Size.x <= this->Width)
			{
				Player->Position.x += velocity;
				if (Ball->Stuck)
					Ball->Position.x += velocity;
			}
		}
		if (this->Keys[GLFW_KEY_SPACE])
			Ball->Stuck = false;
	}
	if (this->State == GameState::GAME_WIN)
	{
		if (this->Keys[GLFW_KEY_ENTER])
		{
			this->KeysProcessed[GLFW_KEY_ENTER] = true;
			Effects->Chaos = false;
			this->State = GameState::GAME_MENU;
		}
	}
}

void Game::Update(float dt)
{
	// update objects
	Ball->Move(dt, this->Width);
	// check for collisions
	this->DoCollisions();
	// update particles
	Particles->Update(dt, *Ball, 1, glm::vec2(Ball->Radius / 2.0f));
	// update PowerUps
	this->UpdatePowerUps(dt);
	// reduce shake time
	if (ShakeTime > 0.0f)
	{
		ShakeTime -= dt;
		if (ShakeTime <= 0.0f)
			Effects->Shake = false;
	}
	// check loss condition
	if (Ball->Position.y >= this->Height) // if ball reach bottom edge
	{
		--this->Lives;
		// if player lose all his lives
		if (this->Lives == 0)
		{
			this->ResetLevel();
			this->State = GameState::GAME_MENU;
			SoundEngine->stopAllSounds();
		}
		this->ResetPlayer();
	}
	// check winning condition
	if (this->State == GameState::GAME_ACTIVE && this->Levels[this->Level].IsCompleted())
	{
		this->ResetLevel();
		this->ResetPlayer();
		Effects->Chaos = true;
		this->State = GameState::GAME_WIN;
	}
}

void Game::Render()
{
	Effects->BeginRender();

	{
		// draw background
		Renderer->DrawSprite(ResourceManager::GetTexture("background"), glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);
		// draw level
		this->Levels[this->Level].Draw(*Renderer);
		// draw player
		Player->Draw(*Renderer);
		// draw PowerUps
		for (PowerUp& powerUp : this->PowerUps)
		{
			if (!powerUp.Destroyed)
				powerUp.Draw(*Renderer);
		}
		// draw particles
		Particles->Draw();
		// draw ball
		Ball->Draw(*Renderer);
	}

	Effects->EndRender();
	Effects->Render(glfwGetTime());

	if (this->State == GameState::GAME_ACTIVE)
	{
		// render text
		std::stringstream ss;
		ss << this->Lives;
		Text->RenderText("Lives: " + ss.str(), 5.0f, 5.0f, 1.0f);
	}
	else if (this->State == GameState::GAME_MENU)
	{
		Text->RenderText("Press ENTER to start", 250.0f, Height / 2, 1.0f);
		Text->RenderText("Press W or S to select level", 245.0f, Height / 2 + 20.0f, 0.75f);
	}
	else if (this->State == GameState::GAME_WIN)
	{
		Text->RenderText("YOU WON!!!", 320.0f, Height / 2.0f - 20.0f, 1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		Text->RenderText(
			"Press ENTER to retry or ESC to quit", 130.0f, Height / 2.0f, 1.0f, glm::vec3(1.0f, 1.0f, 0.0f)
		);
	}
}

void Game::DoCollisions()
{
	for (GameObject& box : this->Levels[this->Level].Bricks)
	{
		if (!box.Destroyed)
		{
			Collision collision = CheckCollision(*Ball, box);
			if (std::get<0>(collision)) // if collision is true
			{
				// destroy block if not solid
				if (!box.IsSolid)
				{
					box.Destroyed = true;
					this->SpawnPowerUps(box);
					// audio
					SoundEngine->play2D("./resources/audio/ball_hit_non-solid.mp3", false);
				}
				else // if block is solid, enable shake effect
				{
					ShakeTime = 0.05f;
					Effects->Shake = true;
					// audio
					SoundEngine->play2D("./resources/audio/ball_hit_solid.wav", false);
				}
				// collision resolution
				Direction dir = std::get<1>(collision);
				glm::vec2 diff_vector = std::get<2>(collision);
				if (!(Ball->PassThrough && !box.IsSolid)) // don't do collision resolution on non-solid bricks if pass-through is activated
				{
					if (dir == Direction::LEFT || dir == Direction::RIGHT) // horizontal collision
					{
						Ball->Velocity.x = -Ball->Velocity.x; // reverse horizontal velocity
						// relocate
						float penetration = Ball->Radius - std::abs(diff_vector.x);
						if (dir == Direction::LEFT)
							Ball->Position.x += penetration; // move ball to right
						else
							Ball->Position.x -= penetration; // move ball to left
					}
					else // vertical collision
					{
						Ball->Velocity.y = -Ball->Velocity.y; // reverse vertical velocity
						// relocate
						float penetration = Ball->Radius - std::abs(diff_vector.y);
						if (dir == Direction::UP)
							Ball->Position.y += penetration; // move ball back down
						else
							Ball->Position.y -= penetration; // move ball back up
					}
				}
			}
		}
	}
	// check collision for player pad (unless stuck)
	Collision result = CheckCollision(*Ball, *Player);
	if (!Ball->Stuck && std::get<0>(result))
	{
		// check where it hit the board, and change velocity based on where it hit the board
		float centerBoard = Player->Position.x + Player->Size.x / 2.0f;
		float distance = (Ball->Position.x + Ball->Radius) - centerBoard;
		float percentage = distance / (Player->Size.x / 2.0f);
		// then move accordingly
		float strength = 2.0f;
		float velocityLength = glm::length(Ball->Velocity);
		Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
		//Ball->Velocity.y = -Ball->Velocity.y;
		Ball->Velocity = glm::normalize(Ball->Velocity) * velocityLength; // keep speed consistent over both axes (multiply by length of old velocity, so total strength is not changed)
		// fix sticky paddle
		Ball->Velocity.y = -1.0f * abs(Ball->Velocity.y);
		Ball->Stuck = Ball->Sticky;
		// audio
		SoundEngine->play2D("./resources/audio/ball_bounces_paddle.wav", false);
	}
	// check collision for powerups
	for (PowerUp& powerUp : this->PowerUps)
	{
		if (!powerUp.Destroyed)
		{
			if (powerUp.Position.y >= this->Height)
				powerUp.Destroyed = true;
			// collided with player, now activate powerup
			if (CheckCollision(*Player, powerUp))
			{
				ActivatePowerUP(powerUp);
				powerUp.Destroyed = true;
				powerUp.Activated = true;
				// audio
				SoundEngine->play2D("./resources/audio/paddle_collided_powerup.wav", false);
			}
		}
	}
}
void ActivatePowerUP(PowerUp& powerUp)
{
	if (powerUp.Type == "speed")
	{
		Ball->Velocity *= 1.2;
	}
	else if (powerUp.Type == "sticky")
	{
		Ball->Sticky = true;
		Player->Color = glm::vec3(1.0f, 0.5f, 1.0f);
	}
	else if (powerUp.Type == "pass-through")
	{
		Ball->PassThrough = true;
		Ball->Color = glm::vec3(1.0f, 0.5f, 0.5f);
	}
	else if (powerUp.Type == "pad-size-increase")
	{
		Player->Size.x += 50;
	}
	else if (powerUp.Type == "confuse")
	{
		if (!Effects->Chaos)
			Effects->Confuse = true; // only activate if chaos wasn't already active
	}
	else if (powerUp.Type == "chaos")
	{
		if (!Effects->Confuse)
			Effects->Chaos = true;
	}
}

void Game::ResetLevel()
{
	if (this->Level == 0)
		this->Levels[0].Load("./resources/levels/1.Standard.level", this->Width, this->Height / 2);
	else if (this->Level == 1)
		this->Levels[1].Load("./resources/levels/2.A_few_small_gaps.level", this->Width, this->Height / 2);
	else if (this->Level == 2)
		this->Levels[2].Load("./resources/levels/3.Space_invader.level", this->Width, this->Height / 2);
	else if (this->Level == 3)
		this->Levels[3].Load("./resources/levels/4.Bounce_galore.level", this->Width, this->Height / 2);
	this->Lives = 3;
}

void Game::ResetPlayer()
{
	// reset player/ball stats
	Player->Size = PLAYER_SIZE;
	Player->Position = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
	Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -(BALL_RADIUS * 2.0f)), INITIAL_BALL_VELOCITY);
	// also disable all active powerups
	Effects->Chaos = Effects->Confuse = false;
	Ball->PassThrough = Ball->Sticky = false;
	Player->Color = glm::vec3(1.0f);
	Ball->Color = glm::vec3(1.0f);
}


bool ShouldSpawn(uint32_t chance)
{
	uint32_t random = rand() % chance;
	return random == 0;
}
void Game::SpawnPowerUps(GameObject& block)
{
	if (ShouldSpawn(75)) // 1 in 75 chance
		this->PowerUps.push_back(PowerUp("speed", glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, block.Position, ResourceManager::GetTexture("powerup_speed")));
	if (ShouldSpawn(75))
		this->PowerUps.push_back(PowerUp("sticky", glm::vec3(1.0f, 0.5f, 1.0f), 20.0f, block.Position, ResourceManager::GetTexture("powerup_sticky")));
	if (ShouldSpawn(75))
		this->PowerUps.push_back(PowerUp("pass-through", glm::vec3(0.5f, 1.0f, 0.5f), 15.0f, block.Position, ResourceManager::GetTexture("powerup_passthrough")));
	if (ShouldSpawn(75))
		this->PowerUps.push_back(PowerUp("pad-size-increase", glm::vec3(1.0f, 0.6f, 0.4), 0.0f, block.Position, ResourceManager::GetTexture("powerup_increase")));
	if (ShouldSpawn(15)) // Negative powerups should spawn more often
		this->PowerUps.push_back(PowerUp("confuse", glm::vec3(1.0f, 0.3f, 0.3f), 10.0f, block.Position, ResourceManager::GetTexture("powerup_confuse")));
	if (ShouldSpawn(15))
		this->PowerUps.push_back(PowerUp("chaos", glm::vec3(0.9f, 0.25f, 0.25f), 10.0f, block.Position, ResourceManager::GetTexture("powerup_chaos")));
}

void Game::UpdatePowerUps(float dt)
{
	for (PowerUp& powerUp : this->PowerUps)
	{
		powerUp.Position += powerUp.Velocity * dt;
		if (powerUp.Activated)
		{
			powerUp.Duration -= dt;

			if (powerUp.Duration <= 0.0f)
			{
				// remove powerup from list (will later be removed)
				powerUp.Activated = false;
				// deactivate effects
				if (powerUp.Type == "sticky")
				{
					if (!isOtherPowerUpActive(this->PowerUps, "sticky"))
					{	// only reset if no other PowerUp of type sticky is active
						Ball->Sticky = false;
						Player->Color = glm::vec3(1.0f);
					}
				}
				else if (powerUp.Type == "pass-through")
				{
					if (!isOtherPowerUpActive(this->PowerUps, "pass-through"))
					{	// only reset if no other PowerUp of type pass-through is active
						Ball->PassThrough = false;
						Ball->Color = glm::vec3(1.0f);
					}
				}
				else if (powerUp.Type == "confuse")
				{
					if (!isOtherPowerUpActive(this->PowerUps, "confuse"))
					{	// only reset if no other PowerUp of type confuse is active
						Effects->Confuse = false;
					}
				}
				else if (powerUp.Type == "chaos")
				{
					if (!isOtherPowerUpActive(this->PowerUps, "chaos"))
					{	// only reset if no other PowerUp of type chaos is active
						Effects->Chaos = false;
					}
				}
			}
		}
	}
	this->PowerUps.erase(std::remove_if(this->PowerUps.begin(), this->PowerUps.end(),
		[](const PowerUp& powerUp) { return powerUp.Destroyed && !powerUp.Activated; }
	), this->PowerUps.end());
}
bool isOtherPowerUpActive(std::vector<PowerUp>& powerUps, std::string type)
{
	for (const PowerUp& powerUp : powerUps)
	{
		if (powerUp.Activated)
		{
			if (powerUp.Type == type)
				return true;
		}
	}
	return false;
}

bool CheckCollision(GameObject& one, GameObject& two) // AABB - AABB collision
{
	// collision x-axis?
	bool collisionX = one.Position.x + one.Size.x >= two.Position.x && two.Position.x + two.Size.x >= one.Position.x;
	// collision y-axis?
	bool collisionY = one.Position.y + one.Size.y >= two.Position.y && two.Position.y + two.Size.y >= one.Position.y;
	// collision only if on both axes
	return collisionX && collisionY;
}

Collision CheckCollision(BallObject& one, GameObject& two)
{
	// get center point circle first
	glm::vec2 center(one.Position + one.Radius);
	// calculate AABB info (center, half-extents_
	glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
	glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x, two.Position.y + aabb_half_extents.y);
	// get difference vector between both centers
	glm::vec2 difference = center - aabb_center;
	glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
	// add clamped value to AABB_center and we get the value of box closest to circle
	glm::vec2 closest = aabb_center + clamped;
	// retrieve vector between center circle and closest point AABB and check if length <= radius
	difference = closest - center;
	
	if (glm::length(difference) < one.Radius) // not <= since in that case a collision also occurs when object one exactly touches object two, which they are at the end of each collision resolution stage.
		return std::make_tuple(true, VectorDirection(difference), difference);
	else
		return std::make_tuple(false, Direction::UP, glm::vec2(0.0f, 0.0f));
}

// calculates which direction a vector is facing (N,E,S or W)
Direction VectorDirection(glm::vec2 target)
{
	glm::vec2 compass[] = {
		glm::vec2(0.0f, -1.0f), // up
		glm::vec2(1.0f, 0.0f),	// right
		glm::vec2(0.0f, 1.0f),	// down
		glm::vec2(-1.0f, 0.0f)	// left
	};
	float max = 0.0f;
	unsigned int best_match = -1;
	for (unsigned int i = 0; i < 4; i++)
	{
		float dot_product = glm::dot(glm::normalize(target), compass[i]);
		if (dot_product > max)
		{
			max = dot_product;
			best_match = i;
		}
	}
	return (Direction)best_match;
}