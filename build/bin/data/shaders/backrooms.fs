#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform vec3 objectColor;
uniform vec3 viewPos;
uniform float ambientStrength;

struct Spotlight {
  vec3 position;
  vec3 direction;
  float cutOff;
  float outerCutOff;
  vec3 color;
  float intensity;
};

uniform Spotlight spotlight;

void main() {
  vec3 color = texture(texture1, TexCoord).rgb * objectColor;
  vec3 normal = normalize(Normal);

  vec3 ambient = ambientStrength * vec3(0.9, 0.9, 0.7);

  vec3 spotlightResult = vec3(0.0);
  if (spotlight.intensity > 0.0) {
    vec3 lightDir = normalize(spotlight.position - FragPos);
    float theta = dot(lightDir, normalize(-spotlight.direction));
    float epsilon = spotlight.cutOff - spotlight.outerCutOff;
    float intensity = clamp((theta - spotlight.outerCutOff) / epsilon, 0.0, 1.0);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * spotlight.color;

    float distance = length(spotlight.position - FragPos);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));

    spotlightResult = spotlight.intensity * intensity * attenuation * diffuse;
  }

  float distance = length(viewPos - FragPos);
  float fogFactor = exp(-distance * 0.02);
  fogFactor = clamp(fogFactor, 0.1, 1.0);

  vec3 result = ambient + spotlightResult;
  result = result * color * fogFactor;

  result.r *= 1.1;
  result.g *= 1.05;

  FragColor = vec4(result, 1.0);
}
