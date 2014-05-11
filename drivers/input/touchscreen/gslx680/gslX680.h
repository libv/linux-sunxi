#ifndef _GSLX680_H_
#define _GSLX680_H_

#define FILTER_POINT //☠☠☠默☠6☠☠要☠太☠
#ifdef FILTER_POINT
#define FILTER_MAX	12
#endif

/*☠☠☠缘☠锟疥不准☠☠锟解，一☠☠☠虏锟揭拷☠☠锟疥，
☠锟角对憋拷缘要☠冉锟阶硷拷☠☠☠募☠锟揭拷训☠☠☠☠0%*/

struct fw_data
{
    u32 offset : 8;
    u32 : 0;
    u32 val;
};

#endif
