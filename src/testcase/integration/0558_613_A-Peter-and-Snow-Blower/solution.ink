// Translated from solution.cpp.

var nax: dynamic = (1e5 + 50);

var p: dynamic = cpp_array(nax);

func sqar(a: dynamic) -> dynamic
{
  return (a * a);
}

func main() -> dynamic
{
  var pi: dynamic = acos(-1);
  var n: dynamic = cpp_uninitialized();
  var x0: dynamic = cpp_uninitialized();
  var y0: dynamic = cpp_uninitialized();
  read(n, x0, y0);
  var br: dynamic = 0;
  var sr: dynamic = 1e18;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      read(x, y);
      p[i] = [x, y];
      br = max(br, (sqar((x - x0)) + sqar((y - y0))));
      sr = min(sr, (sqar((x - x0)) + sqar((y - y0))));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var j: dynamic = (((i + 1)) % n);
      var x1: dynamic = p[i].first;
      var y1: dynamic = p[i].second;
      var x2: dynamic = p[j].first;
      var y2: dynamic = p[j].second;
      var l: dynamic = (sqar((x1 - x2)) + sqar((y1 - y2)));
      if ((l == 0))
      {
        i += 1;
        continue;
      }
      var t: dynamic = ((((((x0 - x1)) * ((x2 - x1))) + (((y0 - y1)) * ((y2 - y1))))) / l);
      t = max(0.0, min(1.0, t));
      var x: dynamic = (x1 + (t * ((x2 - x1))));
      var y: dynamic = (y1 + (t * ((y2 - y1))));
      var dis: dynamic = (sqar((x0 - x)) + sqar((y0 - y)));
      sr = min(sr, dis);
      i += 1;
    }
  }
  write(setprecision(12));
  write(((pi * br) - (pi * sr)), "\n");
}
