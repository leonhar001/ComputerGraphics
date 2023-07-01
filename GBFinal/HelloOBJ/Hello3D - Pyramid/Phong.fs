#version 400

in vec3 scaledNormal;
in vec3 fragmentPos;
in vec4 vertexColor;
in vec2 texCoord;

uniform float ka;
uniform float kd;
uniform float ks;
uniform float n;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 cameraPos;
uniform sampler2D tex_buffer;

out vec4 color;

void main() {

    vec3 ambiente = lightColor * ka; //OK

    //ILUMINA��O DIFUSA
    vec3 N = normalize(scaledNormal); //OK
    vec3 L = normalize(lightPos - fragmentPos); //OK
    float diff = max(dot(N, L), 0.0); //OK

    vec3 diffuse = diff * lightColor * kd; //OK

    //ILUMINA��O ESPECULAR

    vec3 V = normalize(cameraPos - fragmentPos);
	vec3 R = normalize(reflect(-L,N));
	float spec = max(dot(R,V),0.0);
	spec = pow(spec,n);
	vec3 specular = ks * spec * lightColor;

    //maldito final color
    vec3 result = (ambiente + diffuse) + specular;

    // Aplicar a textura
    vec4 texColor = texture(tex_buffer, texCoord);

    // Multiplicar a cor resultante pela cor da textura
   result *= texColor.rgb;

    // Definir a cor final
    color = vec4(result, 1.0f);
}