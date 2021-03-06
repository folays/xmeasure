/* build command: "gcc -o xmeasure xmeasure.c -L/usr/X11R6/lib -lX11" */
/*
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

#include <X11/Xlib.h>
#include <X11/cursorfont.h>

#include <stdio.h>

int
grab_rectangle (Display * display, Window root, XRectangle * xrect)
{
  Cursor cursor_start, cursor_finis;
  unsigned int x, y, rootx, rooty;
  int rx, ry, rw = 0, rh = 0;
  int drawn = False;
  XEvent event;
  GC gc;

  /* get some cursors for rectangle formation */
  cursor_start = XCreateFontCursor (display, XC_ul_angle);
  cursor_finis = XCreateFontCursor (display, XC_lr_angle);

  /* grab the pointer */
  if (GrabSuccess != XGrabPointer (display, root, False, ButtonPressMask,
				   GrabModeAsync, GrabModeAsync, root,
				   cursor_start, CurrentTime))
    return 1;

  /* create a graphics context to draw with */
  gc = XCreateGC (display, root, 0, NULL);
  if (!gc)
    return 2;
  XSetSubwindowMode (display, gc, IncludeInferiors);
  XSetForeground (display, gc, 255);
  XSetFunction (display, gc, GXxor);

  /* get a button-press and pull out the root location */
  XMaskEvent (display, ButtonPressMask, &event);
  rootx = rx = event.xbutton.x_root;
  rooty = ry = event.xbutton.y_root;

  /* get pointer motion events */
  XChangeActivePointerGrab (display, ButtonMotionMask | ButtonReleaseMask,
			    cursor_finis, CurrentTime);

  /* MAKE_RECT converts the original root coordinates and the event root
   * coordinates into a rectangle in xrect */
#define MAKE_RECT(etype)			\
  x = event.etype.x_root;			\
  y = event.etype.y_root;			\
  rw = x - rootx;				\
  if (rw  < 0) rw  = -rw;			\
  rh = y - rooty;				\
  if (rh  < 0) rh  = -rh;			\
  rx = x < rootx ? x : rootx;			\
  ry = y < rooty ? y : rooty

  /* loop to let the user drag a rectangle */
  while (1)
    {
      XNextEvent (display, &event);
      switch (event.type)
	{
	case ButtonRelease:
	  if (drawn)
	    {
	      XDrawRectangle (display, root, gc, rx, ry, rw, rh);
	      drawn = 0;
	    }
	  XFlush (display);
	  /* record the final location */
	  MAKE_RECT (xbutton);
	  /* release resources */
	  XFreeGC (display, gc);
	  XFreeCursor (display, cursor_start);
	  XFreeCursor (display, cursor_finis);
	  xrect->x = rx;
	  xrect->y = ry;
	  xrect->width = rw;
	  xrect->height = rh;
	  XUngrabPointer (display, CurrentTime);
	  XSync (display, 0);
	  return 1;
	case MotionNotify:
	  if (drawn)
	    {
	      XDrawRectangle (display, root, gc, rx, ry, rw, rh);
	      drawn = 0;
	    }
	  while (XCheckTypedEvent (display, MotionNotify, &event))
	    {
	    }
	  MAKE_RECT (xmotion);
	  XDrawRectangle (display, root, gc, rx, ry, rw, rh);
	  drawn = 1;
	  break;
	}
    }

  return 0;
}

int
main ()
{
  Display *dpy;
  Window root;
  XRectangle xrect;

  dpy = XOpenDisplay (NULL);

  int scr = DefaultScreen(dpy);
  int dpi_x = (double)DisplayWidth(dpy, scr) * (double)25.4 / (double)DisplayWidthMM(dpy, scr) + 0.5;
  int dpi_y = (double)DisplayHeight(dpy, scr) * (double)25.4 / (double)DisplayHeightMM(dpy, scr) + 0.5;

  printf ("DPI %d %d\n", dpi_x, dpi_y);

  root = DefaultRootWindow (dpy);

  float total_square_mm = 0;
  while (1)
    {
      grab_rectangle (dpy, root, &xrect);

      printf ("---\n");
      printf ("X: %d Y: %d Width: %d Height: %d\n", xrect.x, xrect.y, xrect.width, xrect.height);

      float mm_x = (double)xrect.width * (double)DisplayWidthMM(dpy, scr) / (double)DisplayWidth(dpy, scr);
      float mm_y = (double)xrect.height * (double)DisplayHeightMM(dpy, scr) / (double)DisplayHeight(dpy, scr);

      printf ("cm : \e[32m%.2f\e[0m \e[32m%.2f\e[0m square cm : \e[33m%.2f\e[0m\n", mm_x / 10, mm_y / 10, mm_x * mm_y / 100);

      total_square_mm += mm_x * mm_y;
      printf ("total square cm : \e[35m%.2f\e[0m\n", total_square_mm / 100);

      fflush(stdout);
    }

  XCloseDisplay (dpy);

  return 0;
}
