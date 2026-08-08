// Translated from solution.cpp.

func transformx(r: dynamic, x: dynamic, y: dynamic)
{
  return ((((r * r) * x)) / (((x * x) + (y * y))));
}

func transformy(r: dynamic, x: dynamic, y: dynamic)
{
  return ((((r * r) * y)) / (((x * x) + (y * y))));
}

func sqr(x: dynamic)
{
  return (x * x);
}

func dist(x0: dynamic, y0: dynamic, x1: dynamic, y1: dynamic)
{
  return sqrt((sqr((x0 - x1)) + sqr((y0 - y1))));
}

class circle
{
  var c: dynamic;
  var r: dynamic;
}

func get_circle(a: dynamic, b: dynamic, c: dynamic)
{
  var x = (1.0 / conj((b - a)));
  var y = (1.0 / conj((c - a)));
  var t = ((((y - x)) / (((conj(x) * y) - (x * conj(y))))) + a);
  return [t, abs((a - t))];
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.setf(ios_base.fixed);
  cout.precision(17);
  var R: dynamic;
  var r: dynamic;
  var k: dynamic;
  read(k);
  while ((((cin >> R) >> r) >> k))
  {
    var invr = 1;
    var xleft = transformx(invr, (2 * R), 0);
    var xright = transformx(invr, (2 * r), 0);
    var d = (xright - xleft);
    var h = (d * k);
    var p3 = cpp_construct(transformx(invr, (xleft + (d / 2)), (h + (d / 2))), transformy(invr, (xleft + (d / 2)), (h + (d / 2))));
    write(get_circle(p1, p2, p3).r, cpp_char("\n"));
  }
}
