// Copyright (c) 2020 Rumen G.Bogdanovski
// All rights reserved.
//
// You can use this software under the terms of 'INDIGO Astronomy
// open-source license' (see LICENSE.md).
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS 'AS IS' AND ANY EXPRESS
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef _IMAGE_PREVIEW_LUT_H
#define _IMAGE_PREVIEW_LUT_H

typedef enum {
	STRETCH_NONE = 0,
	STRETCH_NORMAL = 1,
	STRETCH_HARD = 2,
} preview_stretch;

typedef enum {
	COLOR_BALANCE_AUTO = 0,
	COLOR_BALANCE_NONE,
	COLOR_BALANCE_COUNT
} color_balance_t;

typedef struct {
	double clip_black;
	double clip_white;
} preview_stretch_t;

typedef struct {
	uint8_t stretch_level;
	uint8_t balance; /* 0 = AWB, 1 = red, 2 = green, 3 = blue; */
} stretch_config_t;

typedef struct {
	float brightness;
	float contrast;
} stretch_input_params_t;

static const stretch_input_params_t stretch_params_lut[] ={
	{0   ,    0}, // Does not matter
	{0.25, -2.8},
	{0.40, -2.5}
};

#endif /* _IMAGE_PREVIEW_LUT_H */
