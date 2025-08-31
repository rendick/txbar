#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int number_of_modules = 0;
char *output_of_modules[100];
int width_of_modules[100] = {0};

char *separator = " | ";

void die(char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(EXIT_FAILURE);
}

int strwid(char *str, XFontStruct *font_struct) {
  return XTextWidth(font_struct, str, strlen(str));
}

void declare_commands(Display *dpy, Window win, GC gc, XFontStruct *font_struct,
                      int pos, char *command) {
  FILE *run_command = popen(command, "r");
  if (!run_command)
    perror("popen()");

  char buffer[1024];
  while (fgets(buffer, sizeof(buffer), run_command)) {
    if (strlen(buffer) > 0)
      buffer[strlen(buffer) - 1] = '\0';
    output_of_modules[pos] = strdup(buffer);
  }
  pclose(run_command);
  number_of_modules++;
}

void run_commands(Display *dpy, Window win, GC gc, XFontStruct *font_struct) {
  int free_screen_width = DisplayWidth(dpy, DefaultScreen(dpy));
  for (int i = 0; i < number_of_modules; i++) {
    XDrawString(dpy, win, gc,
                free_screen_width - strwid(output_of_modules[i], font_struct),
                16, output_of_modules[i], strlen(output_of_modules[i]));
    free_screen_width -= strwid(output_of_modules[i], font_struct);

    if (i != number_of_modules - 1) {
      XDrawString(dpy, win, gc,
                  free_screen_width - strwid(separator, font_struct), 16,
                  separator, strlen(separator));
      free_screen_width -= strwid(separator, font_struct);
    }

    XFlush(dpy);
  }
}

int main(void) {
  Display *dpy;
  Window win, root;
  GC gc;
  Font font;
  XFontStruct *font_struct;

  if ((dpy = XOpenDisplay(NULL)) == NULL)
    die("Cannon open X11 display");

  root = DefaultRootWindow(dpy);
  win = XCreateSimpleWindow(dpy, root, 0, 0, 1, 22, 0, 0, 0x000000);

  Atom wm_window_type = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
  Atom wm_window_type_dock =
      XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
  XChangeProperty(dpy, win, wm_window_type, XA_ATOM, 32, PropModeReplace,
                  (unsigned char *)&wm_window_type_dock, 1);

  gc = XCreateGC(dpy, win, 0, NULL);

  font_struct = XLoadQueryFont(dpy, "fixed");

  font = font_struct->fid;
  XSetFont(dpy, gc, font);

  XSetForeground(dpy, gc, 0xffffff);

  XSelectInput(dpy, win, StructureNotifyMask | ExposureMask);
  XMapWindow(dpy, win);

  for (;;) {
    XFlush(dpy);

    number_of_modules = 0;
    XClearWindow(dpy, win);
    XSetForeground(dpy, gc, 0xffffff);

    declare_commands(dpy, win, gc, font_struct, number_of_modules, "date");
    declare_commands(dpy, win, gc, font_struct, number_of_modules, "whoami");
    declare_commands(
        dpy, win, gc, font_struct, number_of_modules,
        "echo \"BAT $(cat /sys/class/power_supply/BAT0/capacity)%\"");

    run_commands(dpy, win, gc, font_struct);

    for (int i = 0; i < number_of_modules; i++) {
      free(output_of_modules[i]);
      output_of_modules[i] = NULL;
    }

    usleep(1000000);
  }

  XFreeGC(dpy, gc);
  XDestroySubwindows(dpy, win);
  XCloseDisplay(dpy);

  return EXIT_SUCCESS;
}
