/* David Oguns
 * Computer Graphics II
 * Ray Tracer
 * April 10, 2008
 * color.h
 *
 * This file contains the definition for the color data structure and
 * related functions to make working with colors easier */

#ifndef _COLOR_H_
#define _COLOR_H_


/* color values - most materials use this to specify how
 * much light is reflected at given wavelengths
 */
typedef struct
{
	float r;
	float g;
	float b;
} color_t;


/* initializes a color structure to 0s on all channels */
color_t* color_init(color_t *color);

/* initializes a color to a specific value */
color_t* color_set(color_t *colorout, float r, float g, float b);

/* copy another color structure */
color_t* color_copy(color_t *colorout, const color_t *color);

/* clamps color channel values and ensures they are below 1.0f */
color_t* color_clamp(color_t *color);

/* adds two colors by corresponding colors channels 
 * if clamp is 0, color channels will not be clamped 
 * if clamp is 1, color channels will be clamped at 1.0f */
color_t* color_add(color_t *colorout, const color_t *color1,
				   const color_t *color2, unsigned int clamp);

/* adds an array of colors by corresponding color channels
 * if clamp is 0, color channels will not be clamped 
 * if clamp is 1, color channels will be clamped at 1.0f */
color_t* color_accumulate(color_t *colorout, const color_t *colors,
						unsigned int ncolors, unsigned int clamp);

/* subtracts two colors */
color_t* color_sub(color_t *colorout, const color_t *color1,
						const color_t *color2);

/* scales the color by a factor 
 * if clamp is 0, color values will not be clamped 
 * if clamp is 1, color values will be clamped at 1.0f */
color_t* color_scale(color_t *colorout, const color_t *color,
					 float scale, unsigned int clamp);

/* multiples two colors channels together to blend
 * if clamp is 0, color values will not be clamped 
 * if clamp is 1, color values will be clamped at 1.0f */
color_t* color_mult(color_t *colorout, const color_t *color1,
					const color_t *color2, unsigned int clamp);

/* blends an array of colors by taking the average on all channels */
color_t* color_average(color_t *colorout, color_t *colors, 
					   unsigned int ncolors);

/**********************************************************
 * 
 *	Tone reproduction related functions
 *
 * ********************************************************/

/* return luminance of color */
float get_luminance(const color_t *color);



#endif

