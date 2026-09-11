// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  var e: dynamic = cpp_uninitialized();
  var f: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var z: dynamic = cpp_uninitialized();
  read(n, k);
  read(x);
  a = x.length();
  var v: dynamic = cpp_construct(30, 0);
  var u: dynamic = cpp_construct(30, 0);
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
  var p: dynamic = cpp_construct((a + 5), 0);
  var q: dynamic = cpp_construct((a + 5), 0);
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
