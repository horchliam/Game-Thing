//
//  sprite_renderer.hpp
//  Game-Thing
//
//  Created by Liam Horch on 2/4/23.
//

#ifndef sprite_renderer_hpp
#define sprite_renderer_hpp

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "texture.h"
#include "shader.h"

class SpriteRenderer {
public:
    SpriteRenderer(Shader &shader);
    ~SpriteRenderer();
    
    void DrawSprite(Texture2D &texture, glm::vec2 position, glm::vec2 size = glm::vec2(10.0f, 10.0f),
                    float rotate = 0, glm::vec3 color = glm::vec3(1.0f));
private:
    Shader shader;
    unsigned int quadVAO;
    void initRenderData();
};

#endif /* sprite_renderer_hpp */
