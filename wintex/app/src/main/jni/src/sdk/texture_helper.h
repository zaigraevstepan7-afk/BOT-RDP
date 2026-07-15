#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/*struct TextureInfo { 
    ImTextureID textureId; 
    int w; 
    int h; 
};

TextureInfo CreateTextureFromFile(const char* filePath) {
    TextureInfo image;
    int n;
    unsigned char* data = stbi_load(filePath, &image.w, &image.h, &n, 0);
    
    if (data == NULL) {
        // Файл не найден или не удалось загрузить
        image.textureId = nullptr;
        image.w = 0;
        image.h = 0;
        return image;
    }
    
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    GLenum format = (n == 3) ? GL_RGB : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, format, image.w, image.h, 0, format, GL_UNSIGNED_BYTE, data);
    
    stbi_image_free(data);
    
    image.textureId = (ImTextureID)(uintptr_t)texture;
    return image;
}*/

struct TextureInfo { 
    ImTextureID textureId; 
    int w; 
    int h; 
};

TextureInfo CreateTextureFromFile(const char* filePath) {
    TextureInfo image;
    int n;
    unsigned char* data = stbi_load(filePath, &image.w, &image.h, &n, 4);
    
    if (data == NULL) {
        image.textureId = nullptr;
        image.w = 0;
        image.h = 0;
        return image;
    }
    
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.w, image.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    stbi_image_free(data);
    
    image.textureId = (ImTextureID)(uintptr_t)texture;
    return image;
}