#ifndef _tttaup_
#define _ttaup_

#ifdef __STDC__
#define PL_(x) x
#else
#define PL_(x) ( )
#endif /* __STDC__ */

#define ptime(DELTA,DEPTH) pphasetime((DELTA),(DEPTH))
#define stime(DELTA,DEPTH) sphasetime((DELTA),(DEPTH))

extern int arrtimes PL_(( double delta, double depth, char ***phases, double **times ));
extern double phasetime PL_(( char *phase, double delta, double depth ));
extern double pphasetime PL_(( double delta, double depth ));
extern double sphasetime PL_(( double delta, double depth ));
extern int arrtimes_ PL_(( double *f_delta, double *f_depth, char *f_phases, double *f_times, int n_phases ));
extern double phasetime_ PL_(( char *f_phase, double *f_delta, double *f_depth, int n_phase ));
extern double pphasetime_ PL_(( double *f_delta, double *f_depth ));
extern double sphasetime_ PL_(( double *f_delta, double *f_depth ));
extern double ptime_ PL_(( double *f_delta, double *f_depth ));
extern double stime_ PL_(( double *f_delta, double *f_depth ));
extern int arr_slowness PL_(( double delta, double depth, char ***phases, double **slowness ));
extern double phase_slowness PL_(( char *phase, double delta, double depth ));
extern double pphase_slowness PL_(( double delta, double depth ));
extern double sphase_slowness PL_(( double delta, double depth ));
extern int tt_taup_set_table PL_(( char *table_path ));
extern int tt_taup_get_modname PL_(( char *modname ));
extern int taup_setup PL_(( char *model, char *phases ));
extern int tt_taup_set_phases PL_(( char * phases ));
extern int tt_taup_set_event_depth PL_(( double edepth ));
extern int tt_taup PL_(( double elat, double elon, double slat, double slon, double selev, double vp_cor, double vs_cor, int * nph, double ** times, char *** phs ));
extern int tt_taup_slowness PL_(( double elat, double elon, double slat, double slon, double selev, double vp_cor, double vs_cor, int * nph, double ** times, double ** slowness, double ** zslowness, double ** dslowness, char *** phs ));
extern int tt_taup_p PL_(( double elat, double elon, double slat, double slon, double selev, double vp_cor, double vs_cor, int * nph, double ** times, double ** ps, double ** dtdhs, char *** phs ));
extern void tt_taup_set_table_ PL_(( char *table_path, int len_table_path ));
extern void tt_taup_set_phases_ PL_(( char *phases, int len_phases ));
extern void tt_taup_set_event_depth_ PL_(( double *edepth ));
extern void tt_taup_ PL_(( double *elat, double *elon, double *slat, double *slon, double *selev, double *vp_cor, double *vs_cor, int * nph, double * times, char * phs, int len_phs ));
extern char * tt_strf2c PL_(( char *string, int len ));

#undef PL_
#endif 

/* $Id: tttaup.h,v 1.1.1.1 1997/04/12 04:18:48 danq Exp $ */
