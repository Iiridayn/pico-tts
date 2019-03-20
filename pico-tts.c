#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "picoapi.h"

const int MEM_SIZE = 1024 * 1024 * 5;
const int BUF_SIZE = 16000;

// Error handling.

#define CHECKED_SYS(RET)   checked_sys(picoSys, RET, 0)
void checked_sys(pico_System picoSys, pico_Status ret, int depth)
{
    // We are going to just assume call related to warnings don't error,
    // since missing warnings isn't the end of the world anyways
    int num_warnings = 0;
    pico_getNrSystemWarnings(
        picoSys,
        &num_warnings
    );

    for( pico_Int32 i = 0; i < num_warnings; i++ ) {
        pico_Retstring message;
        pico_Status code;

        pico_getSystemWarning(
            picoSys,
            i,
            &code,
            message
        );

        fprintf(stderr, "Pico warning %d: %s\n", code, message);
    }


    if( ret != 0 ) {
        // Recursive error handling may not be the best idea ever
        if( depth > 3 ) {
            fprintf(stderr, "Recursive errors\n");
            exit(1);
        }

        pico_Retstring message;

        checked_sys(
            picoSys,
            pico_getSystemStatusMessage(
                picoSys,
                ret,
                message
            ),
            depth + 1
        );

        fprintf(stderr, "pico error %d: %s\n", ret, message);
        exit(1);
    }
}

// This is all just copy and pasted from the above with picoSys replaced by engine.
#define CHECKED_ENGINE(RET)   checked_engine(engine, RET, 0)
void checked_engine(pico_Engine engine, pico_Status ret, int depth)
{
    // We are going to just assume call related to warnings don't error,
    // since missing warnings isn't the end of the world anyways
    int num_warnings = 0;
    pico_getNrEngineWarnings(
        engine,
        &num_warnings
    );

    for( pico_Int32 i = 0; i < num_warnings; i++ ) {
        pico_Retstring message;
        pico_Status code;

        pico_getEngineWarning(
            engine,
            i,
            &code,
            message
        );

        fprintf(stderr, "Pico warning %d: %s\n", code, message);
    }


    if( ret != 0 ) {
        // Recursive error handling may not be the best idea ever
        if( depth > 3 ) {
            fprintf(stderr, "Recursive errors\n");
            exit(1);
        }

        pico_Retstring message;

        checked_engine(
            engine,
            pico_getEngineStatusMessage(
                engine,
                ret,
                message
            ),
            depth + 1
        );

        fprintf(stderr, "pico error %d: %s\n", ret, message);
        exit(1);
    }
}

typedef struct {
    const pico_Char * name;
    const pico_Char * ta_file;
    const pico_Char * sg_file;
} voice_t;
#define ADD_LANG(NAME, SG) { (pico_Char *) NAME, (pico_Char *) NAME"_ta.bin", (pico_Char *) NAME"_"SG"0_sg.bin" }
static const voice_t voices[] = {
    ADD_LANG("de-DE", "gl"), ADD_LANG("en-GB", "kh"), ADD_LANG("en-US", "lh"),
    ADD_LANG("es-ES", "zl"), ADD_LANG("fr-FR", "nk"), ADD_LANG("it-IT", "cm"),
};
static const int voice_count = sizeof (voices) / sizeof (voice_t);

static void usage(char * name)
{
    fprintf(stderr, "Usage:\n%s [-l language]\n-l\tLanguage - one of de-DE, en-GB, (en-US), es-ES, fr-FR, it-IT\n", name);
    exit(0);
}

/*
From picoapi.h:
- pico_initialize
- pico_loadResource
- pico_createVoiceDefinition
- pico_addResourceToVoiceDefinition
- pico_newEngine
- pico_putTextUtf8
- pico_getData (several times)
- pico_disposeEngine
- pico_releaseVoiceDefinition
- pico_unloadResource
- pico_terminate
*/
int main(int argc, char **argv)
{
    const char * input = "en-US";

    char currentOption;
    extern char *optarg;
    while ( (currentOption = getopt(argc, argv, "l:h")) != -1) {
        switch (currentOption) {
            case 'l':
                input = optarg;
                break;
            case 'h':
                usage(argv[0]);
                break;
            default:
                fprintf(stderr, "Getopt returned character code 0%o ??\n", currentOption);
        }
    }

    // TODO: no good - these and the .so need to be distributed somewhere common
    chdir("svox/pico/lang");

    const voice_t * selected = NULL;
    for (int i = 0; i < voice_count; i++) {
        if (0 == strcmp(input, (const char *) voices[i].name)) { // flawfinder: ignore not making an array
            selected = &voices[i];
            break;
        }
    }
    if (selected == NULL) {
        usage(argv[0]);
    }

    // Pico asks us to initialize memory, and won't otherwise allocate memory.
    // Unfortunately it doesn't tell us exactly how much memory it's going to need.
    // The documentation suggests they had a target of 200KB when creating it,
    // but it crashes at 1MB. So we just give it 5MB.

    void * picoMem = malloc(MEM_SIZE);
    short *buf = malloc(BUF_SIZE);

    pico_System picoSys;
    // Defined as an 'out' variable in the header, this initializes picoSys
    CHECKED_SYS(pico_initialize(picoMem, MEM_SIZE, &picoSys));

    // Resource files include the actual data needed for tts.

    // For whatever reason the resource is sometimes referred to by name instead of by handle
    // "Text analysis" lingware resource file
    pico_Resource res_ta;
    pico_Retstring res_ta_name;
    CHECKED_SYS(pico_loadResource(picoSys, selected->ta_file, &res_ta));
    CHECKED_SYS(pico_getResourceName(picoSys, res_ta, res_ta_name));
    //fprintf(stderr, "Loaded resource with name: '%s'\n", res_ta_name);

    // "Signal generation" lingware resource file
    pico_Resource res_sg;
    pico_Retstring res_sg_name;
    CHECKED_SYS(pico_loadResource(picoSys, selected->sg_file, &res_sg));
    CHECKED_SYS(pico_getResourceName(picoSys, res_sg, res_sg_name));
    //fprintf(stderr, "Loaded resource with name: '%s'\n", res_sg_name);

    // This creates a name which we can use for later reference
    CHECKED_SYS(pico_createVoiceDefinition(picoSys, selected->name));

    // This attaches voice data to the name
    CHECKED_SYS(pico_addResourceToVoiceDefinition(picoSys, selected->name, (pico_Char *)res_ta_name));
    CHECKED_SYS(pico_addResourceToVoiceDefinition(picoSys, selected->name, (pico_Char *)res_sg_name));

    // Note that if you incorrectly load resourceData
    // (i.e. only load ta) this segfautls in the error
    // handling code, but gdb will show you the error
    // message if you compile everything with debugging
    // symbols.
    pico_Engine engine;
    CHECKED_SYS(pico_newEngine(picoSys, selected->name, &engine));

    char * line = NULL;
    size_t len = 0;

    ssize_t bytes;
    while ((bytes = getline(&line, &len, stdin)) != -1) {
        // In practice can't handle a line length greater than 260
        pico_Int16 bytes_copied;
        CHECKED_ENGINE(pico_putTextUtf8(engine, (pico_Char*) line, bytes + 1, &bytes_copied));

        //fprintf(stderr, "bytes copied: %d/%d\n", bytes_copied, bytes + 1);

        int buf_len = 0;

        pico_Int16 bytes_received = 0;
        pico_Int16 dtype = 0;
        pico_Status ret;
        do {
            ret = pico_getData(engine, buf + buf_len, BUF_SIZE, &bytes_received, &dtype);

            // We are assuming that we get shorts back... which is currently
            // the only supported dtype anyways but we might as well check.
            if( dtype != PICO_DATA_PCM_16BIT ) {
                fprintf(stderr, "Unknown dtype\n");
                exit(1);
            }

            buf_len += bytes_received / 2;

            fwrite(buf, buf_len, 2, stdout);
            fflush(stdout);
            buf_len = 0;
        } while( ret == PICO_STEP_BUSY );
    }


    // Cleanup here, but program termination will do it for us anyways.
    free(line);
    pico_disposeEngine(picoSys, &engine);
    pico_releaseVoiceDefinition(picoSys, selected->name);
    pico_unloadResource(picoSys, &res_ta);
    pico_unloadResource(picoSys, &res_sg);
    pico_terminate(&picoSys);
    free(picoMem);
    free(buf);

    return 0;
}
