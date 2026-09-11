// Translated from solution.cpp.

func main() -> dynamic
{
  var vp: dynamic = cpp_uninitialized();
  var vd: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var f: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  scanf("%d %d %d %d %d", (&vp), (&vd), (&t), (&f), (&c));
  var kol: dynamic = 0;
  if ((vp < vd))
  {
    var t0: dynamic = c;
    t0 /= vp;
    var T: dynamic = (vp * t);
    T /= (vd - vp);
    var x: dynamic = t;
    while (((x + T) < t0))
    {
      kol += 1;
      x = ((x + (2 * T)) + f);
      T = (vp * x);
      T /= (vd - vp);
    }
  }
  printf("%d", kol);
  return 0;
}
