#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>


// include'ing the soundpipe.h header brings in "dr_wav.h" and "md5.h"

#include "soundpipe.h"

//-------------------------------------------------------------

// function to split filenames into parts

//char **splitpath(char *fpath, char *fprefix, char *fsuffix, char *fparts[4], char *fname) {

char **splitpath(char *fparts[4], char *fname) {

    char *fnamedc, *fnamebc, *fdname, *fbname;
    char fpname[strlen(fname) + 2];
    char fsname[strlen(fname) + 2];
    char *p;

    // get the directory part and adjust for no file name

    fnamedc = strdup(fname);
    fdname = dirname(fnamedc);
    fnamebc = strdup(fname);
    fbname = basename(fnamebc);

    if(fname[strlen(fname) - 1] == '/') {
        strcat(fdname, "/");
        strcat(fdname, fbname);
        fbname = "dummy";
    }
 
//    *fpath = '\0';
//    strncat(fpath, fdname, strlen(fname));
    *(fparts[0]) ='\0';
    strncat(fparts[0], fdname, strlen(fname));

    // get the file prefix

    *fpname = '\0';
    p = strrchr(fbname, '.');
    strncat(fpname, fbname, (int)(p - fbname));

//    *fprefix = '\0';
//    strncat(fprefix, fpname, strlen(fname));
    *(fparts[1]) = '\0';
    strncat(fparts[1], fpname, strlen(fname));
        
    // get the file suffix

    *fsname = '\0';
    if(p) {
        strcat(fsname, p+1);
    }

//    *fsuffix = '\0';
//    strncat(fsuffix, fsname, strlen(fname));
    *(fparts[2]) = '\0';
    strncat(fparts[2], fsname, strlen(fname));

    return(fparts);
}

char *joinpath(char *fpath, char *fparts[4]) {
    
    *fpath = '\0';
    strcat(fpath, fparts[0]);
    strcat(fpath, fparts[1]);
    strcat(fpath, fparts[2]);
    strcat(fpath, fparts[3]);

    return(fpath);
}

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

#define CHANS 4
typedef struct {
    sp_wavin *wavin;
    sp_wavout *wavout[(int)CHANS];
    sp_dmod *dmod;
//    sp_ftbl *ft;
} UserData;


// callback routine to process a single sample

void callback(sp_data *sp, void *udata, SPFLOAT *spin) {
    UserData *ud = udata;
    SPFLOAT in, out;
    int chan;

    in = *spin;

    for(chan = 0; chan < sp->nchan; chan++) {
        sp_dmod_compute(sp, ud->dmod, &in, &out);
        sp->out[chan] = out;
    }
}

//====================================================

//====================================================
// example main
//----------------------------------------------------

int main(int argc, char **argv) {
    
    // Use getopt to process command line because this has been ported to Python

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

    //-------------------------------------------------------------

    // Minimum data storage required

    UserData ud;			// This is the pipeline configuration
    sp_data *sp;			// Pointer to where (base.c)
    int chan;

    char *ofparts[4];
    char ofpath[strlen(ofname)+2]; ofparts[0] = ofpath;
    char ofprefix[strlen(ofname)+2]; ofparts[1] = ofprefix;
    char ofsuffix[strlen(ofname)+2]; ofparts[2] = ofsuffix;
    char ofnum[strlen(ofname)+2]; ofparts[3] = ofnum;

    char ofnamebfr[CHANS][strlen(ofname)+10];
    char *ofnamec[CHANS];
    for(chan = 0; chan < (int)CHANS; chan++) {
        ofnamec[chan] = &(ofnamebfr[chan][0]);
    }

//    splitpath(ofpath, ofprefix, &(ofsuffix[1]), ofparts, ofname);
    splitpath(ofparts, ofname);


    printf("suffix = %s\n", ofparts[2]);

    strcat(ofparts[0], "/");
    strcat(ofparts[1], "_");
    ofparts[3][0] = '\0';
    strcat(ofparts[3], ".");
    strcat(ofparts[3], ofparts[2]);
    sprintf(ofparts[2], "%d", 0);
/*
    printf("path = %s\n", ofparts[0]);
    printf("prefix = %s\n", ofparts[1]);
    printf("number = %s\n", ofparts[2]);
    printf("suffix = %s\n", ofparts[3]);
*/
    joinpath(ofnamec[0], ofparts);
/*
    printf("name = %s\n", ofnamec[0]);
*/
    // Create empty soundpipe

    sp_create(&sp);			// Pass in pointer to storage for sp pointer

//    sp_ftbl_create(sp, &ud.ft, 2048);


    // Add I/O to soundpipe

    sp_wavin_create(&ud.wavin);
    sp_wavin_init(sp, ud.wavin, ifname);

    for(chan = 0; chan < (int)CHANS; chan++) {
        sp_wavout_create(&(ud.wavout[chan]));
        sprintf(ofparts[2], "%d", chan);
        joinpath(ofnamec[chan], ofparts);
        printf("%s\n", ofnamec[chan]);
        sp_wavout_init(sp, ud.wavout[chan], ofnamec[chan]);
    }


    // Build and config soundpipe

    sp_dmod_create(&ud.dmod);
    sp_dmod_init(sp, ud.dmod);


    // adjust soundpipe length based on file to have best control

    sp->len = ud.wavin->wav.totalSampleCount;


    // data buffers just to structure design

    SPFLOAT din;
    SPFLOAT dout[CHANS];
    sp->out = malloc(sizeof(SPFLOAT) * (int)CHANS);
    sp->nchan = (int)CHANS;


    // the processing

    while(sp->len > 0) {
        sp_wavin_compute(sp, ud.wavin, NULL, &din);

        callback(sp, &ud, &din);

        for(chan = 0; chan < sp->nchan; chan++) {
            sp_wavout_compute(sp, ud.wavout[chan], &(sp->out[chan]), &(dout[chan]));
        }

        sp->len--;
        sp->pos++;
    }

    free(sp->out);
    sp->out = 0;   // just to clean up


    // Dismantle soundpipe

    sp_dmod_destroy(&ud.dmod);

    
    // Shutdown I/O

    for(chan = 0; chan < sp->nchan; chan++) {
        sp_wavout_destroy(&(ud.wavout[chan]));
    }

    sp_wavin_destroy(&ud.wavin);


    // Clean up on completion

//    sp_ftbl_destroy(&ud.ft);
    sp_destroy(&sp);

    return 0;  
}
