#include "gusanos/material.h"

Material::Material()
{
	worm_pass = true;
	particle_pass = true;
	flows = false;
	can_breath = true;
	destroyable = false;
	draw_exps = true;
	blocks_light = false;
	destroys_water = false;
	creates_water = false;
	damage = 0;
	is_stagnated_water = false;
	
	index = 0; // Should I not initialize this?
}

Material::~Material()
{
}
