//
//  GameLevel.hpp
//  Game-Thing
//
//  Created by Liam Horch on 2/4/23.
//

#ifndef GameLevel_hpp
#define GameLevel_hpp

#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "GameObject.hpp"
#include "sprite_renderer.h"
#include "resource_manager.h"


/// GameLevel holds all Tiles as part of a Breakout level and
/// hosts functionality to Load/render levels from the harddisk.
class GameLevel
{
public:
    // level state
    std::vector<GameObject> Bricks;
    // constructor
    GameLevel() { }
    // loads level from file
    void Load(const char *file, unsigned int levelWidth, unsigned int levelHeight);
    // render level
    void Draw(SpriteRenderer &renderer);
    // check if the level is completed (all non-solid tiles are destroyed)
    bool IsCompleted();
private:
    // initialize level from tile data
    void init(std::vector<std::vector<unsigned int>> tileData, unsigned int levelWidth, unsigned int levelHeight);
};

#endif /* GameLevel_hpp */
