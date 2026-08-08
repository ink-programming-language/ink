// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  var d: dynamic;
  var e: dynamic;
  var f: dynamic;
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var n: dynamic;
  var x: dynamic;
  var y: dynamic;
  var z: dynamic;
  read(n, k);
  read(x);
  a = x.length();
  var v = cpp_construct(30, 0);
  var u = cpp_construct(30, 0);
  {
    i = 0;
    while ((i < a))
    {
      b = cpp_cast(((x[i] - cpp_char("A"))));
      if ((v[b] == 0))
      {
        v[b] = (i + 1);
      }
      u[b] = (i + 1);
      i += 1;
    }
  }
  var p = cpp_construct((a + 5), 0);
  var q = cpp_construct((a + 5), 0);
  {
    i = 0;
    while ((i < 26))
    {
      p[v[i]] += 1;
      q[u[i]] += 1;
      i += 1;
    }
  }
  d = 0;
  {
    i = 1;
    while ((i <= a))
    {
      d += p[i];
      if ((d > k))
      {
        write("YES");
        return 0;
      }
      d -= q[i];
      i += 1;
    }
  }
  write("NO");
  return 0;
}
