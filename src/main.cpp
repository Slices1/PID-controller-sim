	
#include <iostream>
#include <vector>
#include <time.h>
#include <limits> // for integer limits to convert integer angle to radians
#include <cmath> // for M_PI and trig

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

using namespace std;

class Vec2 {
	public: 
		float x, y;
	
	Vec2(float a = 0.0, float b = 0.0) {
		x = a;
		y = b;
	}
	
	// Operator overloads
		// Scalar Vector product using operator*
		Vec2 operator*(float other) {
			return Vec2(x * other, y * other);
		}

		// Dot product using operator*
		float operator*(const Vec2& other) const {
			return (x * other.x) + (y * other.y);
		}

		// Vec2 addition using operator+
		Vec2 operator+(const Vec2& other) const {
			return Vec2(x + other.x, y + other.y);
		}

		// Vec2 += addition using operator+=
		Vec2& operator+=(const Vec2& other) {
			x += other.x;
			y += other.y;
			return *this;
		}

		// Vec2 subtraction using operator-
		Vec2 operator-(const Vec2& other) const {
			return Vec2(x - other.x, y - other.y);
		}

		// Vec2 unary subtraction using operator-
		Vec2 operator-() const {
			return Vec2(-x, -y);
		}

		// Scalar Vector quotient using operator/
		Vec2 operator/(float other) {
			return Vec2(x / other, y / other);
		}

	float magnitude_squared() {
		return x*x + y*y;
	}
};

// Gravity in pixels per second squared
const float gravity = 9.81f; // 1 metre will be 10 pixels?

void DrawCircle(SDL_Renderer *renderer, int32_t centreX, int32_t centreY, int32_t radius) {
	const int32_t diameter = (radius * 2);

	int32_t x = (radius - 1);
	int32_t y = 0;
	int32_t tx = 1;
	int32_t ty = 1;
	int32_t error = (tx - diameter);

	while (x >= y) {
		//  Each of the following renders an octant of the circle
		SDL_RenderDrawPoint(renderer, centreX + x, centreY - y);
		SDL_RenderDrawPoint(renderer, centreX + x, centreY + y);
		SDL_RenderDrawPoint(renderer, centreX - x, centreY - y);
		SDL_RenderDrawPoint(renderer, centreX - x, centreY + y);
		SDL_RenderDrawPoint(renderer, centreX + y, centreY - x);
		SDL_RenderDrawPoint(renderer, centreX + y, centreY + x);
		SDL_RenderDrawPoint(renderer, centreX - y, centreY - x);
		SDL_RenderDrawPoint(renderer, centreX - y, centreY + x);

		if (error <= 0) {
			++y;
			error += ty;
			ty += 2;
		}

		if (error > 0) {
			--x;
			tx += 2;
			error += (tx - diameter);
		}
	}
}

float getSensorValueAtPoint(const float &displacementSquared) {
	return 100/(sqrt(displacementSquared) + 100); // prop to 1/r
}

class PIDController {
	public:
		float p, i, d;
		float integral, lastError;

	PIDController(float p, float i, float d) {
		this->p = p;
		this->i = i;
		this->d = d;
		integral = 0;
		lastError = 0;
	}

	float update(float error, float dT) {
		integral += error * dT;
		float derivative = (error - lastError) / dT;
		lastError = error;
		return p*error + i*integral + d*derivative;
	}
};

float addNoiseToSensorValue(float value) {
	return value + (rand() % 100 - 50) / 10000.0f; // 0.5% noise
}


// Forward declerations
bool init();
void kill();
void renderText(string text, SDL_Rect dest);

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* box;
TTF_Font* font;

int main(int argc, char** args) {

	if ( !init() ) {
		system("pause");
		return 1;
	}
	
	srand(time(NULL));
	bool running = true;
	Uint32 lastUpdate = 0;
	PIDController xPID = PIDController(0.25, 0.1, 0.1);
	PIDController yPID = PIDController(0.25, 0.1, 0.1);
	int mouseX; int mouseY;
	Vec2 sensorArrayPos = Vec2(1080, 720)/2;
	Vec2 sensorArrayVel;
	int sensorOffset = 20;
	float errorX, errorY;
	SDL_Color diff;
	const int NUM_COLORS = 5;
	const SDL_Color color[NUM_COLORS] = {
		{0, 0, 0}, // black
		{0, 0, 255}, // blue
		{0, 255, 255}, // cyan
		{0, 255, 0}, // green
		{255, 0, 0} // red
	};
	// A static array of 4 colors:  (black, blue, cyan, green, red)
	float colorBounds[NUM_COLORS] = {0.1, 0.2, 0.4, 0.7, 1.0};
	float valueDiff;
	float displacementSquared;
	float value;


	while(running) {
		SDL_Event e;
		SDL_SetRenderDrawColor( renderer, 50, 50, 50, 255 );
		SDL_RenderClear( renderer );

		// Event loop
		while ( SDL_PollEvent( &e ) != 0 ) {
			switch (e.type) {
				case SDL_QUIT:
					running = false;
					break;
			}
			switch(e.key.keysym.sym) {
			    case SDLK_RIGHT: 
			        cout << "Right was pressed" << endl;
			        break;
			    case SDLK_LEFT:
			        cout << "Left was pressed" << endl;
			        break;
			    // case SDLK_UP:
			    //     cout << "Up was pressed" << endl;
			    //     break;
			    // case SDLK_DOWN:
			    //     cout << "Down was pressed" << endl;
			    //     break;	
			}

			// detect input for PID constants
			if (e.type == SDL_MOUSEBUTTONDOWN) {
				cout << "Mouse button was pressed" << endl;
				cout << "Mouse X: " << e.button.x << " Mouse Y: " << e.button.y << endl;
				if (e.button.x > 681) {

					if (e.button.y < 40+30) {
						xPID.p -= 0.01, yPID.p -= 0.01;
					} else if (e.button.y < 70+30) {
						xPID.i -= 0.01, yPID.i -= 0.01;
					} else if (e.button.y < 100+30) {
						xPID.d -= 0.01, yPID.d -= 0.01;
					}

				} else if (e.button.x > 660) {

					if (e.button.y < 40+30) {
						xPID.p += 0.01, yPID.p += 0.01;
					} else if (e.button.y < 70+30) {
						xPID.i += 0.01, yPID.i += 0.01;
					} else if (e.button.y < 100+30) {
						xPID.d += 0.01, yPID.d += 0.01;
					}

				}
			}
		}

		// Physics loop
			Uint32 time = SDL_GetTicks();
			float dT = (time - lastUpdate) / 1000.0f;
			cout << "fps: " << 1/(dT) << endl;
			lastUpdate = time;

			SDL_GetMouseState(&mouseX, &mouseY);

			sensorArrayVel.x += xPID.update(errorX, dT);
			sensorArrayPos.x += sensorArrayVel.x * dT;

			sensorArrayVel.y += yPID.update(errorY, dT);
			sensorArrayPos.y += sensorArrayVel.y * dT;

			{ // get sensor values and errors
				float sensorValues[4];
				int index = 0;
				for (int i=-1; i<=1; i+=2) {
					// goes from top, clockwise
					sensorValues[index] = getSensorValueAtPoint(
						(mouseX-sensorArrayPos.x)*(mouseX-sensorArrayPos.x)
						+ (mouseY - (sensorArrayPos.y + i*sensorOffset))*(mouseY - (sensorArrayPos.y + i*sensorOffset)));
					index++;
					sensorValues[i+2] = getSensorValueAtPoint(
						(mouseX - (sensorArrayPos.x - i*sensorOffset))*(mouseX - (sensorArrayPos.x - i*sensorOffset))
						+ (mouseY - sensorArrayPos.y)*(mouseY - sensorArrayPos.y));
					index++;
				}
				errorY = 200*addNoiseToSensorValue(sensorValues[2] - sensorValues[0]);
				errorX = -200*addNoiseToSensorValue(sensorValues[3] - sensorValues[1]);
			}
			
		// Render loop
			// render heat map
				for (int i=0; i<1080; i+=5) {
					for (int j=0; j<720; j+=5) {
						displacementSquared = (mouseX-i)*(mouseX-i) + (mouseY-j)*(mouseY-j);
						if (displacementSquared < 240000) {
							value = getSensorValueAtPoint(displacementSquared);
							// get colour							
							{
								// Find the correct color bounds
								int k = 0;
								while (k < NUM_COLORS - 1 && value > colorBounds[k+1]) {
									k++;
								}

								// Calculate the correct color
								valueDiff = (value - colorBounds[k]) / (colorBounds[k+1] - colorBounds[k]);
								diff = {
									(Uint8)((color[k+1].r - color[k].r) * valueDiff + color[k].r),
									(Uint8)((color[k+1].g - color[k].g) * valueDiff + color[k].g),
									(Uint8)((color[k+1].b - color[k].b) * valueDiff + color[k].b)
								};
							}
							SDL_SetRenderDrawColor(renderer, diff.r, diff.g, diff.b, 255);
							SDL_RenderDrawPoint(renderer, i, j);
						}
					}
				}
			// render grid background
				SDL_SetRenderDrawColor(renderer, 110, 110, 110, 255);
				for (int i=0; i<1080; i+=100) {
					SDL_RenderDrawLine(renderer, i, 0, i, 720);
				}
				for (int i=0; i<720; i+=100) {
					SDL_RenderDrawLine(renderer, 0, i, 1080, i);
				}
				SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
			// render sensor array
				for (int i=-1; i<=1; i+=2) {
						DrawCircle(renderer, sensorArrayPos.x + i*sensorOffset, sensorArrayPos.y, 7);
						DrawCircle(renderer, sensorArrayPos.x, sensorArrayPos.y + i*sensorOffset, 7);
				}
			// render label in top left
				renderText("Mouse X: " + to_string(mouseX), {10, 10});
				renderText("Mouse Y: " + to_string(mouseY), {10, 40});
				renderText("Sensor X: " + to_string(sensorArrayPos.x), {10, 70});
				renderText("Sensor Y: " + to_string(sensorArrayPos.y), {10, 100});
				renderText("Velocity X: " + to_string(sensorArrayVel.x), {10, 130});
				renderText("Velocity Y: " + to_string(sensorArrayVel.y), {10, 160});
				renderText("Error X: " + to_string(errorX), {10, 190});
				renderText("Error Y: " + to_string(errorY), {10, 220});
				renderText("Integral X: " + to_string(xPID.integral), {10, 250});
				renderText("Integral Y: " + to_string(yPID.integral), {10, 280});
				renderText("Derivative X: " + to_string((errorX - xPID.lastError) / dT), {10, 310});
				renderText("Derivative Y: " + to_string((errorY - yPID.lastError) / dT), {10, 340});

				renderText("(Click to change these)", {1080-350, 10});
				renderText("^ \\/ k_proportional: " + to_string(xPID.p), {1080-420, 40});
				renderText("^ \\/ k_integral: " + to_string(xPID.i), {1080-420, 70});
				renderText("^ \\/ k_derivative: " + to_string(xPID.d), {1080-420, 100});


		// Display window + delay
			// SDL_Delay(15);		
			SDL_RenderPresent(renderer);
	}
	
	kill();
	return 0;
}

void renderText(string text, SDL_Rect dest) {
	SDL_Color fg = { 175, 175, 175 };
	SDL_Surface* surf = TTF_RenderText_Solid(font, text.c_str(), fg);

	dest.w = surf->w;
	dest.h = surf->h;

	SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);

	SDL_RenderCopy(renderer, tex, NULL, &dest);
	SDL_DestroyTexture(tex);
	SDL_FreeSurface(surf);
}

bool init() {
	if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) {
		cout << "Error initializing SDL: " << SDL_GetError() << endl;
		return false;
	} 

	if ( IMG_Init(IMG_INIT_JPG) < 0 ) {
		cout << "Error initializing SDL_image: " << IMG_GetError() << endl;
		return false;
	}

	if ( TTF_Init() < 0 ) {
		cout << "Error initializing SDL_ttf: " << TTF_GetError() << endl;
		return false;
	}

	window = SDL_CreateWindow( "PID Controller", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1080, 720, SDL_WINDOW_SHOWN);
	if ( !window ) {
		cout << "Error creating window: " << SDL_GetError()  << endl;
		return false;
	}

	renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
	if ( !renderer ) {
		cout << "Error creating renderer: " << SDL_GetError() << endl;
		return false;
	}

	SDL_Surface* buffer = IMG_Load("../box.jpg");
	if ( !buffer ) {
		cout << "Error loading image box.jpg: " << SDL_GetError() << endl;
		return false;
	}

	box = SDL_CreateTextureFromSurface( renderer, buffer );
	SDL_FreeSurface( buffer );
	buffer = NULL;
	if ( !box ) {
		cout << "Error creating texture: " << SDL_GetError() << endl;
		return false;
	}

	font = TTF_OpenFont("../font.ttf", 24);
	if ( !font ) {
		cout << "Error loading font: " << TTF_GetError() << endl;
		return false;
	}

	return true;
}

void kill() {
	TTF_CloseFont( font );
	SDL_DestroyTexture( box );
	font = NULL;
	box = NULL;

	SDL_DestroyRenderer( renderer );
	SDL_DestroyWindow( window );
	window = NULL;
	renderer = NULL;

	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}


// Other useful SDL functions for small projects:
//SDL_RenderDrawLine(renderer, 0, 0, 640, 480);
//SDL_RenderDrawPoint(renderer, 320, 240); 
//SDL_RenderFillRect(renderer, &rect);
//SDL_RenderCopy(renderer, box, NULL, &rect);
// int x, y; SDL_GetMouseState(&x, &y);
