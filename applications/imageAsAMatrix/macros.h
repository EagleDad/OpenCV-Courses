#pragma once

#define IGNORE_WARNINGS_OPENCV_PUSH                                            \
    __pragma( warning( push ) ) __pragma(                                      \
        warning( disable : 6294 ) ) __pragma( warning( disable : 6201 ) )      \
        __pragma( warning( disable : 6269 ) ) __pragma(                        \
            warning( disable : 4365 ) ) __pragma( warning( disable : 5219 ) )  \
            __pragma( warning( disable : 4946 ) ) __pragma( warning(           \
                disable : 5054 ) ) __pragma( warning( disable : 4625 ) )       \
                __pragma( warning( disable : 5026 ) ) __pragma( warning(       \
                    disable : 4626 ) ) __pragma( warning( disable : 5027 ) )   \
                    __pragma( warning( disable : 4464 ) ) __pragma(            \
                        warning( disable : 4266 ) )                            \
                        __pragma( warning( disable : 4263 ) ) __pragma(        \
                            warning( disable : 4264 ) )                        \
                            __pragma( warning( disable : 4866 ) ) __pragma(    \
                                warning( disable : 4548 ) )                    \
                                __pragma( warning( disable : 4996 ) )          \
                                    __pragma( warning( disable : 4061 ) )      \
                                        __pragma( warning( disable : 5267 ) )  \
                                            __pragma(                          \
                                                warning( disable : 4127 ) )

#define IGNORE_WARNINGS_POP __pragma( warning( pop ) )
