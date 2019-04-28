#ifndef DEF_REGMINMAX
#define DEF_REGMINMAX

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#define regmin(a,b) (((a)<(b))?(a):(b))
#define regmax(a,b) (((a)>(b))?(a):(b))


#endif
