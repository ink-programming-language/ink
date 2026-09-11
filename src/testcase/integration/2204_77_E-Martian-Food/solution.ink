// Translated from solution.cpp.

func transformx(r: dynamic, x: dynamic, y: dynamic) -> dynamic
{
  return ((((r * r) * x)) / (((x * x) + (y * y))));
}

func transformy(r: dynamic, x: dynamic, y: dynamic) -> dynamic
{
  return ((((r * r) * y)) / (((x * x) + (y * y))));
}

func sqr(x: dynamic) -> dynamic
{
  return (x * x);
}

func dist(x0: dynamic, y0: dynamic, x1: dynamic, y1: dynamic) -> dynamic
{
  return sqrt((sqr((x0 - x1)) + sqr((y0 - y1))));
}

class circle
{
  var c: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
}

func get_circle(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  var x: dynamic = (1.0 / conj((b - a)));
  var y: dynamic = (1.0 / conj((c - a)));
  var t: dynamic = ((((y - x)) / (((conj(x) * y) - (x * conj(y))))) + a);
  return [t, abs((a - t))];
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.setf(ios_base.fixed);
  cout.precision(17);
  var R: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(k);
  while ((((cin >> R) >> r) >> k))
  {
    var invr: dynamic = 1;
    var xleft: dynamic = transformx(invr, (2 * R), 0);
    var xright: dynamic = transformx(invr, (2 * r), 0);
    var d: dynamic = (xright - xleft);
    var h: dynamic = (d * k);
    var p3: dynamic = cpp_construct(transformx(invr, (xleft + (d / 2)), (h + (d / 2))), transformy(invr, (xleft + (d / 2)), (h + (d / 2))));
    write(get_circle(p1, p2, p3).r, cpp_char("\n"));
  }
}
