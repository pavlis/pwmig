#ifndef __elog__
#define __elog__
#ifdef  __cplusplus
extern "C" {
#endif
void elog_log( int flag, const char *format, ... );
void elog_notify( int flag, const char *format, ... );
void elog_complain( int flag, const char *format, ... );
void elog_die( int flag, const char *format, ... );
#define register_error(x) elog_notify(x)
#define complain(x) elog_complain(x)
#define die(x) elog_die(x)
#ifdef  __cplusplus
}
#endif
#endif
