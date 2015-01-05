Word Clockie (aka ClockieTwo)
=============================

Word clockie is a (yet another) word clock made as a first aniversary gift to my
wonderful wife, Ann-Marie. It is a spiritual successor to
[Clockie](http://jhnet.co.uk/projects/clockie), a count-down clock I made when
we were living far appart when we had just started going out. Instead of
counting down to our next meeting, Word Clockie counts up the time since we got
married.

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

The clock has a desktop form-factor and lives in a completely 3D printed
enclosure (by far the largest and most complex object I've designed and
printed). The character matrix is able to tell the time in one-minute-increments
and the duration of our marriage to the day for upto 190 years. It is powered by
a 12 V wall-wort but unlike Clockie it contains a battery-backed realtime clock
to keep time. This means Word Clockie keeps fairly accurate time and also
survives being unplugged. Also inside is a pair of tilt-switches which are used
to detect when the device is gently shaken to trigger a friendly message or
animation.

The matrix of letters is the result of many hours spent attempting to
mechanically generate efficient matrices for a given set of desired messages.
Multiple steps in this process turned out to be NP-hard (and worse) and after
achieveing uninspiring results with various search-based heuristics (such as
[genetic
algorithms](http://miniaturegiantspacehamster.blogspot.com/2011/03/building-word-clock-part-1-genetic.html)),
I started working on simple greedy algorithms to generate the word matrices.
These proved inadequate to reach very high-levels of compactness and a great
deal of hand-tweaking was used to produce the word matrix in clockie two.
Examples of strings which can be produced using this matrix include:

* "It is twelve minutes past seven PM"
* "For Cube" (Cube is Ann-Marie's nickname...)
* "Cubethan for one decade" (...than is my nickname)
* "two years"
* "three months"
* "four weeks"
* "and five days"
