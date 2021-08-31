out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec3 ViewPos;

uniform sampler2D AlbedoTexture;

void main(void) {
    FragColor = texture(AlbedoTexture, TexCoords);
}


#if 0
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
} 
#endif