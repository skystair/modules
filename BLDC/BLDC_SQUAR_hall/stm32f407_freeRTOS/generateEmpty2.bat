@echo off
set /p keyword=enter key word:
echo.>%keyword%.c
echo.>%keyword%.h
echo #include "%keyword%.h" > %keyword%.c
echo.>>%keyword%.c
echo void %keyword%_init(void); >> %keyword%.c
echo void %keyword%_tick1ms(void); >> %keyword%.c
echo void %keyword%_func(void); >> %keyword%.c
echo #ifndef __%keyword%_h__ > %keyword%.h
echo #define __%keyword%_h__ >> %keyword%.h
echo.>>%keyword%.h
echo.>>%keyword%.h
echo void %keyword%_init(void); >> %keyword%.h
echo void %keyword%_tick1ms(void); >> %keyword%.h
echo void %keyword%_func(void); >> %keyword%.h
echo #endif >> %keyword%.h