/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#include "game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "ball.hpp"
#include "ParticleGenerator.hpp"


// Game-related State data
SpriteRenderer *Renderer;
SpriteRenderer *guiRenderer;

Ball *ball;
GameObject *Player;
GameObject *gui;
ParticleGenerator *particles;


Game::Game(unsigned int width, unsigned int height)
    : State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{
    
}

Game::~Game()
{
    delete Renderer;
    delete ball;
    delete Player;
    delete particles;
}

void Game::Init()
{
    // load shaders
    ResourceManager::LoadShader("shaders/sprite.vs", "shaders/sprite.frag", nullptr, "sprite");
    ResourceManager::LoadShader("shaders/particles.vs", "shaders/particles.frag", nullptr, "particle");
    ResourceManager::LoadShader("shaders/guiVs", "shaders/guiFrag", nullptr, "gui");
    // configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width),
        static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
    ResourceManager::GetShader("particle").Use().SetInteger("sprite", 0);
    ResourceManager::GetShader("particle").SetMatrix4("projection", projection);
    ResourceManager::GetShader("gui").Use().SetInteger("image", 0);
    // load textures
    ResourceManager::LoadTexture("textures/background.jpg", false, "background");
    ResourceManager::LoadTexture("textures/awesomeface.png", true, "face");
    ResourceManager::LoadTexture("textures/block.png", false, "block");
    ResourceManager::LoadTexture("textures/block_solid.png", false, "block_solid");
    ResourceManager::LoadTexture("textures/paddle.png", true, "paddle");
    // set render-specific controls
    Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
    particles = new ParticleGenerator(ResourceManager::GetShader("particle"), ResourceManager::GetTexture("face"), 500);
    guiRenderer = new SpriteRenderer(ResourceManager::GetShader("gui"));
    // load levels
    GameLevel one; one.Load("levels/one.lvl", this->Width, this->Height / 2);
    GameLevel two; two.Load("levels/two.lvl", this->Width, this->Height / 2);
    GameLevel three; three.Load("levels/three.lvl", this->Width, this->Height / 2);
    GameLevel four; four.Load("levels/four.lvl", this->Width, this->Height / 2);
    this->Levels.push_back(one);
    this->Levels.push_back(two);
    this->Levels.push_back(three);
    this->Levels.push_back(four);
    this->Level = 0;
    // player object
    glm::vec2 playerPos = glm::vec2(
        this->Width / 2.0f - PLAYER_SIZE.x / 2.0f,
        this->Height - PLAYER_SIZE.y
    );
    glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS,
                                                  -BALL_RADIUS * 2.0f);
    Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));
    gui = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("face"));
    ball = new Ball(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY,
        ResourceManager::GetTexture("face"));
}

void Game::Update(float dt)
{
    ball->move(dt, this->Width);
    
    this->DoCollisions();
    
    particles->Update(dt, *ball, 2, glm::vec2(ball->radius / 2.0f));
    
    if (ball->Position.y >= this->Height) // did ball reach bottom edge?
    {
        this->ResetLevel();
        this->ResetPlayer();
    }
}

void Game::ProcessInput(float dt)
{
    if (this->State == GAME_ACTIVE)
    {
        float velocity = PLAYER_VELOCITY * dt;
        // move playerboard
        if (this->Keys[GLFW_KEY_A])
        {
            if (Player->Position.x >= 0.0f) {
                Player->Position.x -= velocity;
                if (ball->stuck)
                    ball->Position.x -= velocity;
            }
        }
        if (this->Keys[GLFW_KEY_D])
        {
            if (Player->Position.x <= this->Width - Player->Size.x) {
                Player->Position.x += velocity;
                if (ball->stuck)
                    ball->Position.x += velocity;
            }
        }
        if (this->Keys[GLFW_KEY_SPACE]) {
            ball->stuck = false;
        }
    }
}

void Game::Render()
{
    if(this->State == GAME_ACTIVE)
    {
        // draw background
        Renderer->DrawSprite(ResourceManager::GetTexture("background"),
            glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f
        );
        // draw player
        Player->Draw(*Renderer);
        // draw level
        this->Levels[this->Level].Draw(*Renderer);
        // draw particles
        particles->Draw();
        // draw ball
        ball->Draw(*Renderer);
        
//        gui->Draw(*guiRenderer);
    }
}

void Game::ResetPlayer() {
    Player->Position = glm::vec2((this->Width/2) - (Player->Size.x/2),
                                 this->Height - Player->Size.y);
    ball->Position = glm::vec2((this->Width/2) - (ball->Size.x/2), Player->Position.y - ball->Size.y);
    ball->stuck = true;
}

void Game::ResetLevel() {
    for(GameObject &box : this->Levels[this->Level].Bricks) {
        box.Destroyed = false;
    }
}

// collision detection
bool CheckCollision(GameObject &one, GameObject &two);
Collision CheckCollision(Ball &one, GameObject &two);
Direction VectorDirection(glm::vec2 closest);

void Game::DoCollisions()
{
    for (GameObject &box : this->Levels[this->Level].Bricks)
    {
        if (!box.Destroyed)
        {
            Collision collision = CheckCollision(*ball, box);
            if (std::get<0>(collision)) // if collision is true
            {
                // destroy block if not solid
                if (!box.IsSolid)
                    box.Destroyed = true;
                // collision resolution
                Direction dir = std::get<1>(collision);
                glm::vec2 diff_vector = std::get<2>(collision);
                if (dir == LEFT || dir == RIGHT) // horizontal collision
                {
                    ball->Velocity.x = -ball->Velocity.x; // reverse horizontal velocity
                    // relocate
                    float penetration = ball->radius - std::abs(diff_vector.x);
                    if (dir == LEFT)
                        ball->Position.x += penetration; // move ball to right
                    else
                        ball->Position.x -= penetration; // move ball to left;
                }
                else // vertical collision
                {
                    ball->Velocity.y = -ball->Velocity.y; // reverse vertical velocity
                    // relocate
                    float penetration = ball->radius - std::abs(diff_vector.y);
                    if (dir == UP)
                        ball->Position.y -= penetration; // move ball bback up
                    else
                        ball->Position.y += penetration; // move ball back down
                }
            }
        }
    }
    // check collisions for player pad (unless stuck)
    Collision result = CheckCollision(*ball, *Player);
    if (!ball->stuck && std::get<0>(result))
    {
        // check where it hit the board, and change velocity based on where it hit the board
        float centerBoard = Player->Position.x + Player->Size.x / 2.0f;
        float distance = (ball->Position.x + ball->radius) - centerBoard;
        float percentage = distance / (Player->Size.x / 2.0f);
        // then move accordingly
        float strength = 2.0f;
        glm::vec2 oldVelocity = ball->Velocity;
        ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
        //Ball->Velocity.y = -Ball->Velocity.y;
        ball->Velocity = glm::normalize(ball->Velocity) * glm::length(oldVelocity); // keep speed consistent over both axes (multiply by length of old velocity, so total strength is not changed)
        // fix sticky paddle
        ball->Velocity.y = -1.0f * abs(ball->Velocity.y);
    }
}

bool CheckCollision(GameObject &one, GameObject &two) // AABB - AABB collision
{
    // collision x-axis?
    bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
        two.Position.x + two.Size.x >= one.Position.x;
    // collision y-axis?
    bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
        two.Position.y + two.Size.y >= one.Position.y;
    // collision only if on both axes
    return collisionX && collisionY;
}

Collision CheckCollision(Ball &one, GameObject &two) // AABB - Circle collision
{
    // get center point circle first
    glm::vec2 center(one.Position + one.radius);
    // calculate AABB info (center, half-extents)
    glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
    glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x, two.Position.y + aabb_half_extents.y);
    // get difference vector between both centers
    glm::vec2 difference = center - aabb_center;
    glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
    // now that we know the clamped values, add this to AABB_center and we get the value of box closest to circle
    glm::vec2 closest = aabb_center + clamped;
    // now retrieve vector between center circle and closest point AABB and check if length < radius
    difference = closest - center;

    if (glm::length(difference) < one.radius) // not <= since in that case a collision also occurs when object one exactly touches object two, which they are at the end of each collision resolution stage.
        return std::make_tuple(true, VectorDirection(difference), difference);
    else
        return std::make_tuple(false, UP, glm::vec2(0.0f, 0.0f));
}

// calculates which direction a vector is facing (N,E,S or W)
Direction VectorDirection(glm::vec2 target)
{
    glm::vec2 compass[] = {
        glm::vec2(0.0f, 1.0f),    // up
        glm::vec2(1.0f, 0.0f),    // right
        glm::vec2(0.0f, -1.0f),    // down
        glm::vec2(-1.0f, 0.0f)    // left
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
