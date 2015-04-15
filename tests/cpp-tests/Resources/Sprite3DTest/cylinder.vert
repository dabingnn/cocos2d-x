
uniform float offset;
varying vec2 v_texture_coord;
void main(void)
{
    gl_Position = CC_MVPMatrix * a_position;
    v_texture_coord = a_texCoord.xy;
    v_texture_coord.y = (1.0 - v_texture_coord.y);
}
