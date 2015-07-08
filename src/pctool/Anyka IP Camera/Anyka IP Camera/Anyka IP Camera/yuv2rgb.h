#pragma once

#include "stdafx.h"

void init_dither_tab();

void init_s_tab();

void YUV2RGB420(unsigned char *src0, int stride_y, unsigned char *src1, unsigned char *src2, int stride_uv, 
				unsigned char *dst_ori, int width, int height);

void YUV_TO_RGB32(uint8_t *puc_y, int stride_y, uint8_t *puc_u,uint8_t *puc_v, int stride_uv,    
                  uint8_t *puc_out, int width_y, int height_y, int stride_out);

void YUV2RGB420_s(unsigned char *src0, int stride_y, unsigned char *src1, unsigned char *src2, int stride_uv, 
				DWORD *dst_ori, int width, int height);

void YUV_TO_RGB32_s(uint8_t *puc_y, int stride_y,    
                  uint8_t *puc_u, uint8_t *puc_v, int stride_uv,    
                  uint8_t *puc_out, int width_y, int height_y, int stride_out);