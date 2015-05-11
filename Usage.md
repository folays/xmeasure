## Build ##

```
gcc -o xmeasure xmeasure.c -L/usr/X11R6/lib -lX11
```

## Usage ##

  * run xmeasure in terminal
  * Mouse to the corner of the object to be measured
  * Click and hold left mouse button and drag around the object
  * Release the mouse button.  Coordinates, width, and height should be printed on terminal output.

## Bash script to display information in [yad](http://code.google.com/p/yad) ##
<a href='https://picasaweb.google.com/lh/photo/rCBPwpypUD7QbRA9crLZsQ?feat=embedwebsite'><img src='https://lh5.googleusercontent.com/-mTspqI9VX5c/ToUDUxLIVII/AAAAAAAAASQ/hcugkRNyWhM/s144/xmeasure_yad.png' height='144' width='128' /></a>
```
#!/bin/bash
Y="yad --mouse --window-icon /home/tri/icons/ruler.png --title Measure"
function measure {
  i=(`xmeasure`)
  (
    echo ${i[0]} ${i[1]}
    echo ${i[2]} ${i[3]}
    echo ${i[4]} ${i[5]} pixel
    echo ${i[6]} ${i[7]} pixel
  )|$Y --button gtk-ok:0 --width 150 --height 150 --text-info
  $Y --button Measure:0 --button gtk-cancel:1
  [ $? -eq 0 ] && measure
}
measure
```