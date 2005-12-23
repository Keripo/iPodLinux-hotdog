#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "hotdog.h"
#include "hotdog_animation.h"

/********* Linear animation *********/

typedef struct 
{
    hd_rect cur, delta, dest;
    int frames;
    void (*done)(hd_object *);
} anim_lineardata;

void HD_DoLinearAnimation (hd_object *obj) 
{
    anim_lineardata *a = (anim_lineardata *)obj->animdata;
    a->cur.x += a->delta.x; a->cur.y += a->delta.y;
    a->cur.w += a->delta.w; a->cur.h += a->delta.h;
    if (--a->frames <= 0) {
        void (*done)(hd_object *) = a->done;
        free (a); // so it works if done() calls HD_Animate*.
        obj->animdata = 0;
        obj->animate = 0;
        obj->animating = 0;
        // and *now* call the done func...
        if (done) (*done)(obj);
    }
}

void HD_AnimateLinear (hd_object *obj, int sx, int sy, int sw, int sh,
                       int dx, int dy, int dw, int dh, int frames, void (*done)(hd_object *))
{
    anim_lineardata *a = malloc (sizeof(anim_lineardata));
    assert (obj != NULL);
    assert (a != NULL);

    obj->animating = 1;
    obj->x = sx; obj->y = sy; obj->w = sw; obj->h = sh;
    obj->animdata = a;
    a->frames = frames;
    a->cur.x = (sx << 16); a->cur.y = (sy << 16);
    a->cur.w = (sw << 16); a->cur.h = (sh << 16);
    a->dest.x = (dx << 16); a->dest.y = (dy << 16);
    a->dest.w = (dw << 16); a->dest.h = (dh << 16);
    a->delta.x = (a->dest.x - a->cur.x) / frames;
    a->delta.y = (a->dest.y - a->cur.y) / frames;
    a->delta.w = (a->dest.w - a->cur.w) / frames;
    a->delta.h = (a->dest.h - a->cur.h) / frames;
    a->done = done;
    obj->animate = HD_DoLinearAnimation;
}

/********** Sine table, for all circular-ish stuff. *************/

// Units of index = rad*2048/pi. Units of result = 1.16 fixed-point ints.
uint32 sine_table[1025] = {
    0x0000, 0x0064, 0x00C9, 0x012D, 0x0192, 0x01F6, 0x025B, 0x02BF,
    0x0324, 0x0388, 0x03ED, 0x0451, 0x04B6, 0x051A, 0x057F, 0x05E3,
    0x0648, 0x06AC, 0x0711, 0x0775, 0x07DA, 0x083E, 0x08A3, 0x0907,
    0x096C, 0x09D0, 0x0A35, 0x0A99, 0x0AFE, 0x0B62, 0x0BC6, 0x0C2B,
    0x0C8F, 0x0CF4, 0x0D58, 0x0DBC, 0x0E21, 0x0E85, 0x0EEA, 0x0F4E,
    0x0FB2, 0x1017, 0x107B, 0x10DF, 0x1144, 0x11A8, 0x120C, 0x1270,
    0x12D5, 0x1339, 0x139D, 0x1401, 0x1466, 0x14CA, 0x152E, 0x1592,
    0x15F6, 0x165A, 0x16BF, 0x1723, 0x1787, 0x17EB, 0x184F, 0x18B3,
    0x1917, 0x197B, 0x19DF, 0x1A43, 0x1AA7, 0x1B0B, 0x1B6F, 0x1BD3,
    0x1C37, 0x1C9B, 0x1CFF, 0x1D63, 0x1DC7, 0x1E2A, 0x1E8E, 0x1EF2,
    0x1F56, 0x1FBA, 0x201D, 0x2081, 0x20E5, 0x2148, 0x21AC, 0x2210,
    0x2273, 0x22D7, 0x233B, 0x239E, 0x2402, 0x2465, 0x24C9, 0x252C,
    0x2590, 0x25F3, 0x2656, 0x26BA, 0x271D, 0x2781, 0x27E4, 0x2847,
    0x28AA, 0x290E, 0x2971, 0x29D4, 0x2A37, 0x2A9A, 0x2AFE, 0x2B61,
    0x2BC4, 0x2C27, 0x2C8A, 0x2CED, 0x2D50, 0x2DB3, 0x2E15, 0x2E78,
    0x2EDB, 0x2F3E, 0x2FA1, 0x3004, 0x3066, 0x30C9, 0x312C, 0x318E,
    0x31F1, 0x3254, 0x32B6, 0x3319, 0x337B, 0x33DE, 0x3440, 0x34A2,
    0x3505, 0x3567, 0x35C9, 0x362C, 0x368E, 0x36F0, 0x3752, 0x37B4,
    0x3817, 0x3879, 0x38DB, 0x393D, 0x399F, 0x3A01, 0x3A62, 0x3AC4,
    0x3B26, 0x3B88, 0x3BEA, 0x3C4B, 0x3CAD, 0x3D0F, 0x3D70, 0x3DD2,
    0x3E33, 0x3E95, 0x3EF6, 0x3F58, 0x3FB9, 0x401B, 0x407C, 0x40DD,
    0x413E, 0x41A0, 0x4201, 0x4262, 0x42C3, 0x4324, 0x4385, 0x43E6,
    0x4447, 0x44A8, 0x4508, 0x4569, 0x45CA, 0x462B, 0x468B, 0x46EC,
    0x474D, 0x47AD, 0x480E, 0x486E, 0x48CE, 0x492F, 0x498F, 0x49EF,
    0x4A50, 0x4AB0, 0x4B10, 0x4B70, 0x4BD0, 0x4C30, 0x4C90, 0x4CF0,
    0x4D50, 0x4DB0, 0x4E0F, 0x4E6F, 0x4ECF, 0x4F2E, 0x4F8E, 0x4FED,
    0x504D, 0x50AC, 0x510C, 0x516B, 0x51CA, 0x522A, 0x5289, 0x52E8,
    0x5347, 0x53A6, 0x5405, 0x5464, 0x54C3, 0x5522, 0x5581, 0x55DF,
    0x563E, 0x569D, 0x56FB, 0x575A, 0x57B8, 0x5817, 0x5875, 0x58D3,
    0x5931, 0x5990, 0x59EE, 0x5A4C, 0x5AAA, 0x5B08, 0x5B66, 0x5BC4,
    0x5C22, 0x5C7F, 0x5CDD, 0x5D3B, 0x5D98, 0x5DF6, 0x5E53, 0x5EB1,
    0x5F0E, 0x5F6B, 0x5FC9, 0x6026, 0x6083, 0x60E0, 0x613D, 0x619A,
    0x61F7, 0x6254, 0x62B1, 0x630D, 0x636A, 0x63C7, 0x6423, 0x6480,
    0x64DC, 0x6539, 0x6595, 0x65F1, 0x664D, 0x66A9, 0x6705, 0x6761,
    0x67BD, 0x6819, 0x6875, 0x68D1, 0x692D, 0x6988, 0x69E4, 0x6A3F,
    0x6A9B, 0x6AF6, 0x6B51, 0x6BAD, 0x6C08, 0x6C63, 0x6CBE, 0x6D19,
    0x6D74, 0x6DCF, 0x6E29, 0x6E84, 0x6EDF, 0x6F39, 0x6F94, 0x6FEE,
    0x7049, 0x70A3, 0x70FD, 0x7157, 0x71B1, 0x720C, 0x7265, 0x72BF,
    0x7319, 0x7373, 0x73CD, 0x7426, 0x7480, 0x74D9, 0x7533, 0x758C,
    0x75E5, 0x763F, 0x7698, 0x76F1, 0x774A, 0x77A3, 0x77FB, 0x7854,
    0x78AD, 0x7906, 0x795E, 0x79B7, 0x7A0F, 0x7A67, 0x7AC0, 0x7B18,
    0x7B70, 0x7BC8, 0x7C20, 0x7C78, 0x7CD0, 0x7D27, 0x7D7F, 0x7DD7,
    0x7E2E, 0x7E86, 0x7EDD, 0x7F34, 0x7F8B, 0x7FE2, 0x803A, 0x8090,
    0x80E7, 0x813E, 0x8195, 0x81EC, 0x8242, 0x8299, 0x82EF, 0x8345,
    0x839C, 0x83F2, 0x8448, 0x849E, 0x84F4, 0x854A, 0x85A0, 0x85F5,
    0x864B, 0x86A1, 0x86F6, 0x874B, 0x87A1, 0x87F6, 0x884B, 0x88A0,
    0x88F5, 0x894A, 0x899F, 0x89F4, 0x8A48, 0x8A9D, 0x8AF1, 0x8B46,
    0x8B9A, 0x8BEE, 0x8C42, 0x8C96, 0x8CEA, 0x8D3E, 0x8D92, 0x8DE6,
    0x8E39, 0x8E8D, 0x8EE0, 0x8F34, 0x8F87, 0x8FDA, 0x902D, 0x9080,
    0x90D3, 0x9126, 0x9179, 0x91CC, 0x921E, 0x9271, 0x92C3, 0x9315,
    0x9368, 0x93BA, 0x940C, 0x945E, 0x94B0, 0x9502, 0x9553, 0x95A5,
    0x95F6, 0x9648, 0x9699, 0x96EA, 0x973C, 0x978D, 0x97DE, 0x982E,
    0x987F, 0x98D0, 0x9921, 0x9971, 0x99C2, 0x9A12, 0x9A62, 0x9AB2,
    0x9B02, 0x9B52, 0x9BA2, 0x9BF2, 0x9C42, 0x9C91, 0x9CE1, 0x9D30,
    0x9D7F, 0x9DCF, 0x9E1E, 0x9E6D, 0x9EBC, 0x9F0A, 0x9F59, 0x9FA8,
    0x9FF6, 0xA045, 0xA093, 0xA0E1, 0xA12F, 0xA17E, 0xA1CB, 0xA219,
    0xA267, 0xA2B5, 0xA302, 0xA350, 0xA39D, 0xA3EA, 0xA438, 0xA485,
    0xA4D2, 0xA51F, 0xA56B, 0xA5B8, 0xA605, 0xA651, 0xA69D, 0xA6EA,
    0xA736, 0xA782, 0xA7CE, 0xA81A, 0xA866, 0xA8B1, 0xA8FD, 0xA948,
    0xA994, 0xA9DF, 0xAA2A, 0xAA75, 0xAAC0, 0xAB0B, 0xAB56, 0xABA0,
    0xABEB, 0xAC35, 0xAC80, 0xACCA, 0xAD14, 0xAD5E, 0xADA8, 0xADF2,
    0xAE3B, 0xAE85, 0xAECE, 0xAF18, 0xAF61, 0xAFAA, 0xAFF3, 0xB03C,
    0xB085, 0xB0CE, 0xB117, 0xB15F, 0xB1A8, 0xB1F0, 0xB238, 0xB280,
    0xB2C8, 0xB310, 0xB358, 0xB3A0, 0xB3E7, 0xB42F, 0xB476, 0xB4BD,
    0xB504, 0xB54B, 0xB592, 0xB5D9, 0xB620, 0xB667, 0xB6AD, 0xB6F3,
    0xB73A, 0xB780, 0xB7C6, 0xB80C, 0xB852, 0xB897, 0xB8DD, 0xB922,
    0xB968, 0xB9AD, 0xB9F2, 0xBA37, 0xBA7C, 0xBAC1, 0xBB06, 0xBB4A,
    0xBB8F, 0xBBD3, 0xBC17, 0xBC5B, 0xBCA0, 0xBCE3, 0xBD27, 0xBD6B,
    0xBDAE, 0xBDF2, 0xBE35, 0xBE78, 0xBEBC, 0xBEFF, 0xBF41, 0xBF84,
    0xBFC7, 0xC009, 0xC04C, 0xC08E, 0xC0D0, 0xC112, 0xC154, 0xC196,
    0xC1D8, 0xC21A, 0xC25B, 0xC29C, 0xC2DE, 0xC31F, 0xC360, 0xC3A1,
    0xC3E2, 0xC422, 0xC463, 0xC4A3, 0xC4E3, 0xC524, 0xC564, 0xC5A4,
    0xC5E4, 0xC623, 0xC663, 0xC6A2, 0xC6E2, 0xC721, 0xC760, 0xC79F,
    0xC7DE, 0xC81D, 0xC85B, 0xC89A, 0xC8D8, 0xC916, 0xC955, 0xC993,
    0xC9D1, 0xCA0E, 0xCA4C, 0xCA8A, 0xCAC7, 0xCB04, 0xCB41, 0xCB7F,
    0xCBBB, 0xCBF8, 0xCC35, 0xCC72, 0xCCAE, 0xCCEA, 0xCD26, 0xCD63,
    0xCD9F, 0xCDDA, 0xCE16, 0xCE52, 0xCE8D, 0xCEC8, 0xCF04, 0xCF3F,
    0xCF7A, 0xCFB4, 0xCFEF, 0xD02A, 0xD064, 0xD09F, 0xD0D9, 0xD113,
    0xD14D, 0xD187, 0xD1C0, 0xD1FA, 0xD233, 0xD26D, 0xD2A6, 0xD2DF,
    0xD318, 0xD351, 0xD389, 0xD3C2, 0xD3FA, 0xD433, 0xD46B, 0xD4A3,
    0xD4DB, 0xD512, 0xD54A, 0xD582, 0xD5B9, 0xD5F0, 0xD627, 0xD65F,
    0xD695, 0xD6CC, 0xD703, 0xD739, 0xD770, 0xD7A6, 0xD7DC, 0xD812,
    0xD848, 0xD87E, 0xD8B3, 0xD8E9, 0xD91E, 0xD953, 0xD988, 0xD9BD,
    0xD9F2, 0xDA27, 0xDA5B, 0xDA90, 0xDAC4, 0xDAF8, 0xDB2C, 0xDB60,
    0xDB94, 0xDBC7, 0xDBFB, 0xDC2E, 0xDC61, 0xDC94, 0xDCC7, 0xDCFA,
    0xDD2D, 0xDD5F, 0xDD92, 0xDDC4, 0xDDF6, 0xDE28, 0xDE5A, 0xDE8C,
    0xDEBE, 0xDEEF, 0xDF20, 0xDF52, 0xDF83, 0xDFB4, 0xDFE4, 0xE015,
    0xE046, 0xE076, 0xE0A6, 0xE0D6, 0xE106, 0xE136, 0xE166, 0xE196,
    0xE1C5, 0xE1F4, 0xE224, 0xE253, 0xE282, 0xE2B0, 0xE2DF, 0xE30D,
    0xE33C, 0xE36A, 0xE398, 0xE3C6, 0xE3F4, 0xE422, 0xE44F, 0xE47D,
    0xE4AA, 0xE4D7, 0xE504, 0xE531, 0xE55E, 0xE58A, 0xE5B7, 0xE5E3,
    0xE60F, 0xE63B, 0xE667, 0xE693, 0xE6BE, 0xE6EA, 0xE715, 0xE740,
    0xE76B, 0xE796, 0xE7C1, 0xE7EC, 0xE816, 0xE841, 0xE86B, 0xE895,
    0xE8BF, 0xE8E9, 0xE912, 0xE93C, 0xE965, 0xE98E, 0xE9B7, 0xE9E0,
    0xEA09, 0xEA32, 0xEA5A, 0xEA83, 0xEAAB, 0xEAD3, 0xEAFB, 0xEB23,
    0xEB4B, 0xEB72, 0xEB99, 0xEBC1, 0xEBE8, 0xEC0F, 0xEC36, 0xEC5C,
    0xEC83, 0xECA9, 0xECD0, 0xECF6, 0xED1C, 0xED41, 0xED67, 0xED8D,
    0xEDB2, 0xEDD7, 0xEDFC, 0xEE21, 0xEE46, 0xEE6B, 0xEE8F, 0xEEB4,
    0xEED8, 0xEEFC, 0xEF20, 0xEF44, 0xEF68, 0xEF8B, 0xEFAF, 0xEFD2,
    0xEFF5, 0xF018, 0xF03B, 0xF05D, 0xF080, 0xF0A2, 0xF0C5, 0xF0E7,
    0xF109, 0xF12A, 0xF14C, 0xF16D, 0xF18F, 0xF1B0, 0xF1D1, 0xF1F2,
    0xF213, 0xF233, 0xF254, 0xF274, 0xF294, 0xF2B5, 0xF2D4, 0xF2F4,
    0xF314, 0xF333, 0xF353, 0xF372, 0xF391, 0xF3B0, 0xF3CE, 0xF3ED,
    0xF40B, 0xF42A, 0xF448, 0xF466, 0xF484, 0xF4A1, 0xF4BF, 0xF4DC,
    0xF4FA, 0xF517, 0xF534, 0xF550, 0xF56D, 0xF58A, 0xF5A6, 0xF5C2,
    0xF5DE, 0xF5FA, 0xF616, 0xF632, 0xF64D, 0xF668, 0xF684, 0xF69F,
    0xF6BA, 0xF6D4, 0xF6EF, 0xF709, 0xF724, 0xF73E, 0xF758, 0xF772,
    0xF78B, 0xF7A5, 0xF7BE, 0xF7D7, 0xF7F1, 0xF80A, 0xF822, 0xF83B,
    0xF853, 0xF86C, 0xF884, 0xF89C, 0xF8B4, 0xF8CC, 0xF8E3, 0xF8FB,
    0xF912, 0xF929, 0xF940, 0xF957, 0xF96E, 0xF984, 0xF99B, 0xF9B1,
    0xF9C7, 0xF9DD, 0xF9F3, 0xFA09, 0xFA1E, 0xFA33, 0xFA49, 0xFA5E,
    0xFA73, 0xFA87, 0xFA9C, 0xFAB0, 0xFAC5, 0xFAD9, 0xFAED, 0xFB01,
    0xFB14, 0xFB28, 0xFB3B, 0xFB4E, 0xFB61, 0xFB74, 0xFB87, 0xFB9A,
    0xFBAC, 0xFBBF, 0xFBD1, 0xFBE3, 0xFBF5, 0xFC06, 0xFC18, 0xFC29,
    0xFC3B, 0xFC4C, 0xFC5D, 0xFC6E, 0xFC7E, 0xFC8F, 0xFC9F, 0xFCAF,
    0xFCBF, 0xFCCF, 0xFCDF, 0xFCEF, 0xFCFE, 0xFD0D, 0xFD1C, 0xFD2B,
    0xFD3A, 0xFD49, 0xFD57, 0xFD66, 0xFD74, 0xFD82, 0xFD90, 0xFD9E,
    0xFDAB, 0xFDB9, 0xFDC6, 0xFDD3, 0xFDE0, 0xFDED, 0xFDFA, 0xFE06,
    0xFE13, 0xFE1F, 0xFE2B, 0xFE37, 0xFE43, 0xFE4E, 0xFE5A, 0xFE65,
    0xFE70, 0xFE7B, 0xFE86, 0xFE91, 0xFE9B, 0xFEA6, 0xFEB0, 0xFEBA,
    0xFEC4, 0xFECE, 0xFED7, 0xFEE1, 0xFEEA, 0xFEF3, 0xFEFC, 0xFF05,
    0xFF0E, 0xFF16, 0xFF1F, 0xFF27, 0xFF2F, 0xFF37, 0xFF3F, 0xFF46,
    0xFF4E, 0xFF55, 0xFF5C, 0xFF63, 0xFF6A, 0xFF71, 0xFF78, 0xFF7E,
    0xFF84, 0xFF8A, 0xFF90, 0xFF96, 0xFF9C, 0xFFA1, 0xFFA6, 0xFFAC,
    0xFFB1, 0xFFB5, 0xFFBA, 0xFFBF, 0xFFC3, 0xFFC7, 0xFFCB, 0xFFCF,
    0xFFD3, 0xFFD7, 0xFFDA, 0xFFDD, 0xFFE1, 0xFFE4, 0xFFE7, 0xFFE9,
    0xFFEC, 0xFFEE, 0xFFF0, 0xFFF2, 0xFFF4, 0xFFF6, 0xFFF8, 0xFFF9,
    0xFFFB, 0xFFFC, 0xFFFD, 0xFFFE, 0xFFFE, 0xFFFF, 0xFFFF, 0xFFFF,
    0x10000
};

int32 fsin (int32 angle) 
{
    int32 corresp;
    while (angle >= 4096) angle -= 4096;
    while (angle < 0) angle += 4096;

    // Special values
    if ((angle & 1023) == 0) {
        if (angle ==    0) return  (0 << 16);
        if (angle == 1024) return  (1 << 16);
        if (angle == 2048) return  (0 << 16);
        if (angle == 3072) return -(1 << 16);
        assert (("mysterious error", 0));
    }
    
    if (angle < 1024) corresp = angle;
    else if (angle < 2048) corresp = 2048 - angle;
    else if (angle < 3072) corresp = angle - 2048;
    else if (angle < 4096) corresp = 4096 - angle;

    if (angle > 2048) return -sine_table[corresp];
    else return sine_table[corresp];
}

int32 fcos (int32 angle) 
{
    return fsin (angle + 1024);
}


/********* Circular animation *********/

typedef struct 
{
    int32 x, y, r, w, aspect_ratio;
    int32 fbot, ftop, fdelta; /* scale factors for Frontrow-style effect, << 16 */
    int32 angle; /* in rad*2048/pi << 16 */
    int32 adelta; /* same units */
    int frames;
} anim_circledata;

void HD_DoCircleAnimation (hd_object *obj) 
{
    anim_circledata *a = obj->animdata;
    a->angle += a->adelta;
    obj->x = a->x + ((a->r * fcos (a->angle >> 16)) >> 16);
    obj->y = a->y + ((a->r * fsin (a->angle >> 16)) >> 16);
    if (a->fbot != 0x10000 || a->ftop != 0x10000) {
        obj->w = (a->w * (a->fbot + ((a->fdelta >> 8) * ((fsin (a->angle >> 16) + 0x10000) >> 9)))) >> 16;
        obj->h = (obj->w * a->aspect_ratio) >> 16;
    }

    if (a->frames && !--a->frames) {
        free (obj->animdata);
        obj->animate = 0;
        obj->animating = 0;
    }
}

void HD_AnimateCircle (hd_object *obj, int32 x, int32 y, int32 r, int32 fbot, int32 ftop,
                       int32 astart, int32 adist, int frames) 
{
    anim_circledata *a = malloc (sizeof(anim_circledata));
    assert (obj != NULL);
    assert (a != NULL);

    obj->animating = 1;
    obj->animdata = a;
    if (frames < 0) {
        a->frames = 0;
        frames = -frames;
    } else {
        a->frames = frames;
    }
    a->x = x; a->y = y; a->r = r; a->w = obj->w;
    a->aspect_ratio = (obj->h << 16) / obj->w;
    a->fbot = (fbot == 1 || fbot == 0)? (1 << 16) : fbot;
    a->ftop = (ftop == 1 || ftop == 0)? (1 << 16) : ftop;
    a->fdelta = a->ftop - a->fbot;
    a->angle = astart << 16;
    a->adelta = (adist << 16) / frames;
    obj->animate = HD_DoCircleAnimation;    
    obj->x = a->x + ((a->r * fcos (a->angle >> 16)) >> 16);
    obj->y = a->y + ((a->r * fsin (a->angle >> 16)) >> 16);
    if (a->fbot != 0x10000 || a->ftop != 0x10000) {
        obj->w = (a->w * (a->fbot + ((a->fdelta >> 8) * ((fsin (a->angle >> 16) + 0x10000) >> 9)))) >> 16;
        obj->h = (obj->w * a->aspect_ratio) >> 16;
    }
}
                       
