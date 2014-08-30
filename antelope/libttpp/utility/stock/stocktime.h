#ifndef __stocktime__
#define __stocktime__

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef CSSTIME_DEF   
#define CSSTIME_DEF

struct date_time{  /* date/time structure from old time conversion routines */
        double epoch;
        long date;
        int year;
        int month;
        char mname[4];
        int day;
        int doy;
        int hour;
        int minute;
        double second;
};

#endif

#define MDTIME                  1
#define YDTIME                  2
#define MD                      3
#define YD                      4
#define STRTIME                 5
#define STRDATE                 6
#define STRYDTIME               7
#define USTRYDTIME              8

extern double epoch ( int yd );   
extern long yearday ( double e );
extern int is_month_first () ;
extern char *strydtime ( double e );
extern char *strtime ( double e );
extern char *strdate ( double e );
extern char * strlocaltime ( double e );
extern char * strlocalydtime ( double e );
extern char * strlocaldate ( double e );
extern char *epoch2str ( double epoch, char *f );
extern char *zepoch2str ( double epoch, char *f, char *timezone );
extern int fmttime ( double epoch, char *result, int fmt, char *timezone );
extern char * strtdelta ( double e );

extern double epoch_ ( int *f_yd );
extern int yearday_ ( double *f_e );
extern void strtime_ ( char *Result, double *f_e, int n_Result );
extern void strdate_ ( char *Result, double *f_e, int n_Result );
extern void epoch2str_ ( char *Result, double *f_e, char *f, int n_Result, int n_f );
extern double str2epoch_ ( char *fs, int n_s );

extern void bcdtime ( unsigned char *bcd, struct date_time *tstr );
extern void epoch2bcd ( unsigned char *bcd, double epoch );

extern void wait_for_time ( double time, int verbose );
extern void snooze(double howlong) ; 

extern void doy2mday ( int doy, int year, int *month, int *day );
extern void doy2mday_ ( int *doy, int *year, int *month, int *day );
extern int mday2doy ( int year, int month, int day );
extern int mday2doy_ ( int *year, int *month, int *day );
extern double h2e ( int iyear, int iday, int ihour, int imin, double sec );
extern double h2e_ ( int *iyear, int *iday, int *ihour, int *imin, double *sec );
extern void e2h ( double epoch, int *iyear, int *iday, int *ihour, int *imin, double *sec );
extern void e2h_ ( double *epoch, int *iyear, int *iday, int *ihour, int *imin, double *sec );
extern void htoe_ ( int * iyear, int * iday, int * ihour, int * imin, double * sec, double * epoch );
extern void etoh_ ( double *epoch, int * iyear, int * iday, int * ihour, int * imin, double * sec );

extern double dtoepoch ( long date );
extern int isleap ( int year );
extern double timecon ( char *timstr );
extern long todaysdate ( void );
extern int month_day ( struct date_time *dt );
extern int etoh ( struct date_time *dt );
extern int get_nice_times ( double tstart, double tend, int maxincs, double *nstart, double *ninc );

extern int htoe ( struct date_time *dt );
extern int timeprint ( struct date_time *dt );
extern int htoe ( struct date_time *dt );
extern int timeprint ( struct date_time *dt );
extern int zh_today ( struct date_time *dt );
extern int mdtodate ( struct date_time *dt );
extern int time_string2epoch ( char * string, double * time );

extern void switchTZ(char *) ;
extern void restoreTZ(void);
extern int zstr2epoch ( char *s, double *e );
extern int is_epoch_string ( char *s, double *e );
extern double str2epoch ( char *s );

#ifdef  __cplusplus
}
#endif

#endif
