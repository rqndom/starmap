////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <string>
#include <cmath>
#include <fstream>
#include <sstream>
#include <list>
#include <iostream>

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include <GL/gl.h>
#include <GL/glu.h>

////////////////////////////////////////////////////////////////////////
// Constantes
////////////////////////////////////////////////////////////////////////

const int echelle = 5;
const float rayon = 20.0;

const int screen_width = 1024;
const int screen_height = 768;

// The font file was created by Andrew Tyler www.AndrewTyler.net
// and font@andrewtyler.net
const char* font_path = "font/Minecraftia-Regular.ttf";

const char* data_path = "data/";
const char* config_name = "Config1.txt";

////////////////////////////////////////////////////////////////////////
// Texte
////////////////////////////////////////////////////////////////////////

int next_power_of_two(int number)
{
	int i = 1;
	while (i < number)
		i *= 2;
	return i;
}

struct TextSurface {
	GLuint texture;
	SDL_Surface* surface;
	SDL_Surface* screen;
	int width;
	int height;
	
	TextSurface(TTF_Font* font, const char* text, SDL_Color color,
				SDL_Surface* screen) : screen(screen)
	{
		// Surface ------------------------------------------------------

		// Surface du texte
		SDL_Surface* tmp_surface = TTF_RenderText_Solid(font, text, color);

		// Surface finale
		this->width = next_power_of_two(tmp_surface->w);
		this->height = next_power_of_two(tmp_surface->h);

		this->surface = SDL_CreateRGBSurface(
			0, this->width, this->height, screen->format->BitsPerPixel,
			0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000
		);
		
		// Efface la surface finale
		SDL_FillRect(this->surface, NULL, SDL_MapRGBA(this->surface->format, 0, 0, 0, 0));

		// Insere la surface du texte
		SDL_BlitSurface(tmp_surface, NULL, this->surface, NULL);
		SDL_FreeSurface(tmp_surface);

		// Texture ------------------------------------------------------
	
		glEnable(GL_TEXTURE_2D);

		glGenTextures(1, &this->texture);
		glBindTexture(GL_TEXTURE_2D, this->texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
			this->surface->w, this->surface->h, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, this->surface->pixels);
			
	}

	~TextSurface()
	{
		glDeleteTextures(1, &this->texture);
		SDL_FreeSurface(this->surface);
		this->surface = NULL;
	}

	void render(int x, int y)
	{
		glDisable(GL_DEPTH_TEST);

		// Projection
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		gluOrtho2D(0, this->screen->w, 0, this->screen->h);

		// Texte
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, this->texture);

		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex2f(x, y);
		glTexCoord2f(1.0, 0.0); glVertex2f(x + this->width, y);
		glTexCoord2f(1.0, 1.0); glVertex2f(x + this->width, y - this->height);
		glTexCoord2f(0.0, 1.0); glVertex2f(x, y - this->height);
		glEnd();

		glDisable(GL_TEXTURE_2D);
	}
};

////////////////////////////////////////////////////////////////////////
// Etoiles
////////////////////////////////////////////////////////////////////////

struct Etoile {
	std::string nom;

	float longitude;
	float latitude;
	float dist;

	float x;
	float y;
	float z;

	Uint8 r;
	Uint8 g;
	Uint8 b;

	float magnitude;

	TextSurface* texte;

	Etoile() : texte(NULL)
	{
	}
	~Etoile()
	{
		if (this->texte)
			delete this->texte;
	}
	
	void set_color(int r, int g, int b)
	{
		this->r = r;
		this->g = g;
		this->b = b;
	}
	void set_text(TTF_Font* font, std::string nom, SDL_Color color, SDL_Surface* screen)
	{
		this->nom = nom;
		
		if (this->texte)
			delete this->texte;
		this->texte = new TextSurface(font, nom.c_str(), color, screen);
	}
	void set_coords(float longitude, float latitude, float dist)
	{
		this->longitude = longitude;
		this->latitude = latitude;
		this->dist = dist;

		this->x = cos(longitude) * cos(latitude) * dist;
		this->y = sin(latitude) * dist;
		this->z = sin(longitude) * cos(latitude) * dist;

	}
};

////////////////////////////////////////////////////////////////////////
// Lecture
////////////////////////////////////////////////////////////////////////

std::string parse_string(std::fstream& file)
{
	std::string s;
	file >> s;
	return s;
}

int parse_int(std::fstream& file)
{
	std::string s;
	file >> s;
	
	std::istringstream conv(s);
	int n;
	conv >> n;
	
	return n;
}

float to_float(std::string str)
{
	float value;
	std::istringstream conv(str);
	conv >> std::ws >> value;
	return value;
}

std::list<Etoile*> parse_file(TTF_Font* font, SDL_Surface* screen)
{
	// Config .......................................................
	
	std::fstream config(std::string(data_path) + config_name);

	// Extrait les donnees
	std::string file_name = std::string(data_path) + parse_string(config);

	int nbre_etoiles = parse_int(config);
	
	int nom_pos = parse_int(config);
	int nom_taille = parse_int(config);

	int long_pos = parse_int(config);
	int long_taille = parse_int(config);

	int lat_pos = parse_int(config);
	int lat_taille = parse_int(config);
	
	int dist_pos = parse_int(config);
	int dist_taille = parse_int(config);
	
	int mag_pos = parse_int(config);
	int mag_taille = parse_int(config);
	
	int type_pos = parse_int(config);
	int type_taille = parse_int(config);
	
	// Liste ........................................................

	std::fstream data(file_name.c_str());
	
	std::list<Etoile*> etoiles;

	std::string line;
	SDL_Color cyan = {0, 255, 255};
		
	for (int i = 0; i < nbre_etoiles; i++) {
		std::getline(data, line);
		
		Etoile* etoile = new Etoile();
		etoiles.push_back(etoile);

		// Extrait les donnees
		etoile->set_text(font, line.substr(nom_pos, nom_taille), cyan, screen);

		etoile->set_coords(
			to_float(line.substr(long_pos, long_taille)) * M_PI / 180,
			to_float(line.substr(lat_pos, lat_taille)) * M_PI / 180,
			to_float(line.substr(dist_pos, dist_taille))
		);

		etoile->magnitude = to_float(line.substr(mag_pos, mag_taille));

		char type_spectral = line.substr(type_pos, type_taille)[0];
		switch (type_spectral)
		{
			case 'O': etoile->set_color(155, 176, 255); break;
			case 'B': etoile->set_color(202, 215, 255); break;
			case 'A': etoile->set_color(255, 255, 255); break;
			case 'F': etoile->set_color(255, 255, 204); break;
			case 'G': etoile->set_color(255, 255, 0); break;
			case 'K': etoile->set_color(255, 128, 0); break;
			case 'M': etoile->set_color(255, 0, 0); break;
			case 'D': etoile->set_color(192, 192, 192); break;
			case 'L': etoile->set_color(128, 0, 0); break;
			case 'T': etoile->set_color(128, 0, 0); break;
			default: etoile->set_color(0, 255, 0); break;
		}
	}

	return etoiles;
}

////////////////////////////////////////////////////////////////////////
// Rendu
////////////////////////////////////////////////////////////////////////

void render_text(GLdouble ex, GLdouble ey, GLdouble ez,
				GLdouble* view, GLdouble* proj, GLint* port,
				TextSurface* text)
{
	GLdouble tx, ty, tz;
	gluProject(ex, ey, ez, view, proj, port, &tx, &ty, &tz);

	GLdouble eyez = ex * proj[2] + ey * proj[6] + ez * proj[10] + proj[14];
	if (eyez > 0)
		text->render((int)tx, (int)ty);
}

void render_scene(float x, float y, float z, std::list<Etoile*> etoiles,
				TextSurface* angles, SDL_Surface* screen)
{
	// Initialisation -----------------------------------------------

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

	// Perspective --------------------------------------------------

	glViewport(0, 0, screen->w, screen->h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	//glOrtho(-26.7, 26.7, -20, 20, 1, 1000);
	gluPerspective(45, (screen->w * 1.0f / screen->h), 0.1f, 100000.0f);

	gluLookAt(x, y, z,
			  0.0f, 0.0f, 0.0f,
			  0.0f, 1.0f, 0.0f);

	// Objets -------------------------------------------------------

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Lignes
	glColor3ub(0, 0, 255);

	for (int j = 1; j <= rayon / echelle; j++) {
		glBegin(GL_LINE_STRIP);
		int n = 100;
		for (int i = 0; i <= n; i++)
			glVertex3f(
				cos(i * 2.0 / n * M_PI) * j * echelle,
				0,
				sin(i * 2.0 / n * M_PI) * j * echelle
			);
		glEnd();
	}

	glBegin(GL_LINES);
	int n = 8;
	for (int i = 0; i < n; i++) {
		glVertex3f(cos(i * 2.0 / n * M_PI) * rayon, 0, sin(i * 2.0 / n * M_PI) * rayon);
		glVertex3f(-cos(i * 2.0 / n * M_PI) * rayon, 0, -sin(i * 2.0 / n * M_PI) * rayon);
	}
	glEnd();


	glColor3ub(0, 170, 175);
	glEnable(GL_LINE_STIPPLE);
	glLineStipple(1, 0x1111);

	glBegin(GL_LINES);

	for (std::list<Etoile*>::iterator it = etoiles.begin(); it != etoiles.end(); ++it) {
		Etoile* etoile = *it;
		glVertex3f(etoile->x, 0, -etoile->z);
		glVertex3f(etoile->x, etoile->y, -etoile->z);
	}

	glEnd();

	glDisable(GL_LINE_STIPPLE);

	// Points
	glClear(GL_DEPTH_BUFFER_BIT);

	for (std::list<Etoile*>::iterator it = etoiles.begin(); it != etoiles.end(); ++it) {
		Etoile* etoile = *it;

		glColor3ub(etoile->r, etoile->g, etoile->b);
		glPointSize(5 * exp(-etoile->magnitude * 0.02));

		glBegin(GL_POINTS);
		glVertex3f(etoile->x, etoile->y, -etoile->z);
		glEnd();
	}

	// Texte --------------------------------------------------------

	GLdouble view[16];
	GLdouble proj[16];
	GLint port[4];

	glGetDoublev(GL_MODELVIEW_MATRIX, view);
	glGetDoublev(GL_PROJECTION_MATRIX, proj);
	glGetIntegerv(GL_VIEWPORT, port);

	for (std::list<Etoile*>::iterator it = etoiles.begin(); it != etoiles.end(); ++it) {
		Etoile* etoile = *it;
		render_text(etoile->x, etoile->y, -etoile->z, view, proj, port, etoile->texte);
	}

	for (int i = 0; i < 4; i++)
		render_text(
			cos(i * M_PI / 2.0) * rayon,
			0,
			-sin(i * M_PI / 2.0) * rayon,
			view, proj, port,
			&angles[i]
		);
}

////////////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////////////

int main(int, char**)
{
	// Initialisation SDL -------------------------------------------

	// Video
	SDL_Init(SDL_INIT_VIDEO);
	SDL_WM_SetCaption("Carte Stellaire", NULL);

	SDL_Surface* screen = SDL_SetVideoMode(screen_width, screen_height,
										32, SDL_OPENGL | SDL_DOUBLEBUF);

	//~glClear(GL_ACCUM_BUFFER_BIT);

	// Texte
	TTF_Init();
	TTF_Font* font = TTF_OpenFont(font_path, 8);
	if (!font) {
		std::cout << "error: cannot load font " << font_path << std::endl;
		return 1;
	}

	// General
	SDL_ShowCursor(false);

	// Variables ----------------------------------------------------

	SDL_Color bleu = {0, 0, 255};

	TextSurface angles[4] = {
		{font, "0°", bleu, screen},
		{font, "90°", bleu, screen},
		{font, "180°", bleu, screen},
		{font, "270°", bleu, screen},
	};
	
	int mouse_x(320), mouse_y(240), mouse_z(0);

	std::list<Etoile*> etoiles = parse_file(font, screen);

	// Boucle Principale --------------------------------------------

	bool continuer = true;
	SDL_Event event;

	while (continuer) {
		// Evenements ...............................................

		while(SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					continuer = false;
					break;

				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_ESCAPE)
						continuer = false;
					break;

				case SDL_MOUSEMOTION:
					mouse_x = event.motion.x;
					mouse_y = event.motion.y;
					break;

				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == SDL_BUTTON_WHEELDOWN)
						mouse_z++;
					if (event.button.button == SDL_BUTTON_WHEELUP)
						mouse_z--;
					break;
			}
		}

		// Calculs ..................................................

		float angle_x = (mouse_y - 0.5 * screen->h) * 0.01;
		float angle_y = (mouse_x - 0.5 * screen->w) * 0.01;

		float distance = 3 * rayon * exp(mouse_z * 0.1);

		if (angle_x > 0.5 * M_PI) angle_x = 0.5 * M_PI;
		if (angle_x < -0.5 * M_PI) angle_x = -0.5 * M_PI;

		float px = cos(angle_y) * cos(angle_x) * distance;
		float py = sin(angle_x) * distance;
		float pz = sin(angle_y) * cos(angle_x) * distance;

		// Rendu ....................................................

		render_scene(px, py, pz, etoiles, angles, screen);

		// Motion Blur
		//~float q = 0.7;
		//~glAccum(GL_MULT, q);
		//~glAccum(GL_ACCUM, 1 - q);
		//~glAccum(GL_RETURN, 1.0);

		glFlush();
		SDL_GL_SwapBuffers();
	}

	// Fermeture ----------------------------------------------------

	while (!etoiles.empty()) {
		delete etoiles.front();
		etoiles.pop_front();
	}

	TTF_CloseFont(font);
	TTF_Quit();

	SDL_Quit();

	return 0;
}
