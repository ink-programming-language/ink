// Translated from solution.cpp.

var ans = 0;

var a: dynamic;

var b: dynamic;

var c: dynamic;

var d: dynamic;

var e: dynamic;

var f: dynamic;

var g: dynamic;

var x: dynamic;

var y: dynamic;

var z: dynamic;

func main()
{
  scanf("%d%d%d%d%d%d%d", (&a), (&b), (&c), (&d), (&e), (&f), (&g));
  ans = b;
  x = a;
  y = d;
  z = e;
  var t = ((((x & 1)) + ((y & 1))) + ((z & 1)));
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
