Word Clockie (aka ClockieTwo)
=============================

Word clockie is a (yet another) word clock made as a first aniversary gift to my
wonderful wife, Ann-Marie. It is a spiritual successor to
[Clockie](http://jhnet.co.uk/projects/clockie), a count-down clock I made when
we were living far appart when we had just started going out. Instead of
counting down to our next meeting, Word Clockie counts up the time since we got
married.

![Word Clockie showing the time](http://jhnet.co.uk/misc/clockieTwo/00%20Showing%20Time.jpg)

![Side view of Word Clockie](http://jhnet.co.uk/misc/clockieTwo/02%20Side.jpg)

With the soppy bit out of the way, Word Clockie consists of a matrix of letters
selectively illuminated up to spell out the current time.  Following the
[surprisingly recent](https://www.youtube.com/watch?v=9ko9CeylUTs) revelation
that that [easily sourced LED
matrices](http://www.ebay.co.uk/sch/i.html?_trksid=p2050601.m570.l1313.TR11.TRC1.A0.Xwedding+place+cards&_nkw=led+matrix&_sacat=0&_from=R40)
solve the traditionally hard problem of building large LED arrays with nicely
defused light without leaking between LEDs, our anniversary seemed like a good
excuse to build one. Word clocks have of course been [done to
death](hackaday.com/?s=word+clock) but in spite of this Word Clockie hopefully
has a handful of fun twists not shared by other offerings.

![Word Clockie's underlying display modules](http://jhnet.co.uk/misc/clockieTwo/05%20Displays.jpg)


Hardware
--------

The clock has a desktop form-factor and lives in a completely 3D printed
enclosure (by far the largest and most complex object I've designed and
printed). The character matrix is able to tell the time in one-minute-increments
and the duration of our marriage to the day for up to 190 years. It is powered
by a 12 V wall-wort but unlike Clockie it contains a battery-backed realtime
clock to keep time. This means Word Clockie keeps fairly accurate time and also
survives being unplugged. Also inside is a pair of tilt-switches which are used
to detect when the device is gently shaken to trigger a friendly message or
animation. Finally, as with Clockie there are no buttons or switches to set the
time and so an [optical system](http://www.amp.jhnet.co.uk/programmer/) is used
to set the time if required.

![Word Clockie electronics](http://jhnet.co.uk/misc/clockieTwo/12%20Control%20electronics.jpg)

![Display electronics](http://jhnet.co.uk/misc/clockieTwo/10%20Display%20test.jpg)


Letter Matrix
-------------

The matrix of letters is the result of many hours spent attempting to
mechanically generate efficient matrices for a given set of desired messages.
Multiple steps in this process turned out to be NP-hard (and worse) and after
achieveing uninspiring results with various search-based heuristics (such as
[genetic
algorithms](http://miniaturegiantspacehamster.blogspot.com/2011/03/building-word-clock-part-1-genetic.html)),
I started working on simple greedy algorithms to generate the word matrices.
These proved inadequate to reach very high-levels of compactness and a great
deal of hand-tweaking was used to produce the word matrix in Word Clockie.
Examples of strings which can be produced using this matrix include:

* "It is twelve minutes past seven PM"
* "For Cube" (Cube is Ann-Marie's nickname...)
* "Cubethan for one decade" (...than is my nickname)
* "two years"
* "three months"
* "four weeks"
* "and five days"
* "E e e e e k"
* "Ten to go"
* (A heart glyph)

The letter matrix was cut out of sticky-backed paper printed black using a
desktop plotter. Each letter was then painstakingly peeled off. The letter 'E'
strangely proved the most troublesome as it failed to cut completely in the
majority of cases requiring removal using a scalpel. 

![Display mask with letters removed](http://jhnet.co.uk/misc/clockieTwo/06%20Mask%20cut%20out.jpg)

With the letters removed, the mask was then transferred to a laminating pouch.
Letters such as 'O' and 'A' then had to have their central shape stuck down in
the gaps.

![Transferring mask to laminate](http://jhnet.co.uk/misc/clockieTwo/07%20Mask%20to%20laminate.jpg)

The pouch was then run through the laminator yielding a fairly sturdy mask with
a pleasant finish.

![Completed mask](http://jhnet.co.uk/misc/clockieTwo/09%20Mask%20attempt%20two%20of%20four.jpg)


Software
--------

The software is a relatively pedestrian affair running on an Arduino Pro MIni
with a handful of flourishes. In addition to displaying messages using the
letter mask, the display can also operate as a low-resolution bitmap display.
This is used to display either simple graphics (such as an animated face) or
scrolling text with 'real' fonts and proper keming.

![Word Clockie Smiling](http://jhnet.co.uk/misc/clockieTwo/01%20Smiling.jpg)

The software operates on a pair of frame buffers where one is considered the
"current" image and the other is used to build up the next image. When a new
image is ready to display, the buffers are "flipped" to display the new image.
This "flipping" process may be controlled by one of a number of simple (fading)
animations. These animations are implemented by either controlling the display's
duty-cycle (when fading to/from black) or rapidly switching between the two
frame buffers to implement a cross-fade (i.e. somewhat like software driven
PWM).
