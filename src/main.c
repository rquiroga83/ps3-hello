/*
 * PS3 Homebrew - RSX Framebuffer + Text + Pad Input + SPE Vector Math
 * Uses PSL1GHT SDK: libgcm_sys, librsx, libio, libsysutil
 *
 * Displays text on screen using a built-in 8x8 bitmap font rendered
 * directly to the RSX framebuffer. Sends a vector calculation to one
 * of the Cell BE's SPEs and displays the results. Reads controller
 * input and exits when the X (cross) button is pressed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

#include <ppu-types.h>
#include <rsx/rsx.h>
#include <rsx/gcm_sys.h>
#include <rsx/mm.h>
#include <sysutil/video.h>
#include <sysutil/sysutil.h>
#include <io/pad.h>
#include <lv2/process.h>
#include <sys/spu.h>
#include <sys/thread.h>

#include "vecmath.h"

/* ---------- constants ---------- */
#define MAX_BUFFERS     2
#define FONT_W          8
#define FONT_H          8

/* ---------- globals ---------- */
static gcmContextData *context = NULL;
static u32 curr_buf = 0;
static u32 res_width, res_height;

typedef struct {
    u32  width;
    u32  height;
    u32  pitch;
    u32 *ptr;
    u32  offset;
} displayBuffer;

static displayBuffer buffers[MAX_BUFFERS];

static int running = 1;

/* ---------- SPE data ---------- */
extern const unsigned int spu_bin[];
extern const unsigned int spu_bin_size;

static vecmath_data_t spe_data __attribute__((aligned(128)));
static int spe_ok = 0;  /* 1 if SPE ran successfully */

/* ================================================================
 *  Minimal 8x8 bitmap font (ASCII 32..126)
 *  Each character is 8 rows, each row is a byte (MSB = left pixel).
 * ================================================================ */
static const u8 font8x8[95][8] = {
    /* 32 ' ' */ {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    /* 33 '!' */ {0x18,0x18,0x18,0x18,0x18,0x00,0x18,0x00},
    /* 34 '"' */ {0x6C,0x6C,0x24,0x00,0x00,0x00,0x00,0x00},
    /* 35 '#' */ {0x24,0x7E,0x24,0x24,0x7E,0x24,0x00,0x00},
    /* 36 '$' */ {0x18,0x3E,0x58,0x3C,0x1A,0x7C,0x18,0x00},
    /* 37 '%' */ {0x62,0x64,0x08,0x10,0x26,0x46,0x00,0x00},
    /* 38 '&' */ {0x30,0x48,0x30,0x56,0x88,0x76,0x00,0x00},
    /* 39 ''' */ {0x18,0x18,0x10,0x00,0x00,0x00,0x00,0x00},
    /* 40 '(' */ {0x08,0x10,0x20,0x20,0x20,0x10,0x08,0x00},
    /* 41 ')' */ {0x20,0x10,0x08,0x08,0x08,0x10,0x20,0x00},
    /* 42 '*' */ {0x00,0x24,0x18,0x7E,0x18,0x24,0x00,0x00},
    /* 43 '+' */ {0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00},
    /* 44 ',' */ {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x10},
    /* 45 '-' */ {0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00},
    /* 46 '.' */ {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00},
    /* 47 '/' */ {0x02,0x04,0x08,0x10,0x20,0x40,0x00,0x00},
    /* 48 '0' */ {0x3C,0x46,0x4A,0x52,0x62,0x3C,0x00,0x00},
    /* 49 '1' */ {0x18,0x38,0x18,0x18,0x18,0x7E,0x00,0x00},
    /* 50 '2' */ {0x3C,0x42,0x04,0x18,0x20,0x7E,0x00,0x00},
    /* 51 '3' */ {0x3C,0x42,0x0C,0x02,0x42,0x3C,0x00,0x00},
    /* 52 '4' */ {0x08,0x18,0x28,0x48,0x7E,0x08,0x00,0x00},
    /* 53 '5' */ {0x7E,0x40,0x7C,0x02,0x42,0x3C,0x00,0x00},
    /* 54 '6' */ {0x1C,0x20,0x7C,0x42,0x42,0x3C,0x00,0x00},
    /* 55 '7' */ {0x7E,0x02,0x04,0x08,0x10,0x10,0x00,0x00},
    /* 56 '8' */ {0x3C,0x42,0x3C,0x42,0x42,0x3C,0x00,0x00},
    /* 57 '9' */ {0x3C,0x42,0x42,0x3E,0x04,0x38,0x00,0x00},
    /* 58 ':' */ {0x00,0x18,0x18,0x00,0x18,0x18,0x00,0x00},
    /* 59 ';' */ {0x00,0x18,0x18,0x00,0x18,0x18,0x10,0x00},
    /* 60 '<' */ {0x04,0x08,0x10,0x20,0x10,0x08,0x04,0x00},
    /* 61 '=' */ {0x00,0x00,0x7E,0x00,0x7E,0x00,0x00,0x00},
    /* 62 '>' */ {0x20,0x10,0x08,0x04,0x08,0x10,0x20,0x00},
    /* 63 '?' */ {0x3C,0x42,0x04,0x08,0x08,0x00,0x08,0x00},
    /* 64 '@' */ {0x3C,0x42,0x5E,0x52,0x5E,0x40,0x3C,0x00},
    /* 65 'A' */ {0x18,0x24,0x42,0x7E,0x42,0x42,0x00,0x00},
    /* 66 'B' */ {0x7C,0x42,0x7C,0x42,0x42,0x7C,0x00,0x00},
    /* 67 'C' */ {0x3C,0x42,0x40,0x40,0x42,0x3C,0x00,0x00},
    /* 68 'D' */ {0x78,0x44,0x42,0x42,0x44,0x78,0x00,0x00},
    /* 69 'E' */ {0x7E,0x40,0x7C,0x40,0x40,0x7E,0x00,0x00},
    /* 70 'F' */ {0x7E,0x40,0x7C,0x40,0x40,0x40,0x00,0x00},
    /* 71 'G' */ {0x3C,0x42,0x40,0x4E,0x42,0x3C,0x00,0x00},
    /* 72 'H' */ {0x42,0x42,0x7E,0x42,0x42,0x42,0x00,0x00},
    /* 73 'I' */ {0x7E,0x18,0x18,0x18,0x18,0x7E,0x00,0x00},
    /* 74 'J' */ {0x1E,0x04,0x04,0x04,0x44,0x38,0x00,0x00},
    /* 75 'K' */ {0x44,0x48,0x70,0x48,0x44,0x42,0x00,0x00},
    /* 76 'L' */ {0x40,0x40,0x40,0x40,0x40,0x7E,0x00,0x00},
    /* 77 'M' */ {0x42,0x66,0x5A,0x42,0x42,0x42,0x00,0x00},
    /* 78 'N' */ {0x42,0x62,0x52,0x4A,0x46,0x42,0x00,0x00},
    /* 79 'O' */ {0x3C,0x42,0x42,0x42,0x42,0x3C,0x00,0x00},
    /* 80 'P' */ {0x7C,0x42,0x42,0x7C,0x40,0x40,0x00,0x00},
    /* 81 'Q' */ {0x3C,0x42,0x42,0x4A,0x44,0x3A,0x00,0x00},
    /* 82 'R' */ {0x7C,0x42,0x42,0x7C,0x44,0x42,0x00,0x00},
    /* 83 'S' */ {0x3C,0x40,0x3C,0x02,0x42,0x3C,0x00,0x00},
    /* 84 'T' */ {0x7E,0x18,0x18,0x18,0x18,0x18,0x00,0x00},
    /* 85 'U' */ {0x42,0x42,0x42,0x42,0x42,0x3C,0x00,0x00},
    /* 86 'V' */ {0x42,0x42,0x42,0x24,0x24,0x18,0x00,0x00},
    /* 87 'W' */ {0x42,0x42,0x42,0x5A,0x66,0x42,0x00,0x00},
    /* 88 'X' */ {0x42,0x24,0x18,0x18,0x24,0x42,0x00,0x00},
    /* 89 'Y' */ {0x42,0x42,0x24,0x18,0x18,0x18,0x00,0x00},
    /* 90 'Z' */ {0x7E,0x04,0x08,0x10,0x20,0x7E,0x00,0x00},
    /* 91 '[' */ {0x3C,0x20,0x20,0x20,0x20,0x3C,0x00,0x00},
    /* 92 '\' */ {0x40,0x20,0x10,0x08,0x04,0x02,0x00,0x00},
    /* 93 ']' */ {0x3C,0x04,0x04,0x04,0x04,0x3C,0x00,0x00},
    /* 94 '^' */ {0x10,0x28,0x44,0x00,0x00,0x00,0x00,0x00},
    /* 95 '_' */ {0x00,0x00,0x00,0x00,0x00,0x00,0x7E,0x00},
    /* 96 '`' */ {0x18,0x18,0x08,0x00,0x00,0x00,0x00,0x00},
    /* 97 'a' */ {0x00,0x00,0x3C,0x02,0x3E,0x42,0x3E,0x00},
    /* 98 'b' */ {0x40,0x40,0x7C,0x42,0x42,0x42,0x7C,0x00},
    /* 99 'c' */ {0x00,0x00,0x3C,0x40,0x40,0x40,0x3C,0x00},
    /*100 'd' */ {0x02,0x02,0x3E,0x42,0x42,0x42,0x3E,0x00},
    /*101 'e' */ {0x00,0x00,0x3C,0x42,0x7E,0x40,0x3C,0x00},
    /*102 'f' */ {0x0C,0x10,0x3C,0x10,0x10,0x10,0x10,0x00},
    /*103 'g' */ {0x00,0x00,0x3E,0x42,0x42,0x3E,0x02,0x3C},
    /*104 'h' */ {0x40,0x40,0x7C,0x42,0x42,0x42,0x42,0x00},
    /*105 'i' */ {0x18,0x00,0x38,0x18,0x18,0x18,0x3C,0x00},
    /*106 'j' */ {0x04,0x00,0x04,0x04,0x04,0x04,0x44,0x38},
    /*107 'k' */ {0x40,0x40,0x44,0x48,0x70,0x48,0x44,0x00},
    /*108 'l' */ {0x38,0x18,0x18,0x18,0x18,0x18,0x3C,0x00},
    /*109 'm' */ {0x00,0x00,0x66,0x5A,0x5A,0x42,0x42,0x00},
    /*110 'n' */ {0x00,0x00,0x7C,0x42,0x42,0x42,0x42,0x00},
    /*111 'o' */ {0x00,0x00,0x3C,0x42,0x42,0x42,0x3C,0x00},
    /*112 'p' */ {0x00,0x00,0x7C,0x42,0x42,0x7C,0x40,0x40},
    /*113 'q' */ {0x00,0x00,0x3E,0x42,0x42,0x3E,0x02,0x02},
    /*114 'r' */ {0x00,0x00,0x5C,0x62,0x40,0x40,0x40,0x00},
    /*115 's' */ {0x00,0x00,0x3E,0x40,0x3C,0x02,0x7C,0x00},
    /*116 't' */ {0x10,0x10,0x7C,0x10,0x10,0x10,0x0C,0x00},
    /*117 'u' */ {0x00,0x00,0x42,0x42,0x42,0x42,0x3E,0x00},
    /*118 'v' */ {0x00,0x00,0x42,0x42,0x24,0x24,0x18,0x00},
    /*119 'w' */ {0x00,0x00,0x42,0x42,0x5A,0x5A,0x66,0x00},
    /*120 'x' */ {0x00,0x00,0x42,0x24,0x18,0x24,0x42,0x00},
    /*121 'y' */ {0x00,0x00,0x42,0x42,0x42,0x3E,0x02,0x3C},
    /*122 'z' */ {0x00,0x00,0x7E,0x04,0x18,0x20,0x7E,0x00},
    /*123 '{' */ {0x0C,0x10,0x10,0x20,0x10,0x10,0x0C,0x00},
    /*124 '|' */ {0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x00},
    /*125 '}' */ {0x30,0x08,0x08,0x04,0x08,0x08,0x30,0x00},
    /*126 '~' */ {0x00,0x32,0x4C,0x00,0x00,0x00,0x00,0x00},
};

/* ================================================================
 *  System event callback (handles XMB quit requests)
 * ================================================================ */
static void sysutil_callback(u64 status, u64 param, void *usrdata)
{
    (void)param;
    (void)usrdata;

    switch (status) {
    case SYSUTIL_EXIT_GAME:
        running = 0;
        break;
    default:
        break;
    }
}

/* ================================================================
 *  RSX video / framebuffer helpers
 * ================================================================ */

/* Wait for the RSX to finish flipping the previous frame */
static void waitFlip(void)
{
    while (gcmGetFlipStatus() != 0)
        usleep(200);
    gcmResetFlipStatus();
}

/* Request a flip to the given buffer and wait */
static void flip(u32 buffer)
{
    gcmSetFlip(context, (u8)buffer);
    rsxFlushBuffer(context);
    gcmSetWaitFlip(context);
}

/* Allocate a framebuffer in RSX memory and register it with GCM */
static void makeBuffer(u32 id, u32 width, u32 height)
{
    u32 pitch = width * sizeof(u32);        /* 4 bytes per pixel, XRGB */
    u32 size  = pitch * height;

    buffers[id].ptr    = (u32 *)rsxMemalign(64, size);
    buffers[id].width  = width;
    buffers[id].height = height;
    buffers[id].pitch  = pitch;

    rsxAddressToOffset(buffers[id].ptr, &buffers[id].offset);
    gcmSetDisplayBuffer(id, buffers[id].offset, pitch, width, height);
}

/* Detect the current video resolution and set up double-buffered display */
static void initScreen(void)
{
    videoState state;
    videoResolution resolution;
    videoConfiguration vconfig;
    void *host_addr;

    /* Allocate 1 MB of host memory for the RSX command buffer */
    host_addr = memalign(1024 * 1024, 1024 * 1024);
    context   = rsxInit(0x10000, 1024 * 1024, host_addr);

    /* Query current video output state */
    videoGetState(0, 0, &state);

    /* Get pixel dimensions for the active resolution */
    videoGetResolution(state.displayMode.resolution, &resolution);
    res_width  = resolution.width;
    res_height = resolution.height;

    /* Configure video output */
    memset(&vconfig, 0, sizeof(videoConfiguration));
    vconfig.resolution = state.displayMode.resolution;
    vconfig.format     = VIDEO_BUFFER_FORMAT_XRGB;
    vconfig.pitch      = res_width * sizeof(u32);
    videoConfigure(0, &vconfig, NULL, 0);
    videoGetState(0, 0, &state);

    gcmSetFlipMode(GCM_FLIP_VSYNC);

    /* Create two framebuffers for double buffering */
    makeBuffer(0, res_width, res_height);
    makeBuffer(1, res_width, res_height);

    gcmResetFlipStatus();
    flip(curr_buf);
}

/* ================================================================
 *  Drawing primitives (software rasterisation to framebuffer)
 * ================================================================ */

/* Fill the entire framebuffer with a solid colour (XRGB) */
static void clearScreen(u32 color)
{
    u32 *fb    = buffers[curr_buf].ptr;
    u32  count = buffers[curr_buf].width * buffers[curr_buf].height;
    u32  i;
    for (i = 0; i < count; i++)
        fb[i] = color;
}

/* Draw a single character at pixel position (px, py) with scale factor */
static void drawChar(u32 *fb, u32 pitch, char c, u32 px, u32 py,
                     u32 fg, u32 scale)
{
    const u8 *glyph;
    u32 row, col, sy, sx;

    if (c < 32 || c > 126)
        c = '?';
    glyph = font8x8[c - 32];

    for (row = 0; row < FONT_H; row++) {
        u8 bits = glyph[row];
        for (col = 0; col < FONT_W; col++) {
            if (bits & (0x80 >> col)) {
                /* scale the pixel */
                for (sy = 0; sy < scale; sy++) {
                    for (sx = 0; sx < scale; sx++) {
                        u32 x = px + col * scale + sx;
                        u32 y = py + row * scale + sy;
                        if (x < res_width && y < res_height)
                            fb[y * (pitch / 4) + x] = fg;
                    }
                }
            }
        }
    }
}

/* Draw a null-terminated string at (x, y) with given colour and scale */
static void drawString(const char *str, u32 x, u32 y, u32 color, u32 scale)
{
    u32 *fb    = buffers[curr_buf].ptr;
    u32  pitch = buffers[curr_buf].pitch;
    u32  cx    = x;

    while (*str) {
        if (*str == '\n') {
            cx = x;
            y += FONT_H * scale + 2;
        } else {
            drawChar(fb, pitch, *str, cx, y, color, scale);
            cx += FONT_W * scale;
        }
        str++;
    }
}

/* ================================================================
 *  Main
 * ================================================================ */
int main(int argc, const char *argv[])
{
    padInfo  padinfo;
    padData  paddata;
    u32      frame = 0;

    (void)argc;
    (void)argv;

    /* Initialise subsystems */
    initScreen();
    ioPadInit(7);                          /* support up to 7 pads */
    sysUtilRegisterCallback(0, sysutil_callback, NULL);

    printf("RSX framebuffer text demo started\n");

    /* ---- Run SPE vector calculation ---- */
    {
        sysSpuImage spu_image;
        u32 group_id;
        u32 thread_id;
        u32 cause, status;
        s32 ret;

        /* Fill input vector: (1.0, 2.0, 3.0, 4.0) */
        spe_data.input[0] = 1.0f;
        spe_data.input[1] = 2.0f;
        spe_data.input[2] = 3.0f;
        spe_data.input[3] = 4.0f;
        spe_data.done = 0;

        printf("SPE: sizeof(vecmath_data_t)=%u addr=%p\n",
               (unsigned int)sizeof(vecmath_data_t), &spe_data);

        /* Initialize SPU subsystem (6 SPEs available on Cell BE) */
        ret = sysSpuInitialize(6, 0);
        printf("SPE: sysSpuInitialize ret=%d\n", ret);

        /* Load SPU image from embedded binary */
        ret = sysSpuImageImport(&spu_image, spu_bin, 0);
        printf("SPE: sysSpuImageImport ret=%d entry=0x%x segs=%u\n",
               ret, spu_image.entryPoint, spu_image.segmentCount);

        /* Create thread group */
        sysSpuThreadGroupAttribute grpattr;
        memset(&grpattr, 0, sizeof(grpattr));
        grpattr.nameSize = 7;
        grpattr.nameAddress = (u32)(uintptr_t)"spugrp";
        grpattr.groupType = 0;
        grpattr.memContainer = 0;
        ret = sysSpuThreadGroupCreate(&group_id, 1, 100, &grpattr);
        printf("SPE: sysSpuThreadGroupCreate ret=%d group=%u\n", ret, group_id);

        /* Create SPU thread */
        sysSpuThreadAttribute thattr;
        memset(&thattr, 0, sizeof(thattr));
        thattr.nameAddress = (u32)(uintptr_t)"sputhr";
        thattr.nameSize = 7;
        thattr.attribute = SPU_THREAD_ATTR_NONE;

        sysSpuThreadArgument arg;
        arg.arg0 = (u64)(uintptr_t)&spe_data;
        arg.arg1 = 0;
        arg.arg2 = 0;
        arg.arg3 = 0;
        ret = sysSpuThreadInitialize(&thread_id, group_id, 0, &spu_image, &thattr, &arg);
        printf("SPE: sysSpuThreadInitialize ret=%d thread=%u\n", ret, thread_id);

        /* Start and wait for completion */
        ret = sysSpuThreadGroupStart(group_id);
        printf("SPE: sysSpuThreadGroupStart ret=%d\n", ret);

        printf("SPE: waiting for join...\n");
        ret = sysSpuThreadGroupJoin(group_id, &cause, &status);
        printf("SPE: sysSpuThreadGroupJoin ret=%d cause=%u status=%u\n", ret, cause, status);

        if (spe_data.done) {
            spe_ok = 1;
            printf("SPE done: dot=%.2f mag=%.2f\n", spe_data.dot_product, spe_data.magnitude);
        } else {
            printf("SPE did not complete (done=%u)\n", spe_data.done);
        }

        sysSpuImageClose(&spu_image);
    }

    /* Main loop */
    while (running) {
        /* Check for system events (XMB quit, etc.) */
        sysUtilCheckCallback();

        /* Poll controller */
        ioPadGetInfo(&padinfo);
        if (padinfo.status[0]) {
            ioPadGetData(0, &paddata);

            if (paddata.BTN_CROSS) {
                running = 0;
            }
        }

        /* ---- Render frame ---- */
        waitFlip();

        /* Dark blue background */
        clearScreen(0x00102040);

        /* Title (scale 4x) */
        drawString("Hola Mundo PS3!", 80, 60, 0x00FFFFFF, 4);

        /* Subtitle (scale 2x) */
        drawString("RSX framebuffer + bitmap font demo", 80, 130, 0x0000CC00, 2);

        /* Instructions */
        drawString("Press X (cross) to exit", 80, 180, 0x00CCCCCC, 2);

        /* Frame counter */
        {
            char info[64];
            sprintf(info, "Frame: %u", frame);
            drawString(info, 80, 240, 0x00AAAAAA, 2);
        }

        /* Resolution info */
        {
            char res[64];
            sprintf(res, "Resolution: %ux%u", res_width, res_height);
            drawString(res, 80, 280, 0x00AAAAAA, 2);
        }

        /* SPE results */
        if (spe_ok) {
            char buf[128];

            drawString("--- SPE Vector Math ---", 80, 340, 0x00FFD700, 2);

            sprintf(buf, "Input:  (%.1f, %.1f, %.1f, %.1f)",
                    spe_data.input[0], spe_data.input[1],
                    spe_data.input[2], spe_data.input[3]);
            drawString(buf, 80, 370, 0x0099CCFF, 2);

            sprintf(buf, "Output: (%.1f, %.1f, %.1f, %.1f)",
                    spe_data.output[0], spe_data.output[1],
                    spe_data.output[2], spe_data.output[3]);
            drawString(buf, 80, 400, 0x0099CCFF, 2);

            sprintf(buf, "Dot product: %.2f", spe_data.dot_product);
            drawString(buf, 80, 430, 0x0099CCFF, 2);

            sprintf(buf, "Magnitude:   %.2f", spe_data.magnitude);
            drawString(buf, 80, 460, 0x0099CCFF, 2);
        } else {
            drawString("SPE: not available", 80, 340, 0x00FF4444, 2);
        }

        /* Flip to display the frame we just drew */
        flip(curr_buf);
        curr_buf = !curr_buf;
        frame++;
    }

    /* Clean up */
    printf("Exiting...\n");
    ioPadEnd();
    gcmSetWaitFlip(context);
    rsxFinish(context, 1);

    sysProcessExit(0);
    return 0;
}
