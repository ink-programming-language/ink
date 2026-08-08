// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var n: dynamic;
  var m: dynamic;
  var i: dynamic;
  var j: dynamic;
  var a: dynamic;
  var b: dynamic;
  var o = 0;
  var e = 0;
  var o1 = 0;
  var e1 = 0;
  var c: dynamic;
  var d: dynamic;
  read(n, m);
  {
    i = 0;
    while ((i < n))
    {
      read(a);
      if (((a % 2) == 1))
      {
        o += 1;
      } else
      {
        e += 1;
      }
      i += 1;
    }
  }
  {
    j = 0;
    while ((j < m))
    {
      read(b);
      if (((b % 2) == 1))
      {
        o1 += 1;
      } else
      {
        e1 += 1;
      }
      j += 1;
    }
  }
  c = min(o, e1);
  d = min(o1, e);
  write((c + d));
  return 0;
}
