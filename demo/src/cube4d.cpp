#include <utils/converters.h>


#include <chrono>

// Some compilers (msvc...) don't include the math constants (e.g. PI) unless this macro is defined
#define _USE_MATH_DEFINES
#include <cmath>

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <thread>

#include <app.h>
#include <linalg.h>
#include <utils.h>

const nr_uint dim = 4;

Vector h_cube[]
{
    Vector(-1, -1, -1, -1, 1),
    Vector(-1, -1, -1,  1, 1),
    Vector(-1, -1,  1, -1, 1),
    Vector(-1, -1,  1,  1, 1),
    Vector(-1,  1, -1, -1, 1),
    Vector(-1,  1, -1,  1, 1),
    Vector(-1,  1,  1, -1, 1),
    Vector(-1,  1,  1,  1, 1),
    Vector( 1, -1, -1, -1, 1),
    Vector( 1, -1, -1,  1, 1),
    Vector( 1, -1,  1, -1, 1),
    Vector( 1, -1,  1,  1, 1),
    Vector( 1,  1, -1, -1, 1),
    Vector( 1,  1, -1,  1, 1),
    Vector( 1,  1,  1, -1, 1),
    Vector( 1,  1,  1,  1, 1),
};

nr_float h_near[]
{
    -5, -5, 0.5, 0.5
};

nr_float h_far[]
{
    5, 5, 10, 10
};

void transform(Tetrahedron simplexes[48], const nr_float angle)
{
    Matrix r = Matrix::rotation(Y, Z, angle);
    Matrix t = Matrix::translation(1.5, 1.5, 2, 2);
    
    Matrix op = t * r;

    Vector cube[16];
    for (auto i = 0; i < 16; ++i)
    {
		cube[i] = op * h_cube[i];
    }

	cube4dToSimplices(cube, simplexes);
}

class Cube4dApp : public App
{
public:
	Cube4dApp()
		: App("Cube4d", nr::ScreenDimension{ 640, 480 }, dim, 48)
	{
		setNearPlane(h_near);
		setFarPlane(h_far);
	}

protected:
	void update() override
	{
		transform(reinterpret_cast<Tetrahedron*>(getHostSimplexes<dim>()), M_PI / 20);
	}
};

int main(const int argc, const char* argv[])
{
	if (!App::init()) return EXIT_FAILURE;

	auto app = Cube4dApp();
	app.run();

	App::deinit();
}