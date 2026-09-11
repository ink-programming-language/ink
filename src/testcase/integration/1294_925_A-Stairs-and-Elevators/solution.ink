// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var ladder: dynamic = cpp_uninitialized();

var lift: dynamic = cpp_uninitialized();

var v: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var lads: dynamic = cpp_uninitialized();

var lifts: dynamic = cpp_uninitialized();

func sa(xa: dynamic, ya: dynamic, xb: dynamic, yb: dynamic, lads: dynamic, v: dynamic) -> dynamic
{
  var xrun: dynamic = abs((xa - xb));
  var ydist: dynamic = abs((ya - yb));
  if ((ya == yb))
  {
    return xrun;
  }
  var anw: dynamic = 1e18;
  var arp: dynamic = (upper_bound(lads.begin(), lads.end(), xa) - lads.begin());
  var alp: dynamic = (arp - 1);
  var brp: dynamic = (upper_bound(lads.begin(), lads.end(), xb) - lads.begin());
  var blp: dynamic = (brp - 1);
  var alll: dynamic = [alp, arp, blp, brp];
  for (var c: dynamic in alll)
  {
    if (((c >= 0) && (c < lads.size())))
    {
      anw = min(anw, ((abs((lads[c] - xa)) + abs((lads[c] - xb))) + ((((ydist + v) - 1)) / v)));
    }
  }
  return anw;
}

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cout.precision(10);
  read(n, m, ladder, lift, v);
  lads.resize(ladder);
  lifts.resize(lift);
  {
    var i: dynamic = 0;
    while ((i < ladder))
    {
      read(lads[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < lift))
    {
      read(lifts[i]);
      i += 1;
    }
  }
  read(q);
  {
    var i: dynamic = 0;
    while ((i < q))
    {
      var x1: dynamic = cpp_uninitialized();
      var y1: dynamic = cpp_uninitialized();
      var x2: dynamic = cpp_uninitialized();
      var y2: dynamic = cpp_uninitialized();
      read(y1, x1, y2, x2);
      write(min(sa(x1, y1, x2, y2, lads, 1), sa(x1, y1, x2, y2, lifts, v)), "\n");
      i += 1;
    }
  }
}
