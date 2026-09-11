// Translated from solution.cpp.

var ans: dynamic = 0;

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

var d: dynamic = cpp_uninitialized();

var e: dynamic = cpp_uninitialized();

var f: dynamic = cpp_uninitialized();

var g: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var z: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  scanf("%d%d%d%d%d%d%d", (&a), (&b), (&c), (&d), (&e), (&f), (&g));
  ans = b;
  x = a;
  y = d;
  z = e;
  var t: dynamic = ((((x & 1)) + ((y & 1))) + ((z & 1)));
  if ((y & 1))
  {
    swap(x, y);
  }
  if ((z & 1))
  {
    swap(z, y);
  }
  if ((t >= 2))
  {
    if ((z > 0))
    {
      ans += 3;
      x -= 1;
      y -= 1;
      z -= 1;
    }
  }
  ans += ((x / 2) * 2);
  ans += ((y / 2) * 2);
  ans += ((z / 2) * 2);
  printf("%lld", ans);
  return 0;
}
