// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var ladder: dynamic;

var lift: dynamic;

var v: dynamic;

var q: dynamic;

var lads: dynamic;

var lifts: dynamic;

func sa(xa: dynamic, ya: dynamic, xb: dynamic, yb: dynamic, lads: dynamic, v: dynamic)
{
  var xrun = abs((xa - xb));
  var ydist = abs((ya - yb));
  if ((ya == yb))
  {
    return xrun;
  }
  var anw = 1e18;
  var arp = (upper_bound(lads.begin(), lads.end(), xa) - lads.begin());
  var alp = (arp - 1);
  var brp = (upper_bound(lads.begin(), lads.end(), xb) - lads.begin());
  var blp = (brp - 1);
  var alll = [alp, arp, blp, brp];
  for (var c in alll)
  {
    if (((c >= 0) && (c < lads.size())))
    {
      anw = min(anw, ((abs((lads[c] - xa)) + abs((lads[c] - xb))) + ((((ydist + v) - 1)) / v)));
    }
  }
  return anw;
}

func main()
{
  ios.sync_with_stdio(0);
  cout.precision(10);
  read(n, m, ladder, lift, v);
  lads.resize(ladder);
  lifts.resize(lift);
  {
    var i = 0;
    while ((i < ladder))
    {
      read(lads[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < lift))
    {
      read(lifts[i]);
      i += 1;
    }
  }
  read(q);
  {
    var i = 0;
    while ((i < q))
    {
      var x1: dynamic;
      var y1: dynamic;
      var x2: dynamic;
      var y2: dynamic;
      read(y1, x1, y2, x2);
      write(min(sa(x1, y1, x2, y2, lads, 1), sa(x1, y1, x2, y2, lifts, v)), "\n");
      i += 1;
    }
  }
}
