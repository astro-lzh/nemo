/* --------------------------------------------------------- *\
|* $Id$
|*
|* Functions prototypes
|*
\* --------------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif

int put_data_select(char * ,
		    int    ,
		    char * a[],
		    bool *    ,
                    FILE * c[],
		    int     );

int get_data_select(char * ,
		    int    ,
		    char * a[],
		    bool *    ,
                    FILE * c[],
		    int     );


#ifdef __cplusplus
}
#endif
