#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>


// include'ing the soundpipe.h header brings in "dr_wav.h" and "md5.h"

#include "soundpipe.h"

//====================================================
// dummy module
//----------------------------------------------------

typedef struct {
    SPFLOAT prm;
} sp_dmod;

int sp_dmod_create(sp_dmod **p) {
    *p = malloc(sizeof(sp_dmod));

    return SP_OK;
}

int sp_dmod_destroy(sp_dmod **p) {
    free(*p);

    return SP_OK;
}

int sp_dmod_init(sp_data *sp, sp_dmod *p) {
    p->prm = 0.0;

    return SP_OK;
}

int sp_dmod_compute(sp_data *sp, sp_dmod *p, SPFLOAT *in, SPFLOAT *out) {
    *out = *in;
 
    return SP_OK;
}

//====================================================

//====================================================
// dummy application
//----------------------------------------------------

// Notes:
// 1) an ftable is a canned waveform table
// 2) not sure what the sp_ftbl members are
// 3) not sure sp->len is but appears to be total samples

typedef struct {
    sp_wavin *wavin;
    sp_wavout *wavout;
    sp_dmod *dmod;
//    sp_ftbl *ft;
} UserData;


// callback routine to process a single sample

void callback(sp_data *sp, void *udata) {
    UserData *ud = udata;
    SPFLOAT in, out, out2;

    sp_wavin_compute(sp, ud->wavin, NULL, &in);

    sp_dmod_compute(sp, ud->dmod, &in, &out);

    sp_wavout_compute(sp, ud->wavout, &out, &out2);
}

void callback2(sp_data *sp, void *udata, SPFLOAT *spin) {
    UserData *ud = udata;
    SPFLOAT in, out;

    in = *spin;

    sp_dmod_compute(sp, ud->dmod, &in, &out);

    *(sp->out) = out;
}

//====================================================


int main(int argc, char **argv) {
    
    // Use getopt to process command line because this metaphor has been ported to Python

    extern char *optarg;
    extern int optind;
    int c, err = 0;
    int debug=0, oflag=0, iflag=0, sflag=0;
    char *ofname, *ifname;
    static const char *optstr = "dso:?";
    static char usage[] = "usage: %s [-ds] [-o name] [name ...]\n";

    while ((c = getopt(argc, argv, optstr)) != -1)
        switch (c) {
            case 'd':
                debug = 1;
                break;
            case 's':
                sflag = 1;
                break;
            case 'o':
                oflag = 1;
                ofname = optarg;
                break;
            case '?':
                err = 1;
                break;
            default:  // should never get here
                break;
       }

    // Check any mandatory options here
    if (err) {
        fprintf(stderr, usage, argv[0]);
        exit(1);
    }

    // Check for optional input file
    // for (; optind < argc; optind++)
    //     printf("argument: \"%s\"\n", argv[optind]);
    if (optind < argc) {
        ifname = argv[optind];
        iflag = 1;
        optind++;
    }
    if(optind < argc) {
        fprintf(stderr, "ignored additional arguments\n");
        fprintf(stderr, usage, argv[0]);
    } 

    // Re-assign stdin and stdout if specified (ignored in this case)

    if(iflag && sflag)
        freopen(ifname, "r", stdin);

    if(oflag && sflag)
        freopen(ofname, "w", stdout);

    // Verify file names supplied if required

    if(!iflag && !sflag) {
        fprintf(stderr, "missing required input file\n");
        fprintf(stderr, usage, argv[0]);
        exit(1);
    }

    if(!oflag && !sflag) {
        fprintf(stderr, "missing required -o output file\n");
        fprintf(stderr, usage, argv[0]);
        exit(1);
    }

    // Display debug info

    if(debug) {
        if(iflag)
            fprintf(stderr, "ifname: \"%s\"\n", ifname);
        else
            fprintf(stderr, "ifname: stdin\n");

        if(oflag)
            fprintf(stderr, "ofname: \"%s\"\n", ofname);
        else
            fprintf(stderr, "ofname: stdout\n");
    } 



    // Minimum data storage required

    UserData ud;			// This is the pipeline configuration
    sp_data *sp;			// Pointer to where (base.c)


    // Create empty soundpipe

    sp_create(&sp);			// Pass in pointer to storage for sp pointer

//    sp_ftbl_create(sp, &ud.ft, 2048);


    // Add I/O to soundpipe

    sp_wavin_create(&ud.wavin);
    sp_wavin_init(sp, ud.wavin, ifname);

    sp_wavout_create(&ud.wavout);
    sp_wavout_init(sp, ud.wavout, ofname);


    // Build and config soundpipe

    sp_dmod_create(&ud.dmod);
    sp_dmod_init(sp, ud.dmod);

/*
    // Run processing in soundpipe
    // 1) "callback" pointer to a function that processes a single sample in all channels
    // 2) "sp_process" processing routine that does all samples

    sp_process_raw(sp, &ud, callback); 
*/
/*
    // Run pipeline based on simplest concept this runs to sp->len
    while(sp->len > 0) {
        callback(sp, &ud);
        sp->len--;
        sp->pos++;
    }
*/
/*
    while(ud.wavin->pos <= ud.wavin->wav.totalSampleCount) {
        callback(sp, &ud);
    }
*/

    // adjust soundpipe length based on file to have best control

    sp->len = ud.wavin->wav.totalSampleCount;

    // data buffers just to structure design

    SPFLOAT din = 0.0;
    SPFLOAT dout = 0.0;
    sp->out = malloc(sizeof(SPFLOAT) * 1);

    // the processing

    while(sp->len > 0) {
        sp_wavin_compute(sp, ud.wavin, NULL, &din);
        callback2(sp, &ud, &din);
        sp_wavout_compute(sp, ud.wavout, sp->out, &dout);
        sp->len--;
        sp->pos++;
    }


    free(sp->out);
    sp->out = 0;   // just to clean up


    // Dismantle soundpipe

    sp_dmod_destroy(&ud.dmod);

    
    // Shutdown I/O

    sp_wavout_destroy(&ud.wavout);
    sp_wavin_destroy(&ud.wavin);


    // Clean up on completion

//    sp_ftbl_destroy(&ud.ft);
    sp_destroy(&sp);

    return 0;  
}
