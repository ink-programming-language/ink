// Translated from solution.cpp.

func main()
{
  var vp: dynamic;
  var vd: dynamic;
  var t: dynamic;
  var f: dynamic;
  var c: dynamic;
  scanf("%d %d %d %d %d", (&vp), (&vd), (&t), (&f), (&c));
  var kol = 0;
  if ((vp < vd))
  {
    var t0 = c;
    t0 /= vp;
    var T = (vp * t);
    T /= (vd - vp);
    var x = t;
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
