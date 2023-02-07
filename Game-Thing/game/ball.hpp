//
//  ball.hpp
//  Game-Thing
//
//  Created by Liam Horch on 2/6/23.
//

#ifndef ball_hpp
#define ball_hpp

#include "GameObject.hpp"


class Ball: public GameObject {
public:
    float radius;
    bool stuck;
    
    Ball();
    Ball(glm::vec2 pos, float radius, glm::vec2 velocity, Texture2D sprite);
    glm::vec2 move(float dt, unsigned int window_width);
    void reset(glm::vec2 position, glm::vec2 velocity);
};

#endif /* ball_hpp */
