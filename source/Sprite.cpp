/* Sprite.cpp
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "Sprite.h"

#include "ImageBuffer.h"
#include "Preferences.h"
#include "Screen.h"

#include "opengl.h"
#include <SDL2/SDL.h>

#include <algorithm>

using namespace std;



Sprite::Sprite(const string &name)
	: name(name)
{
}



const string &Sprite::Name() const
{
	return name;
}



// Upload the given frames. The given buffer will be cleared afterwards.
void Sprite::AddFrames(ImageBuffer &buffer, bool is2x)
{
	// Do nothing if the buffer is empty.
	if(!buffer.Pixels())
		return;
	
	// If this is the 1x image, its dimensions determine the sprite's size.
	if(!is2x)
	{
		width = buffer.Width();
		height = buffer.Height();
		frames = buffer.Frames();
	}
	
	// Check whether this sprite is large enough to require size reduction.
	if(Preferences::Has("Reduce large graphics") && buffer.Width() * buffer.Height() >= 1000000)
		buffer.ShrinkToHalfSize();
	
	// Upload the images as a single array texture.
	glGenTextures(1, &texture[is2x]);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture[is2x]);
	
	// Use linear interpolation and no wrapping.
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	// Upload the image data.
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, // target, mipmap level, internal format,
		buffer.Width(), buffer.Height(), buffer.Frames(), // width, height, depth,
		0, GL_RGBA, GL_UNSIGNED_BYTE, buffer.Pixels()); // border, input format, data type, data.
	
	// Unbind the texture.
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	
	// Free the ImageBuffer memory.
	buffer.Clear();
}



// Free up all textures loaded for this sprite.
void Sprite::Unload()
{
	glDeleteTextures(2, texture);
	texture[0] = texture[1] = 0;
	
	width = 0.f;
	height = 0.f;
	frames = 0;
}



// Get the width, in pixels, of the 1x image.
float Sprite::Width() const
{
	return width;
}



// Get the height, in pixels, of the 1x image.
float Sprite::Height() const
{
	return height;
}



// Get the number of frames in the animation.
int Sprite::Frames() const
{
	return frames;
}



// Get the offset of the center from the top left corner; this is for easy
// shifting of corner to center coordinates.
Point Sprite::Center() const
{
	return Point(.5 * width, .5 * height);
}



// Get the texture index, based on whether the screen is high DPI or not.
uint32_t Sprite::Texture() const
{
	return Texture(Screen::IsHighResolution());
}



// Get the index of the texture for the given high DPI mode.
uint32_t Sprite::Texture(bool isHighDPI) const
{
	return (isHighDPI && texture[1]) ? texture[1] : texture[0];
}
