#ifndef __ALT_STRING_H__
#define __ALT_STRING_H__

#ifndef NULL
#define NULL ((void*)0)
#endif

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
/*
char * strcpy(char * dest, const char * src)
{
    if ( dest == NULL || src == NULL) // 地址检查
    {
        return NULL;
    }

    if ( dest == src) // 相同地址检查
    {
        return dest;
    }

    char * str = dest;
    while ( ( *str++ = *src++ ) != '\0' )  // 循环复制
    {
        ;
    }

    return dest;
}
*/
unsigned int div10(unsigned short x)
{
    unsigned q,r;
    q=(x>>1)+(x>>2);
    q=q+(q>>4);
    q=q+(q>>8);
    q=q+(q>>16);
    q=q>>3;
    r=x-(((q<<2)+q)<<1);
    return q+(r>9);
}

/**
 * C version of Lukás Chmela's itoa
 * 兼容标准的 C 编译器 (GCC, Clang, MSVC)
 */
char* itoa(int value, char* result, int base) {
    // 检查进制合法性
    // if (base < 2 || base > 36) { *result = '\0'; return result; }
    // 只支持10进制
    if (base != 10) { *result = '\0'; return result; }

    char *ptr = result, *ptr1 = result, tmp_char;
    int tmp_value, tmp_value1;

    do {
        tmp_value = value;
        // value /= base;
        value = div10(value);
        tmp_value1 = (value << 3) + value + value;
        // 计算索引并从映射表中取值
        // C 语言允许直接索引字符串常量
        // *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - tmp_value1)];
    } while ( value );

    // 处理负号
    if (tmp_value < 0) { *ptr++ = '-'; }
    
    *ptr-- = '\0';
  
    // 原地反转字符串
    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
    
    return result;
}

#ifdef __cplusplus
}
#endif

#endif /* __ALT_STRING_H__ */
