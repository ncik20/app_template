#ifndef __ALT_STRING_H__
#define __ALT_STRING_H__

#ifndef NULL
#define NULL ((void*)0)
#endif

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

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

#ifdef __cplusplus
}
#endif

#endif /* __ALT_STRING_H__ */
