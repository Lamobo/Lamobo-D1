#include "stdafx.h"
#include <windows.h>   
   
long int crv_tab[256];   
long int cbu_tab[256];   
long int cgu_tab[256];   
   
long int cgv_tab[256];   
long int tab_76309[256];   
unsigned char clp[1024];   
   
   
void init_dither_tab()   
{   
    long int crv,cbu,cgu,cgv;   
    int i,ind;   
   
    crv = 104597; cbu = 132201;    
    cgu = 25675;  cgv = 53279;   
   
    for (i = 0; i < 256; i++) {   
        crv_tab[i] = (i-128) * crv;   
        cbu_tab[i] = (i-128) * cbu;   
        cgu_tab[i] = (i-128) * cgu;   
        cgv_tab[i] = (i-128) * cgv;   
        tab_76309[i] = 76309*(i-16);   
    }   
   
    for (i=0; i<384; i++)   
        clp[i] =0;   
    ind=384;   
    for (i=0;i<256; i++)   
        clp[ind++]=i;   
    ind=640;   
    for (i=0;i<384;i++)   
        clp[ind++]=255;   
}   
   
   
void YUV2RGB420(unsigned char *src0, int stride_y, unsigned char *src1, unsigned char *src2, int stride_uv, 
				unsigned char *dst_ori, int width, int height)   
{       
    int y1,y2,u,v;    
    unsigned char *py1,*py2, *pu1, *pv1;   
    int i,j, c1, c2, c3, c4;   
    unsigned char *d1, *d2;   
   
    //src0=src;   
    //src1=src+width*height;   
    //src2=src+width*height+width*height/4;   
   
    py1=src0;   
    py2=py1+width;   
    d1=dst_ori;   
    d2=d1+4*width;
	pu1 = src1;
	pv1 = src2;
    for (j = 0; j < height; j += 2) {    
        for (i = 0; i < width; i += 2) {   
   
            u = *pu1++;   
            v = *pv1++;   
   
            c1 = crv_tab[v];   
            c2 = cgu_tab[u];   
            c3 = cgv_tab[v];   
            c4 = cbu_tab[u];   
   
            //up-left   
            y1 = tab_76309[*py1++];
			*d1++ = 255;
			*d1++ = clp[384+((y1 + c4)>>16)];
			*d1++ = clp[384+((y1 - c2 - c3)>>16)];  
            *d1++ = clp[384+((y1 + c1)>>16)]; 
   
            //down-left   
            y2 = tab_76309[*py2++];
			*d2++ = 255;
			*d2++ = clp[384+((y2 + c4)>>16)];
			*d2++ = clp[384+((y2 - c2 - c3)>>16)];  
            *d2++ = clp[384+((y2 + c1)>>16)]; 
   
            //up-right   
            y1 = tab_76309[*py1++];
			*d1++ = 255;
			*d1++ = clp[384+((y1 + c4)>>16)]; 
			*d1++ = clp[384+((y1 - c2 - c3)>>16)];
            *d1++ = clp[384+((y1 + c1)>>16)];         
   
            //down-right   
            y2 = tab_76309[*py2++];
			*d2++ = 255;
			*d2++ = clp[384+((y2 + c4)>>16)];
			*d2++ = clp[384+((y2 - c2 - c3)>>16)];
            *d2++ = clp[384+((y2 + c1)>>16)];     
        }   
        d1 += 4*width;   
        d2 += 4*width;   
        py1+=   stride_y;   
        py2+=   stride_y;
		src1 += stride_uv;
		src2 += stride_uv;
		pu1 = src1;
		pv1 = src2;
    }          
}   
   
   
//How to use:   
//YUV_TO_RGB24(pY,width,pU,pV,width>>1,pRGBBuf,width,(int)0-height,width*3);   
typedef   UCHAR    uint8_t;   
typedef   ULONGLONG   uint64_t;   
   
#define MAXIMUM_Y_WIDTH 1280   
static uint64_t mmw_mult_Y    = 0x2568256825682568;   
static uint64_t mmw_mult_U_G  = 0xf36ef36ef36ef36e;   
static uint64_t mmw_mult_U_B  = 0x40cf40cf40cf40cf;   
static uint64_t mmw_mult_V_R  = 0x3343334333433343;   
static uint64_t mmw_mult_V_G  = 0xe5e2e5e2e5e2e5e2;   
   
   
static uint64_t mmb_0x10      = 0x1010101010101010;   
static uint64_t mmw_0x0080    = 0x0080008000800080;   
static uint64_t mmw_0x00ff    = 0x00ff00ff00ff00ff;   
   
static uint64_t mmw_cut_red   = 0x7c007c007c007c00;   
static uint64_t mmw_cut_green = 0x03e003e003e003e0;   
static uint64_t mmw_cut_blue  = 0x001f001f001f001f;   
   
   
void YUV_TO_RGB32(uint8_t *puc_y, int stride_y,    
                  uint8_t *puc_u, uint8_t *puc_v, int stride_uv,    
                  uint8_t *puc_out, int width_y, int height_y, int stride_out)    
{   
    int y, horiz_count;   
    uint8_t *puc_out_remembered;   
    //int stride_out = width_y * 3;   
       
    if (height_y < 0) {   
        //we are flipping our output upside-down   
        height_y  = -height_y;   
        puc_y     += (height_y   - 1) * stride_y ;   
        puc_u     += (height_y/2 - 1) * stride_uv;   
        puc_v     += (height_y/2 - 1) * stride_uv;   
        stride_y  = -stride_y;   
        stride_uv = -stride_uv;   
    }   
   
    horiz_count = -(width_y >> 3);   
   
    for (y=0; y<height_y; y++) {   
        if (y == height_y-1) {   
            //this is the last output line - we need to be careful not to overrun the end of this line   
            uint8_t temp_buff[4*MAXIMUM_Y_WIDTH+1];
            puc_out_remembered = puc_out;   
            puc_out = temp_buff; //write the RGB to a temporary store   
        }   
        _asm {   
            push eax   
            push ebx   
            push ecx   
            push edx   
            push edi   
   
            mov eax, puc_out          
            mov ebx, puc_y          
            mov ecx, puc_u          
            mov edx, puc_v   
            mov edi, horiz_count   
   
            horiz_loop:   
   
            movd mm2, [ecx]   
            pxor mm7, mm7   
   
            movd mm3, [edx]   
            punpcklbw mm2, mm7          
   
            movq mm0, [ebx]             
            punpcklbw mm3, mm7          
   
            movq mm1, mmw_0x00ff
   
            psubusb mm0, mmb_0x10
   
            psubw mm2, mmw_0x0080
            pand mm1, mm0
   
            psubw mm3, mmw_0x0080
            psllw mm1, 3
   
            psrlw mm0, 8
            psllw mm2, 3                
   
            pmulhw mm1, mmw_mult_Y      
            psllw mm0, 3                
   
            psllw mm3, 3                
            movq mm5, mm3               
   
            pmulhw mm5, mmw_mult_V_R    
            movq mm4, mm2               
   
            pmulhw mm0, mmw_mult_Y      
            movq mm7, mm1               
   
            pmulhw mm2, mmw_mult_U_G    
            paddsw mm7, mm5   
   
            pmulhw mm3, mmw_mult_V_G   
            packuswb mm7, mm7   
   
            pmulhw mm4, mmw_mult_U_B   
            paddsw mm5, mm0         
   
            packuswb mm5, mm5   
            paddsw mm2, mm3             
   
            movq mm3, mm1               
            movq mm6, mm1               
   
            paddsw mm3, mm4   
            paddsw mm6, mm2   
   
            punpcklbw mm7, mm5   
            paddsw mm2, mm0   
   
            packuswb mm6, mm6   
            packuswb mm2, mm2   
   
            packuswb mm3, mm3   
            paddsw mm4, mm0   
   
            packuswb mm4, mm4   
            punpcklbw mm6, mm2   
   
            punpcklbw mm3, mm4   
   
            // 32-bit shuffle.   
            pxor mm0, mm0   
   
            movq mm1, mm6   
            punpcklbw mm1, mm0   
   
            movq mm0, mm3   
            punpcklbw mm0, mm7   
   
            movq mm2, mm0   
   
            punpcklbw mm0, mm1   
            punpckhbw mm2, mm1   
   
            // 24-bit shuffle and sav   
            movd   [eax], mm0   
            psrlq mm0, 32   
   
            movd  4[eax], mm0   
   
            movd  8[eax], mm2   
   
   
            psrlq mm2, 32               
   
            movd  12[eax], mm2           
   
            // 32-bit shuffle.   
            pxor mm0, mm0               
   
            movq mm1, mm6               
            punpckhbw mm1, mm0          
   
            movq mm0, mm3               
            punpckhbw mm0, mm7          
   
            movq mm2, mm0               
   
            punpcklbw mm0, mm1          
            punpckhbw mm2, mm1          
   
            // 24-bit shuffle and sav   
            movd 16[eax], mm0           
            psrlq mm0, 32               
   
            movd 20[eax], mm0           
            add ebx, 8                  
   
            movd 24[eax], mm2           
            psrlq mm2, 32               
   
            add ecx, 4                  
            add edx, 4                  
   
            movd 28[eax], mm2           
            add eax, 32                 
   
            inc edi   
            jne horiz_loop   
   
            pop edi   
            pop edx   
            pop ecx   
            pop ebx   
            pop eax   
   
            emms   
        }   
   
        if (y == height_y-1) {   
            //last line of output - we have used the temp_buff and need to copy   
            int x = 4 * width_y;                  //interation counter   
            uint8_t *ps = puc_out;                // source pointer (temporary line store)   
            uint8_t *pd = puc_out_remembered;     // dest pointer   
            while (x--) *(pd++) = *(ps++);          // copy the line   
        }   
   
        puc_y   += stride_y;   
        if (y%2) {   
            puc_u   += stride_uv;   
            puc_v   += stride_uv;   
        }   
        puc_out += stride_out;    
    }   
}

long int rv_tab[256];   
long int gu_tab[256];  
long int gv_tab[256];
long int bu_tab[256];

void init_s_tab()   
{   
    int i;
   
    for (i = 0; i < 256; i++) {   
        rv_tab[i] = (i-128) * 1436 >> 10;   
        gu_tab[i] = (i-128) * 352 >> 10;
        gv_tab[i] = (i-128) * 731 >> 10;
        bu_tab[i] = (i-128) * 1815 >> 10;
    }
}

void YUV2RGB420_s(unsigned char *src0, int stride_y, unsigned char *src1, unsigned char *src2, int stride_uv, 
				DWORD *dst_ori, int width, int height)   
{       
    int u,v;
    unsigned char *py1,*py2, *pu1, *pv1;
    int i,j, c1, c2, c3;
    //unsigned char *d1, *d2;
	DWORD *d1, *d2;

	//memset(dst_ori, 255, width * height * 4);
   
    //src0=src;   
    //src1=src+width*height;   
    //src2=src+width*height+width*height/4;   
   
    py1 = src0;   
    py2 = py1 + stride_y;   
    d1 = dst_ori;   
    d2 = d1 + width;
	pu1 = src1;
	pv1 = src2;

    for (j = 0; j < height; j += 2) {    
        for (i = 0; i < width; i += 2) {   
   
            u = (char)((*pu1++) - 128);   
            v = (char)((*pv1++) - 128);

            //up-left
			//c1 = *py1 + ((1436 * v) >> 10);
			c1 = *py1 + rv_tab[v + 128];
			if (c1 < 0) c1 = 0;
			if (c1 > 255) c1 = 255;

			//c2 = *py1 - ((352 * u + 731 * v) >> 10);
			c2 = *py1 - gu_tab[u + 128] - gv_tab[v + 128];
			if (c2 < 0) c2 = 0;
			if (c2 > 255) c2 = 255;

			//c3 = *py1 + ((1815 * u) >> 10);
			c3 = *py1 + bu_tab[u + 128];
			if (c3 < 0) c3 = 0;
			if (c3 > 255) c3 = 255;

			*d1++ = (c1 << 16) | (c2 << 8) | (c3);
			//*d1++ = c3;
			//*d1++ = c2;  
            //*d1++ = c1; 
			//*d1++ = 255;
			++py1;

            //down-left   
			//c1 = *py2 + ((1436 * v) >> 10);
			c1 = *py2 + rv_tab[v + 128];
			if (c1 < 0) c1 = 0;
			if (c1 > 255) c1 = 255;

			//c2 = *py2 - ((352 * u + 731 * v) >> 10);
			c2 = *py2 - gu_tab[u + 128] - gv_tab[v + 128];
			if (c2 < 0) c2 = 0;
			if (c2 > 255) c2 = 255;

			//c3 = *py2 + ((1815 * u) >> 10);
			c3 = *py2 + bu_tab[u + 128];
			if (c3 < 0) c3 = 0;
			if (c3 > 255) c3 = 255;
			
			*d2++ = (c1 << 16) | (c2 << 8) | (c3);
			//*d2++ = c3;
			//*d2++ = c2;  
			//*d2++ = c1; 
			//*d2++ = 255;
			++py2;
   
            //up-right   
			//c1 = *py1 + ((1436 * v) >> 10);
			c1 = *py1 + rv_tab[v + 128];
			if (c1 < 0) c1 = 0;
			if (c1 > 255) c1 = 255;

			//c2 = *py1 - ((352 * u + 731 * v) >> 10);
			c2 = *py1 - gu_tab[u + 128] - gv_tab[v + 128];
			if (c2 < 0) c2 = 0;
			if (c2 > 255) c2 = 255;

			//c3 = *py1 + ((1815 * u) >> 10);
			c3 = *py1 + bu_tab[u + 128];
			if (c3 < 0) c3 = 0;
			if (c3 > 255) c3 = 255;
			
			*d1++ = (c1 << 16) | (c2 << 8) | (c3);
			//*d1++ = c3;
			//*d1++ = c2;  
            //*d1++ = c1; 
			//*d1++ = 255;
			++py1;
   
            //down-right   
            //c1 = *py2 + ((1436 * v) >> 10);
			c1 = *py2 + rv_tab[v + 128];
			if (c1 < 0) c1 = 0;
			if (c1 > 255) c1 = 255;

			//c2 = *py2 - ((352 * u + 731 * v) >> 10);
			c2 = *py2 - gu_tab[u + 128] - gv_tab[v + 128];
			if (c2 < 0) c2 = 0;
			if (c2 > 255) c2 = 255;

			//c3 = *py2 + ((1815 * u) >> 10);
			c3 = *py2 + bu_tab[u + 128];
			if (c3 < 0) c3 = 0;
			if (c3 > 255) c3 = 255;
			
			*d2++ = (c1 << 16) | (c2 << 8) | (c3);
			//*d2++ = c3;
			//*d2++ = c2;  
            //*d2++ = c1; 
			//*d2++ = 255;
			++py2;
        }
		
        d1 += width;
        d2 += width;
        py1 += 2 * stride_y - width;   
        py2 += 2 * stride_y - width;
		src1 += stride_uv;
		src2 += stride_uv;
		pu1 = src1;
		pv1 = src2;
    }          
}

//How to use:   
//YUV_TO_RGB24(pY,width,pU,pV,width>>1,pRGBBuf,width,(int)0-height,width*3);
static uint64_t mmw_mult_U_G_s  = 0x0b020b020b020b02;   
static uint64_t mmw_mult_U_B_s  = 0x38bc38bc38bc38bc;   
static uint64_t mmw_mult_V_R_s  = 0x2ce52ce52ce52ce5;   
static uint64_t mmw_mult_V_G_s  = 0x16d916d916d916d9;   

static uint64_t mmw_0x0080_s    = 0x0080008000800080;   
static uint64_t mmw_0x00ff_s    = 0x00ff00ff00ff00ff;   

void YUV_TO_RGB32_s(uint8_t *puc_y, int stride_y,    
                  uint8_t *puc_u, uint8_t *puc_v, int stride_uv,    
                  uint8_t *puc_out, int width_y, int height_y, int stride_out)    
{   
    int y, horiz_count;   
    uint8_t *puc_out_remembered;   
    //int stride_out = width_y * 3;   
       
    if (height_y < 0) {   
        //we are flipping our output upside-down   
        height_y  = -height_y;   
        puc_y     += (height_y   - 1) * stride_y ;   
        puc_u     += (height_y/2 - 1) * stride_uv;   
        puc_v     += (height_y/2 - 1) * stride_uv;   
        stride_y  = -stride_y;   
        stride_uv = -stride_uv;   
    }   
   
    horiz_count = -(width_y >> 3);   
   
    for (y=0; y<height_y; y++) {   
        if (y == height_y-1) {   
            //this is the last output line - we need to be careful not to overrun the end of this line   
            uint8_t temp_buff[4*MAXIMUM_Y_WIDTH+1];
            puc_out_remembered = puc_out;   
            puc_out = temp_buff; //write the RGB to a temporary store   
        }   
        _asm {   
            push eax   
            push ebx   
            push ecx   
            push edx   
            push edi   
   
            mov eax, puc_out          
            mov ebx, puc_y          
            mov ecx, puc_u          
            mov edx, puc_v   
            mov edi, horiz_count   
   
            horiz_loop:   
   
            movd mm2, [ecx]				//mm2 = u3 u2 u1 u0
            pxor mm7, mm7				//clear mm7
   
            movd mm3, [edx]				//mm3 = v3 v2 v1 v0
            punpcklbw mm2, mm7			//mm2 = 00 u3 00 u2 00 u1 00 u0    
   
            movq mm0, [ebx]				//mm0 = y7 y6 y5 y4 y3 y2 y1 y0       
            punpcklbw mm3, mm7			//mm3 = 00 v3 00 v2 00 v1 00 v0    
   
            movq mm1, mmw_0x00ff_s		//mm1 = mmw_0x00ff
   
            //psubusb mm0, mmb_0x10
   
            psubw mm2, mmw_0x0080_s		//mm2 = u - 128 
            pand mm1, mm0				//mm1 = y6 y4 y2 y0
   
            psubw mm3, mmw_0x0080_s		//mm3 = v - 128
            //psllw mm1, 3				
   
            psrlw mm0, 8				//mm0 = y7 y5 y3 y1
            psllw mm2, 3                //mm2 = (u - 128) << 3
   
            //pmulhw mm1, mmw_mult_Y      
            //psllw mm0, 3				  
   
            psllw mm3, 3				//mm3 = (v - 128) << 3               
            movq mm5, mm3				//mm5 = mm3 = v - 128    
   
            pmulhw mm5, mmw_mult_V_R_s	//mm5 = ((v - 128) << 3) * (0x2ce5)[1.403 * 2^13] 
            movq mm4, mm2               //mm4 = mm2 = (u - 128) << 3
   
           //pmulhw mm0, mmw_mult_Y      
            movq mm7, mm1               //mm7 = mm1 = yl
   
            pmulhw mm2, mmw_mult_U_G_s  //mm2 = ((u - 128) << 3) * (0x0b02)[0.334 * 2^13]
            paddsw mm7, mm5				//mm7 = Rl = yl + ((v - 128) << 3) * (0x2ce5)[1.403 * 2^13] 
   
            pmulhw mm3, mmw_mult_V_G_s  //mm3 = ((v - 128) << 3) * (0x16d9)[0.714 * 2^13]
            packuswb mm7, mm7			//mm7 = Rl[0~255]
   
            pmulhw mm4, mmw_mult_U_B_s	//mm4 = ((u - 128) << 3) * (0x38bc)[1.773 * 2^13]
            paddsw mm5, mm0				//mm5 = Rh = yh + ((v - 128) << 3) * (0x2ce5)[1.403 * 2^13]
   
            packuswb mm5, mm5			//mm5 = Rh[0~255]
            paddsw mm2, mm3             //mm2 = mm2 + mm3 = ((u - 128) << 3) * (0x0b02)[0.334 * 2^13] + ((v - 128) << 3) * (0x16d9)[0.714 * 2^13]
   
            movq mm3, mm1				//mm3 = mm1 = yl 
            movq mm6, mm1				//mm6 = mm1 = yl    
   
            paddsw mm3, mm4				//mm3 = Bl = yl + ((u - 128) << 3) * (0x38bc)[1.773 * 2^13]
			//paddsw mm6, mm2
            psubsw mm6, mm2				//mm6 = Gl = yl - (((u - 128) << 3) * (0x0b02)[0.334 * 2^13] + ((v - 128) << 3) * (0x16d9)[0.714 * 2^13])
   
            punpcklbw mm7, mm5			//mm7 = RhRl
			movq mm5, mm0				//mm5 = yh
            //paddsw mm2, mm0
			psubsw mm5, mm2				//mm5 = Gh = yh - (((u - 128) << 3) * (0x0b02)[0.334 * 2^13] + ((v - 128) << 3) * (0x16d9)[0.714 * 2^13])
			movq mm2, mm5				//mm2 = Gh

            packuswb mm6, mm6			//mm6 = Gl[0~255]
            packuswb mm2, mm2			//mm2 = Gh[0~255]
   
            packuswb mm3, mm3			//mm3 = Bl[0~255]
            paddsw mm4, mm0				//mm4 = Bh = yh + ((u - 128) << 3) * (0x38bc)[1.773 * 2^13]
   
            packuswb mm4, mm4			//mm4 = Bh[0~255]
            punpcklbw mm6, mm2			//mm6 = GhGl
   
            punpcklbw mm3, mm4			//mm3 = BhBl
   
            // 32-bit shuffle.   
            pxor mm0, mm0				//clear mm0
   
            movq mm1, mm6				//mm1 = GhGl
            punpcklbw mm1, mm0			//mm1 = Gl
   
            movq mm0, mm3				//mm0 = BhBl
            punpcklbw mm0, mm7			//mm0 = R3B3 R2B2 R1B1 R0B0
   
            movq mm2, mm0				//mm2 = R3B3 R2B2 R1B1 R0B0
   
            punpcklbw mm0, mm1			//mm0 = 00 R1 G1 B1 00 R0 G0 B0
            punpckhbw mm2, mm1			//mm2 = 00 R3 G3 B3 00 R2 G2 B2
   
            // 24-bit shuffle and sav   
            movd   [eax], mm0			//write address 00 R0 G0 B0
            psrlq mm0, 32				//mm0 = 00 R1 G1 B1
   
            movd  4[eax], mm0			//write address 00 R1 G1 B1
   
            movd  8[eax], mm2			//write address 00 R2 G2 B2
   
   
            psrlq mm2, 32				//mm2 = 00 R3 G3 B3
   
            movd  12[eax], mm2          //write address 00 R3 G3 B3
   
            // 32-bit shuffle.   
            pxor mm0, mm0				//clear mm0         
   
            movq mm1, mm6               //mm1 = GhGl
            punpckhbw mm1, mm0          //mm1 = Gh
   
            movq mm0, mm3               //mm0 = BhBl 
            punpckhbw mm0, mm7          //mm0 = R7B7 R6B6 R5B5 R4B4
   
            movq mm2, mm0               //mm2 = R7B7 R6B6 R5B5 R4B4
   
            punpcklbw mm0, mm1          //mm0 = 00R5 G5B5 00R4 G4B4
            punpckhbw mm2, mm1          //mm2 = 00R7 G7B7 00R6 G6B6
   
            // 24-bit shuffle and sav   
            movd 16[eax], mm0			//write address 00 R4 G4 B4
            psrlq mm0, 32               //mm0 = 00 R5 G5 B5
   
            movd 20[eax], mm0           //write address 00 R5 G5 B5
            add ebx, 8                  //puc_y += 8
   
            movd 24[eax], mm2			//write address 00 R6 G6 B6
            psrlq mm2, 32               //mm2 = 00 R7 G7 B7
   
            add ecx, 4                  //puc_u += 4
            add edx, 4                  //puc_v += 4
   
            movd 28[eax], mm2           //write address 00 R7 G7 B7
            add eax, 32                 //puc_out(rgb) += 32
   
            inc edi   
            jne horiz_loop   
   
            pop edi   
            pop edx   
            pop ecx   
            pop ebx   
            pop eax   
   
            emms   
        }   
   
        if (y == height_y-1) {   
            //last line of output - we have used the temp_buff and need to copy   
            int x = 4 * width_y;                  //interation counter   
            uint8_t *ps = puc_out;                // source pointer (temporary line store)   
            uint8_t *pd = puc_out_remembered;     // dest pointer   
            while (x--) *(pd++) = *(ps++);          // copy the line   
        }   
   
        puc_y   += stride_y;   
        if (y%2) {   
            puc_u   += stride_uv;   
            puc_v   += stride_uv;   
        }   
        puc_out += stride_out;    
    }   
}