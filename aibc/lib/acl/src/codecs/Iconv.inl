
#include "GBKUnicode.tab"

///iconv
static int gb2312_mbtowc( unsigned int *apcOutput, const unsigned char *apcSource, int aiSize )
{
    unsigned char   liChar1 = apcSource[0];
    
    if((liChar1 >= 0x21 && liChar1 <= 0x29) || (liChar1 >= 0x30 && liChar1 <= 0x77)) 
    {
        if( aiSize >= 2 ) 
        {
            unsigned char liChar2 = apcSource[1];
            if( liChar2 >= 0x21 && liChar2 < 0x7f ) 
            {
                unsigned int i = 94 * (liChar1 - 0x21) + (liChar2 - 0x21);
                unsigned short liWChar = 0xfffd;
                
                if( i < 1410 ) 
                {
                    if( i < 831 )
                        liWChar = giGB2312_2UNI_PAGE21[i];
                } 
                else 
                {
                    if( i < 8178 )
                        liWChar = giGB2312_2UNI_PAGE30[i-1410];
                }
                
                if( liWChar != 0xfffd ) 
                {
                    *apcOutput = (unsigned int)liWChar;
                    return  (2);
                }
            }
      
            return  (-1);
        }
        
        return  (-2);
    }
    
    return  (-1);
}

static int gb2312_wctomb( unsigned char *apcOutput, unsigned int aiWChar, int aiSize )
{
    if( aiSize >= 2 )
    {
        const _INDEXMASK *summary = NULL;
        
        if( aiWChar >= 0x0000 && aiWChar < 0x0460 )
            summary = &giGB2312_UNI2INDX_PAGE00[(aiWChar>>4)];
        else if( aiWChar >= 0x2000 && aiWChar < 0x2650 )
            summary = &giGB2312_UNI2INDX_PAGE20[(aiWChar>>4)-0x200];
        else if( aiWChar >= 0x3000 && aiWChar < 0x3230 )
            summary = &giGB2312_UNI2INDX_PAGE30[(aiWChar>>4)-0x300];
        else if( aiWChar >= 0x4e00 && aiWChar < 0x9cf0 )
            summary = &giGB2312_UNI2INDX_PAGE4E[(aiWChar>>4)-0x4e0];
        else if( aiWChar >= 0x9e00 && aiWChar < 0x9fb0 )
            summary = &giGB2312_UNI2INDX_PAGE9E[(aiWChar>>4)-0x9e0];
        else if( aiWChar >= 0xff00 && aiWChar < 0xfff0 )
            summary = &giGB2312_UNI2INDX_PAGEFF[(aiWChar>>4)-0xff0];
        
        if( summary ) 
        {
            unsigned short liUsed = summary->used;
            unsigned int i = aiWChar & 0x0f;
            
            if (liUsed & ((unsigned short) 1 << i)) 
            {
                unsigned short liChar;
                /* Keep in `liUsed' only the bits 0..i-1. */
                liUsed &= ((unsigned short) 1 << i) - 1;
                /* Add `summary->indx' and the number of bits set in `used'. */
                liUsed = (liUsed & 0x5555) + ((liUsed & 0xaaaa) >> 1);
                liUsed = (liUsed & 0x3333) + ((liUsed & 0xcccc) >> 2);
                liUsed = (liUsed & 0x0f0f) + ((liUsed & 0xf0f0) >> 4);
                liUsed = (liUsed & 0x00ff) + (liUsed >> 8);
                liChar = giGB2312_2CHARSET[summary->indx + liUsed];
                apcOutput[0] = (liChar >> 8); apcOutput[1] = (liChar & 0xff);
                return  (2);
            }
        }
        
        return  (-1);
    }
    
    return  (-2);
}

static int cp936ext_mbtowc( unsigned int *apcOutput, const unsigned char *apcSource, int aiSize )
{
    unsigned char liChar1 = apcSource[0];
    if( (liChar1 == 0xa6) || (liChar1 == 0xa8)) 
    {
        if (aiSize >= 2) 
        {
            unsigned char liChar2 = apcSource[1];
            if((liChar2 >= 0x40 && liChar2 < 0x7f) || (liChar2 >= 0x80 && liChar2 < 0xff)) 
            {
                unsigned int i = 190 * (liChar1 - 0x81) + (liChar2 - (liChar2 >= 0x80 ? 0x41 : 0x40));
                unsigned short liWChar = 0xfffd;
                if (i < 7410) 
                {
                    if (i >= 7189 && i < 7211)
                        liWChar = giCP936EXT_2UNI_PAGEA6[i-7189];
                } 
                else 
                {
                    if (i >= 7532 && i < 7538)
                        liWChar = giCP936EXT_2UNI_PAGEA8[i-7532];
                }
                
                if( liWChar != 0xfffd) 
                {
                    *apcOutput = (unsigned int) liWChar;
                    return  (2);
                }
            }
        
            return  (-1);
        }
    
        return  (-2);
    }
    
    return  (-1);
}

static int cp936ext_wctomb( unsigned char *apcOutput, unsigned int aiWChar, int aiSize )
{
    if (aiSize >= 2) 
    {
        unsigned short liChar = 0;
        
        if (aiWChar >= 0x0140 && aiWChar < 0x0150)
            liChar = giCP936EXT_PAGE01[aiWChar-0x0140];
        else if (aiWChar >= 0x0250 && aiWChar < 0x0268)
            liChar = giCP936EXT_PAGE02[aiWChar-0x0250];
        else if (aiWChar >= 0xfe30 && aiWChar < 0xfe48)
            liChar = giCP936EXT_PAGEFE[aiWChar-0xfe30];
        
        if (liChar != 0) 
        {
            apcOutput[0] = (liChar >> 8); apcOutput[1] = (liChar & 0xff);
            return 2;
        }
        
        return  (-1);
    }
  
    return  (-2);
}

static int gbkext1_mbtowc( unsigned int *apcOutput, const unsigned char *apcSource, int aiSize )
{
    unsigned char liChar1 = apcSource[0];
  
    if ((liChar1 >= 0x81 && liChar1 <= 0xa0)) 
    {
        if (aiSize >= 2) 
        {
            unsigned char liChar2 = apcSource[1];
            
            if ((liChar2 >= 0x40 && liChar2 < 0x7f) || (liChar2 >= 0x80 && liChar2 < 0xff)) 
            {
                unsigned int i = 190 * (liChar1 - 0x81) + (liChar2 - (liChar2 >= 0x80 ? 0x41 : 0x40));
                unsigned short liWChar = 0xfffd;
                {
                    if (i < 6080)
                    liWChar = giGBKEXT1_2UNI_PAGE81[i];
                }
                
                if (liWChar != 0xfffd) 
                {
                    *apcOutput = (unsigned int) liWChar;
                    return  (2);
                }
            }
            
            return  (-1);
        }
        
        return  (-2);
    }
    
    return  (-1);
}

static int gbkext2_mbtowc( unsigned int *apcOutput, const unsigned char *apcSource, int aiSize )
{
    unsigned char liChar1 = apcSource[0];
    
    if ((liChar1 >= 0xa8 && liChar1 <= 0xfe)) 
    {
        if (aiSize >= 2) 
        {
            unsigned char liChar2 = apcSource[1];
            if ((liChar2 >= 0x40 && liChar2 < 0x7f) || (liChar2 >= 0x80 && liChar2 < 0xa1)) 
            {
                unsigned int i = 96 * (liChar1 - 0x81) + (liChar2 - (liChar2 >= 0x80 ? 0x41 : 0x40));
                unsigned short liWChar = 0xfffd;
                {
                    if (i < 12016)
                    liWChar = giGBKEXT2_2UNI_PAGEA8[i-3744];
                }
                
                if (liWChar != 0xfffd) 
                {
                    *apcOutput = (unsigned int) liWChar;
                    return  (2);
                }
            }
            
            return  (-1);
        }
        
        return  (-2);
    }
    
    return  (-1);
}

static int gbkext_inv_wctomb( unsigned char *apcOutput, unsigned int aiWChar, int aiSize )
{
    if (aiSize >= 2) 
    {
        const _INDEXMASK *summary = NULL;
        
        if (aiWChar >= 0x0200 && aiWChar < 0x02e0)
            summary = &giGBKEXT_INV_UNI2INDX_PAGE02[(aiWChar>>4)-0x020];
        else if (aiWChar >= 0x2000 && aiWChar < 0x22c0)
            summary = &giGBKEXT_INV_UNI2INDX_PAGE20[(aiWChar>>4)-0x200];
        else if (aiWChar >= 0x2500 && aiWChar < 0x2610)
            summary = &giGBKEXT_INV_UNI2INDX_PAGE25[(aiWChar>>4)-0x250];
        else if (aiWChar >= 0x3000 && aiWChar < 0x3100)
            summary = &giGBKEXT_INV_UNI2INDX_PAGE30[(aiWChar>>4)-0x300];
        else if (aiWChar >= 0x3200 && aiWChar < 0x33e0)
            summary = &giGBKEXT_INV_UNI2INDX_PAGE32[(aiWChar>>4)-0x320];
        else if (aiWChar >= 0x4e00 && aiWChar < 0x9fb0)
            summary = &giGBKEXT_INV_UNI2INDX_PAGE4E[(aiWChar>>4)-0x4e0];
        else if (aiWChar >= 0xf900 && aiWChar < 0xfa30)
            summary = &giGBKEXT_INV_UNI2INDX_PAGEF9[(aiWChar>>4)-0xf90];
        else if (aiWChar >= 0xfe00 && aiWChar < 0xfff0)
            summary = &giGBKEXT_INV_UNI2INDX_PAGEFE[(aiWChar>>4)-0xfe0];
            
        if (summary) 
        {
            unsigned short liUsed = summary->used;
            unsigned int i = aiWChar & 0x0f;
            
            if (liUsed & ((unsigned short) 1 << i)) 
            {
                unsigned short liChar;
                
                /* Keep in `liUsed' only the bits 0..i-1. */
                liUsed &= ((unsigned short) 1 << i) - 1;
                
                /* Add `summary->indx' and the number of bits set in `used'. */
                liUsed = (liUsed & 0x5555) + ((liUsed & 0xaaaa) >> 1);
                liUsed = (liUsed & 0x3333) + ((liUsed & 0xcccc) >> 2);
                liUsed = (liUsed & 0x0f0f) + ((liUsed & 0xf0f0) >> 4);
                liUsed = (liUsed & 0x00ff) + (liUsed >> 8);
                liChar = giGBKEXT_INV_2CHARSET[summary->indx + liUsed];
                apcOutput[0] = (liChar >> 8); apcOutput[1] = (liChar & 0xff);
                return  (2);
            }
        }
        
        return  (-1);
    }
    
    return  (-2);
}

static int ascii_mbtowc( unsigned int *apcOutput, const unsigned char *apcSource, int aiSize )
{
    unsigned char liChar = *apcSource;
  
    if (liChar < 0x80) 
    {
        *apcOutput = (unsigned int) liChar;
        return  (1);
    }
    
    return  (-1);
}

static int ascii_wctomb( unsigned char *apcOutput, unsigned int liWChar, int aiSize )
{
    if (liWChar < 0x0080) 
    {
        *apcOutput = liWChar;
        return  (1);
    }
    
    return  (-1);
}

static int gbk_mbtowc( unsigned int *apcOutput, const unsigned char *apcSource, int aiSize )
{
    unsigned char c = *apcSource;

    if (c >= 0x81 && c < 0xff) 
    {
        if (aiSize < 2)
            return  (-2);
        
        if (c >= 0xa1 && c <= 0xf7) 
        {
            unsigned char liChar2 = apcSource[1];
            if (c == 0xa1) 
            {
                if (liChar2 == 0xa4) 
                {
                    *apcOutput = 0x00b7;
                    return  (2);
                }
                
                if (liChar2 == 0xaa) 
                {
                    *apcOutput = 0x2014;
                    return  (2);
                }
            }
            
            if (liChar2 >= 0xa1 && liChar2 < 0xff) 
            {
                unsigned char buf[2];
                int ret;
                
                buf[0] = c-0x80; buf[1] = liChar2-0x80;
                ret = gb2312_mbtowc(apcOutput,buf,2);
                if (ret !=  (-1))
                    return ret;
                
                buf[0] = c; buf[1] = liChar2;
                ret = cp936ext_mbtowc(apcOutput,buf,2);
                if (ret !=  (-1))
                    return ret;
            }
        }
        
        if (c >= 0x81 && c <= 0xa0)
            return gbkext1_mbtowc(apcOutput,apcSource,2);
            
        if (c >= 0xa8 && c <= 0xfe)
            return gbkext2_mbtowc(apcOutput,apcSource,2);
            
        if (c == 0xa2) 
        {
            unsigned char liChar2 = apcSource[1];
            if (liChar2 >= 0xa1 && liChar2 <= 0xaa) 
            {
                *apcOutput = 0x2170+(liChar2-0xa1);
                return  (2);
            }
        }
    }
    
    return  (-1);
}

static int gbk_wctomb( unsigned char *apcOutput, unsigned int liWChar, int aiSize )
{
    unsigned char buf[2];
    int ret;

    if (liWChar != 0x30fb && liWChar != 0x2015) 
    {
        ret = gb2312_wctomb(buf,liWChar,2);
        if (ret !=  (-1)) 
        {
            if (ret != 2) abort();
            if (aiSize < 2)
                return  (-2);
            
            apcOutput[0] = buf[0]+0x80;
            apcOutput[1] = buf[1]+0x80;
            return  (2);
        }
    }
    
    ret = gbkext_inv_wctomb(buf,liWChar,2);
    if (ret !=  (-1)) 
    {
        if (ret != 2) abort();
        if (aiSize < 2)
            return  (-2);
        apcOutput[0] = buf[0];
        apcOutput[1] = buf[1];
        return  (2);
    }
    
    if (liWChar >= 0x2170 && liWChar <= 0x2179) 
    {
        apcOutput[0] = 0xa2;
        apcOutput[1] = 0xa1 + (liWChar-0x2170);
        return  (2);
    }
    
    ret = cp936ext_wctomb(buf,liWChar,2);
    if (ret !=  (-1)) 
    {
        if (ret != 2) abort();
        if (aiSize < 2)
            return  (-2);
        
        apcOutput[0] = buf[0];
        apcOutput[1] = buf[1];
        return  (2);
    }
    
    if (liWChar == 0x00b7) 
    {
        if (aiSize < 2)
        return  (-2);
        
        apcOutput[0] = 0xa1;
        apcOutput[1] = 0xa4;
        return  (2);
    }
    
    if (liWChar == 0x2014) 
    {
        if (aiSize < 2)
            return  (-2);
    
        apcOutput[0] = 0xa1;
        apcOutput[1] = 0xaa;
        return  (2);
    }

    return  (-1);
}

static int ces_gbk_mbtowc( unsigned int *apcOutput, const unsigned char *apcSource, int aiSize )
{
    unsigned char liChar = *apcSource;

    /* Code set 0 (ASCII or GB 1988-89) */
    if (liChar < 0x80)
        return ascii_mbtowc(apcOutput,apcSource,aiSize);
    
    /* Code set 1 (GBK) */
    if (liChar >= 0x81 && liChar < 0xff) 
    {
        if (aiSize < 2)
            return  (-2);
        return gbk_mbtowc(apcOutput,apcSource,2);
    }
    
    return  (-1);
}

static int ces_gbk_wctomb( unsigned char *apcOutput, unsigned int liWChar, int aiSize )
{
    unsigned char buf[2];
    int ret;

    /* Code set 0 (ASCII or GB 1988-89) */
    ret = ascii_wctomb(apcOutput,liWChar,aiSize);
    
    if (ret !=  (-1))
        return ret;

    /* Code set 1 (GBK) */
    ret = gbk_wctomb(buf,liWChar,2);
    if (ret !=  (-1)) 
    {
        if (ret != 2) abort();
        if (aiSize < 2)
            return  (-2);
        
        apcOutput[0] = buf[0];
        apcOutput[1] = buf[1];
        return  (2);
    }

    return  (-1);
}

static int big5_mbtowc( unsigned int *apcOutput, const unsigned char *apcSource, int aiSize )
{
    unsigned char liChar1 = apcSource[0];
    
    if ((liChar1 >= 0xa1 && liChar1 <= 0xc7) || (liChar1 >= 0xc9 && liChar1 <= 0xf9)) 
    {
        if (aiSize >= 2) 
        {
            unsigned char liChar2 = apcSource[1];
            
            if ((liChar2 >= 0x40 && liChar2 < 0x7f) || (liChar2 >= 0xa1 && liChar2 < 0xff)) 
            {
                unsigned int liIdx = 157 * (liChar1 - 0xa1) + (liChar2 - (liChar2 >= 0xa1 ? 0x62 : 0x40));
                unsigned short liWChar = 0xfffd;
                
                if (liIdx < 6280) 
                {
                    if (liIdx < 6121)
                        liWChar = giBIG5_2UNI_PAGEA1[liIdx];
                } 
                else 
                {
                    if (liIdx < 13932)
                        liWChar = giBIG5_2UNI_PAGEC9[liIdx-6280];
                }
                
                if (liWChar != 0xfffd) 
                {
                    *apcOutput = (unsigned int) liWChar;
                    return  (2);
                }
            }
            
            return  (-1);
        }
        
        return  (-2);
    }
    
    return  (-1);
}

static int big5_wctomb( unsigned char *apcOutput, unsigned int aiWChar, int aiSize )
{
    if (aiSize >= 2) 
    {
        const _INDEXMASK *summary = NULL;
        
        if (aiWChar >= 0x0000 && aiWChar < 0x0100)
            summary = &giBIG5_UNI2INDX_PAGE00[(aiWChar>>4)];
        else if (aiWChar >= 0x0200 && aiWChar < 0x0460)
            summary = &giBIG5_UNI2INDX_PAGE02[(aiWChar>>4)-0x020];
        else if (aiWChar >= 0x2000 && aiWChar < 0x22c0)
            summary = &giBIG5_UNI2INDX_PAGE20[(aiWChar>>4)-0x200];
        else if (aiWChar >= 0x2400 && aiWChar < 0x2650)
            summary = &giBIG5_UNI2INDX_PAGE24[(aiWChar>>4)-0x240];
        else if (aiWChar >= 0x3000 && aiWChar < 0x33e0)
            summary = &giBIG5_UNI2INDX_PAGE30[(aiWChar>>4)-0x300];
        else if (aiWChar >= 0x4e00 && aiWChar < 0x9fb0)
            summary = &big5_uni2indx_page4e[(aiWChar>>4)-0x4e0];
        else if (aiWChar >= 0xfa00 && aiWChar < 0xfa10)
            summary = &giBIG5_UNI2INDX_PAGEFA[(aiWChar>>4)-0xfa0];
        else if (aiWChar >= 0xfe00 && aiWChar < 0xff70)
            summary = &giBIG5_UNI2INDX_PAGEFE[(aiWChar>>4)-0xfe0];
            
        if (summary) 
        {
            unsigned short liUsed = summary->used;
            unsigned int liIdx = aiWChar & 0x0f;
            
            if (liUsed & ((unsigned short) 1 << liIdx)) 
            {
                unsigned short liChar;
                /* Keep in `liUsed' only the bits 0..liIdx-1. */
                liUsed &= ((unsigned short) 1 << liIdx) - 1;
                
                /* Add `summary->indx' and the number of bits set in `used'. */
                liUsed = (liUsed & 0x5555) + ((liUsed & 0xaaaa) >> 1);
                liUsed = (liUsed & 0x3333) + ((liUsed & 0xcccc) >> 2);
                liUsed = (liUsed & 0x0f0f) + ((liUsed & 0xf0f0) >> 4);
                liUsed = (liUsed & 0x00ff) + (liUsed >> 8);
                liChar = giBIG5_2CHARSET[summary->indx + liUsed];
                
                apcOutput[0] = (liChar >> 8); 
                apcOutput[1] = (liChar & 0xff);
                return  (2);
            }
        }
        
        return  (-1);
    }
    
    return  (-2);
}

static int ces_big5_mbtowc( unsigned int *apcOutput, const unsigned char *apcSource, int aiSize )
{
    unsigned char liChar = *apcSource;
    
    /* Code set 0 (ASCII) */
    if (liChar < 0x80)
        return ascii_mbtowc(apcOutput,apcSource,aiSize);
        
    /* Code set 1 (BIG5) */
    if (liChar >= 0xa1 && liChar < 0xff) 
    {
        if (aiSize < 2)
            return  (-2);

        {
            unsigned char liChar2 = apcSource[1];
            if ((liChar2 >= 0x40 && liChar2 < 0x7f) || (liChar2 >= 0xa1 && liChar2 < 0xff))
                return big5_mbtowc(apcOutput,apcSource,2);
            else
                return  (-1);
        }
    }
    
    return  (-1);
}

static int ces_big5_wctomb( unsigned char *apcOutput, unsigned int aiWChar, int aiSize )
{
    unsigned char buf[2];
    int ret;

    /* Code set 0 (ASCII) */
    ret = ascii_wctomb(apcOutput,aiWChar,aiSize);
    if (ret != (-1) )
        return ret;

    /* Code set 1 (BIG5) */
    ret = big5_wctomb(buf,aiWChar,2);
    if (ret != (-1) ) 
    {
        if (ret != 2) abort();
        if (aiSize < 2)
            return  (-2);
            
        apcOutput[0] = buf[0];
        apcOutput[1] = buf[1];
        return  (2);
    }

    return  (-1);
}
