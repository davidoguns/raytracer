/* David Oguns
 * Computer Graphics II
 * April 12, 2008
 * color.c
 * Ray Tracer
 *
 * This file contains the description of all of the functions associated
 * with color operations such as mixing and blending
 */

#include "color.h"

 /* initializes a color structure to 0s on all channels */
color_t* color_init(color_t *color)
{
	color->r = 0.0f;
	color->g = 0.0f;
	color->b = 0.0f;

	return color;
}

/* copy another color structure */
color_t* color_copy(color_t *colorout, const color_t *color)
{
	colorout->r = color->r;
	colorout->g = color->g;
	colorout->b = color->b;

	return colorout;
}

/* clamps color channel values and ensures they are below 1.0f */
color_t* color_clamp(color_t *color)
{
	if(color->r > 1.0f)
		color->r = 1.0f;
	if(color->g > 1.0f)
		color->g = 1.0f;
	if(color->b > 1.0f)
		color->b = 1.0f;

	return color;
}

/* adds two colors by corresponding colors channels 
 * if clamp is 0, color channels will not be clamped 
 * if clamp is 1, color channels will be clamped at 1.0f */
color_t* color_add(color_t *colorout, const color_t *color1,
				   const color_t *color2, unsigned int clamp)
{
	colorout->r = color1->r + color2->r;
	colorout->g = color1->g + color2->g;
	colorout->b = color1->b + color2->b;

	if(clamp)
	{
		if(colorout->r > 1.0f)
			colorout->r = 1.0f;
		if(colorout->g > 1.0f)
			colorout->g = 1.0f;
		if(colorout->b > 1.0f)
			colorout->b = 1.0f;
	}

	return colorout;
}

/* adds all colors in color channel
 * if clamp is 0, color values will not be clamped 
 * if clamp is 1, color values will be clamped at 1.0f */
color_t* color_accumulate(color_t *colorout, const color_t *colors,
						unsigned int ncolors, unsigned int clamp)
{
	unsigned int i = 0;
	for(; i < ncolors; ++i)
	{
		colorout->r += colors[i].r;
		colorout->g += colors[i].g;
		colorout->b += colors[i].b;
	}
	if(clamp)
	{
		if(colorout->r > 1.0f)
			colorout->r = 1.0f;
		if(colorout->g > 1.0f)
			colorout->g = 1.0f;
		if(colorout->b > 1.0f)
			colorout->b = 1.0f;
	}

	return colorout;
}

/* subtracts two colors */
color_t* color_sub(color_t *colorout, const color_t *color1,
						const color_t *color2)
{
	colorout->r = color1->r - color2->r;
	colorout->g = color1->g - color2->g;
	colorout->b = color1->b - color2->b;

	return colorout;
}

/* blends an array of colors by taking the average on all channels */
color_t* color_average(color_t *colorout, color_t *colors, 
					   unsigned int ncolors)
{
	/* first add all colors */
	color_accumulate(colorout, colors, ncolors, 0);
	/* then divide by the number of colors there are */
	colorout->r /= (float)ncolors;
	colorout->g /= (float)ncolors;
	colorout->b /= (float)ncolors;

	return colorout;
}

/* multiples two colors channels together to blend
 * if clamp is 0, color values will not be clamped 
 * if clamp is 1, color values will be clamped at 1.0f */
color_t* color_mult(color_t *colorout, const color_t *color1,
					const color_t *color2, unsigned int clamp)
{
	colorout->r = color1->r * color2->r;
	colorout->g = color1->g * color2->g;
	colorout->b = color1->b * color2->b;

	if(clamp)
	{
		if(colorout->r > 1.0f)
			colorout->r = 1.0f;
		if(colorout->g > 1.0f)
			colorout->g = 1.0f;
		if(colorout->b > 1.0f)
			colorout->b = 1.0f;
	}
	return colorout;
}

/* scales the color by a factor 
 * if clamp is 0, color values will not be clamped 
 * if clamp is 1, color values will be clamped at 1.0f */
color_t* color_scale(color_t *colorout, const color_t *color,
					 float scale, unsigned int clamp)
{
	colorout->r = color->r * scale;
	colorout->g = color->g * scale;
	colorout->b = color->b * scale;
	
	return colorout;
}
