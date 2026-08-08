// Translated from solution.cpp.

var nax = (1e5 + 50);

var p = cpp_array(nax);

func sqar(a: dynamic)
{
  return (a * a);
}

func main()
{
  var pi = acos(-1);
  var n: dynamic;
  var x0: dynamic;
  var y0: dynamic;
  read(n, x0, y0);
  var br = 0;
  var sr = 1e18;
  {
    var i = 0;
    while ((i < n))
    {
      var x: dynamic;
      var y: dynamic;
      read(x, y);
      p[i] = [x, y];
      br = max(br, (sqar((x - x0)) + sqar((y - y0))));
      sr = min(sr, (sqar((x - x0)) + sqar((y - y0))));
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      var j = (((i + 1)) % n);
      var x1 = p[i].first;
      var y1 = p[i].second;
      var x2 = p[j].first;
      var y2 = p[j].second;
      var l = (sqar((x1 - x2)) + sqar((y1 - y2)));
      if ((l == 0))
      {
        i += 1;
        continue;
      }
      var t = ((((((x0 - x1)) * ((x2 - x1))) + (((y0 - y1)) * ((y2 - y1))))) / l);
      t = max(0.0, min(1.0, t));
      var x = (x1 + (t * ((x2 - x1))));
      var y = (y1 + (t * ((y2 - y1))));
      var dis = (sqar((x0 - x)) + sqar((y0 - y)));
      sr = min(sr, dis);
      i += 1;
    }
  }
  write(setprecision(12));
  write(((pi * br) - (pi * sr)), "\n");
}
