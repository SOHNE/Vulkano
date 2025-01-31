#ifndef VULKANO_VOAPI_H
#define VULKANO_VOAPI_H

#ifdef VO_SHARED
#if defined _WIN32 || defined __CYGWIN__
    #ifdef vulkano_EXPORTS
      #ifdef __GNUC__
        #define VO_API __attribute__ ( ( dllexport ) )
      #else
        #define VO_API __declspec( dllexport )
      #endif
    #else
      #ifdef __GNUC__
        #define VO_API __attribute__ ( ( dllimport ) )
      #else
        #define VO_API __declspec( dllimport )
      #endif
    #endif
  #else
    #if __GNUC__ >= 4
      #define VO_API __attribute__ ( ( visibility ( "default" ) ) )
    #else
      #define VO_API
    #endif
  #endif
#else
    #define VO_API
#endif

#endif //VULKANO_VOAPI_H
