#include <fontconfig/fontconfig.h>
#include <X11/Xft/Xft.h>
#include <X11/Xutil.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>
#define BUFFER_SIZE 1024

void set_fullscreen(Display *dpy, Window win);
char* get_sstring(int mul, const char* instr, int base);
void mem_cleanup(Display *display, XftFont **font, XftFont **font_small, FcPattern **pattern, FcPattern **pattern_small, FcPattern **match, FcPattern **match_small, char **fonto, char **fonto_small);

int main() {
 // define base vars
 Display *display; // xdisplay
 Window window; // xwindow
 XEvent e; // xevent? (redo docs after understanding xlib)
 int s; // xscreen?
 display = XOpenDisplay(NULL); // attempt to open x display
 if(display == NULL) { // error checking
  printf("failed to open display\n");
  return 1;
 }
 char fontn[] = "BMSPA.TTF";
 int sbase = 95; int sbase_small = 25;
 char fontsn[] = "BM space"; // font style base
 // char fonto[] = "BM space:size=95"; // base 95
 // char fonto_small[] = "BM space:size=25"; // base 25 // yoff base 100
 int fullscreen = 0;
 int global_scale = 1;
 char cwd[PATH_MAX]; char fd[PATH_MAX];
 if(getcwd(cwd, sizeof(cwd)) == NULL) perror(":( couldnt grab cwd");
 // build font path
 strcpy(fd, cwd); strcat(fd, "/font/"); strcat(fd, fontn);
 // set up base xlib stuffs
 s = DefaultScreen(display); // main screen
 window = XCreateSimpleWindow(display, RootWindow(display, s), 100, 100, 1000, 1000, 1, BlackPixel(display, s), WhitePixel(display, s)); // main windo
 XSelectInput(display, window, ExposureMask | KeyPressMask); // scan 4 sumthing
 XMapWindow(display, window); // i think map out window to write text
 XStoreName(display, window, "NerdClock;v1.0"); // write window title
 // set up xft drawing
 XftDraw *draw = XftDrawCreate(display, window, DefaultVisual(display, s), DefaultColormap(display, s));
 FcInit(); // initilize fontconfig
 // init style string for base draw
 char *fonto = get_sstring(global_scale, fontsn, sbase);
 char *fonto_small = get_sstring(global_scale, fontsn, sbase_small);
 // load custom font file
 FcConfigAppFontAddFile(FcConfigGetCurrent(), (const FcChar8 *)fd);
 FcPattern *pattern = FcNameParse((const FcChar8 *)fonto);
 FcPattern *pattern_small = FcNameParse((const FcChar8 *)fonto_small);
// build pattern 4 xft
 FcPattern *match;
 FcResult result;
 FcConfigSubstitute(FcConfigGetCurrent(), pattern, FcMatchPattern);
 FcDefaultSubstitute(pattern);
 match = FcFontMatch(FcConfigGetCurrent(), pattern, &result);
 FcPattern *match_small;
 FcResult result_small;
 FcConfigSubstitute(FcConfigGetCurrent(), pattern_small, FcMatchPattern);
 FcDefaultSubstitute(pattern_small);
 match_small = FcFontMatch(FcConfigGetCurrent(), pattern_small, &result);
 // open font after checks
 if(!match) {
  printf(":( font matching failed");
  return 1;
 }
 XftFont *font_small = XftFontOpenPattern(display, match_small);
 XftFont *font = XftFontOpenPattern(display, match);
 if(!font) {
  printf(":( failed to load font");
  return 1;
 }
 // set colors
 XftColor color; // text color
 XColor bgcolor; // background color (x because no fancy font shenannigans)
 // RGB(54, 57, 63) (bg color)
 bgcolor.red = (unsigned short)(54 * 257);
 bgcolor.green = (unsigned short)(57 * 257);
 bgcolor.blue = (unsigned short)(63 * 257);
 bgcolor.flags = DoRed | DoGreen | DoBlue;
 // set values for text color
 XRenderColor xr = { 65535, 65535, 65535, 65535 };
 // alloc colors
 XftColorAllocValue(display, DefaultVisual(display, s), DefaultColormap(display, s), &xr, &color);
 XAllocColor(display, DefaultColormap(display, s), &bgcolor);
 if(!font) { printf("unable to load font"); return 1; } // error handle
 XGCValues values; // x graphics context values
 values.foreground = BlackPixel(display, s);
 GC gc = XCreateGC(display, window, GCForeground, &values);
 char dateh[100]; // var to hold current date
 char timeh[100]; // var to hold current time 
 Screen *screen2 = XScreenOfDisplay(display, s); // get screen from display
 int screen_width = XDisplayWidth(display, s); // get width of screen
 int screen_height = XDisplayHeight(display, s); // get height of screen
 XGlyphInfo ext; // var to hold text info
 XResizeWindow(display, window, screen_width, screen_height); // make window size of screen
 // main loop
 while (1) {
 // while an event is happening
  while(XPending(display)) {
   XNextEvent(display, &e);
   if(e.type == KeyPress) {
    KeySym key = XLookupKeysym(&e.xkey, 0);
     if(key == XK_space) {
      set_fullscreen(display, window);
      printf("fullscreen toggle\n");
     } else if(key == XK_Up) {
       global_scale++;
       // clear previoulsy loaded fonts & free memory
       mem_cleanup(display, &font, &font_small, &pattern, &pattern_small, &match, &match_small, &fonto, &fonto_small);
       // init style string for base draw
       fonto = get_sstring(global_scale, fontsn, sbase);
       fonto_small = get_sstring(global_scale, fontsn, sbase_small);
       // load custom font file
       pattern = FcNameParse((const FcChar8 *)fonto);
       pattern_small = FcNameParse((const FcChar8 *)fonto_small);
       // build pattern 4 xft
       FcConfigSubstitute(FcConfigGetCurrent(), pattern, FcMatchPattern);
       FcDefaultSubstitute(pattern);
       match = FcFontMatch(FcConfigGetCurrent(), pattern, &result);
       FcConfigSubstitute(FcConfigGetCurrent(), pattern_small, FcMatchPattern);
       FcDefaultSubstitute(pattern_small);
       match_small = FcFontMatch(FcConfigGetCurrent(), pattern_small, &result);
       // open new fonts
       font = XftFontOpenPattern(display, match);
       font_small = XftFontOpenPattern(display, match_small);
        printf("global scale set to, %d\n", global_scale);
     } else if(key == XK_Down) {
       if(global_scale == 1) {
        global_scale = 1;
       } else {
        --global_scale;
       // clear previoulsy loaded fonts & free memory
       mem_cleanup(display, &font, &font_small, &pattern, &pattern_small, &match, &match_small, &fonto, &fonto_small);
       // init style string for base draw
       fonto = get_sstring(global_scale, fontsn, sbase);
       fonto_small = get_sstring(global_scale, fontsn, sbase_small);
       // load custom font file
       pattern = FcNameParse((const FcChar8 *)fonto);
       pattern_small = FcNameParse((const FcChar8 *)fonto_small);
       // build pattern 4 xft
       FcConfigSubstitute(FcConfigGetCurrent(), pattern, FcMatchPattern);
       FcDefaultSubstitute(pattern);
       match = FcFontMatch(FcConfigGetCurrent(), pattern, &result);
       FcConfigSubstitute(FcConfigGetCurrent(), pattern_small, FcMatchPattern);
       FcDefaultSubstitute(pattern_small);
       match_small = FcFontMatch(FcConfigGetCurrent(), pattern_small, &result);
       // open new fonts
       font = XftFontOpenPattern(display, match);
       font_small = XftFontOpenPattern(display, match_small);
        printf("global scale set to, %d\n", global_scale);
       }
     }
   }
  }
 // main drawing logic(?)
 XSetWindowBackground(display, window, bgcolor.pixel);

 time_t t = time(NULL);
 struct tm *ptr = localtime(&t);
 strftime(dateh, sizeof(dateh), "%a;%b;%d", ptr);
 strftime(timeh, sizeof(timeh), "%I:%M:%S", ptr);

 XftTextExtentsUtf8(display, font, (FcChar8 *)timeh, strlen(timeh), &ext);
 int text_width = ext.xOff;
 int text_height = font->ascent + font->descent;
 int x_center = (screen_width - text_width) / 2;
 int y_center = (screen_height + text_height) / 2 - font->descent;

 Pixmap buffer = XCreatePixmap(display, window, screen_width, screen_height, DefaultDepth(display, s));
 XftDraw *buffer_draw = XftDrawCreate(display, buffer, DefaultVisual(display, s), DefaultColormap(display, s));

 XSetForeground(display, gc, bgcolor.pixel);
 XFillRectangle(display, buffer, gc, 0, 0, screen_width, screen_height);
 XftDrawStringUtf8(buffer_draw, &color, font, x_center, y_center, (FcChar8 *)timeh, strlen(timeh));
 XftDrawStringUtf8(buffer_draw, &color, font_small, x_center, y_center - 100 * global_scale, (FcChar8 *)dateh, strlen(dateh));

 XCopyArea(display, buffer, window, gc, 0, 0, screen_width, screen_height, 0, 0);

 XftDrawDestroy(buffer_draw);
 XFreePixmap(display, buffer);

 sleep(1);
 }

 XftDrawDestroy(draw);
 mem_cleanup(display, &font, &font_small, &pattern, &pattern_small, &match, &match_small, &fonto, &fonto_small);
 XCloseDisplay(display);
 return 0;
}

void set_fullscreen(Display *dpy, Window win) {
    Atom wm_state = XInternAtom(dpy, "_NET_WM_STATE", False);
    Atom fullscreen = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);

    XEvent xev = {0};
    xev.type = ClientMessage;
    xev.xclient.window = win;
    xev.xclient.message_type = wm_state;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = 2; // 1 to add, 0 to remove, 2 to toggle
    xev.xclient.data.l[1] = fullscreen;
    xev.xclient.data.l[2] = 0; // no second property
    xev.xclient.data.l[3] = 1; // normal source indication
    xev.xclient.data.l[4] = 0;

    XSendEvent(dpy,
        DefaultRootWindow(dpy),
        False,
        SubstructureNotifyMask | SubstructureRedirectMask,
        &xev);
}

char* get_sstring(int mul, const char* instr, int base) {
 const char* sizeb = ":size=";
 int r = base * mul;
 size_t needed = strlen(instr) + strlen(sizeb) + 20;
 char* result = malloc(needed);
 if(!result) return NULL;
 snprintf(result, needed, "%s%s%d", instr, sizeb, r);
 return result;
}

void mem_cleanup(Display *display, XftFont **font, XftFont **font_small, FcPattern **pattern, FcPattern **pattern_small, FcPattern **match, FcPattern **match_small, char **fonto, char **fonto_small) {
 if (*font) {
        XftFontClose(display, *font);
        *font = NULL;
    }
    if (*font_small) {
        XftFontClose(display, *font_small);
        *font_small = NULL;
    }
    if (*pattern) {
        FcPatternDestroy(*pattern);
        *pattern = NULL;
    }
    if (*pattern_small) {
        FcPatternDestroy(*pattern_small);
        *pattern_small = NULL;
    }
    if (*match) {
        FcPatternDestroy(*match);
        *match = NULL;
    }
    if (*match_small) {
        FcPatternDestroy(*match_small);
        *match_small = NULL;
    }
    if (*fonto) {
        free(*fonto);
        *fonto = NULL;
    }
    if (*fonto_small) {
        free(*fonto_small);
        *fonto_small = NULL;
    }
}
