struct raw_bmp;
struct TextureID;

//defined in billboarding
void billboard_identity();
void billboard_render_at(double x, double y, double z);
void draw2DModeON();
void draw2DModeOFF();

//defined in texture.h
void loadTexture32(const string& file, TextureID& text_tag);