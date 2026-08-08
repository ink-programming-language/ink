// Translated from solution.cpp.

func main()
{
  var c: dynamic;
  var v0: dynamic;
  var v1: dynamic;
  var a: dynamic;
  var l: dynamic;
  read(c, v0, v1, a, l);
  var ans = 1;
  var t = 0;
  var s = v0;
  t += s;
  while ((t < c))
  {
    t -= l;
    if (((s + a) > v1))
    {
      s = v1;
    } else
    {
      s += a;
    }
    t += s;
    ans += 1;
  }
  write(ans, "\n");
}
