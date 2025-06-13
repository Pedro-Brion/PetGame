#version 330 core
out vec4 FragColor;

in vec2 TextCoord;

uniform sampler2D spriteTexture;
uniform vec3 spriteColor;

void main()
{
   //FragColor = texture(spriteTexture, TextCoord) * vec4(spriteColor, 1.0);
   vec4 texColor = texture(spriteTexture, TextCoord);
   if(texColor.a < 0.1) discard;

    FragColor=texColor * vec4(spriteColor,1.f);
}