// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  if ((n > m))
  {
    write("YES\n");
    return 0;
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      a[i] %= m;
      i += 1;
    }
  }
  var p: dynamic = cpp_construct(m, -1);
  var np: dynamic = cpp_construct(m, -1);
  {
    var i: dynamic = 0;
    while (((i < n) && (p[0] == -1)))
    {
      var v: dynamic = a[i];
      copy(p.begin(), p.end(), np.begin());
      if ((np[v] == -1))
      {
        np[v] = 1;
      }
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          if (((p[j] != -1) && (p[(((j + v)) % m)] == -1)))
          {
            np[(((j + v)) % m)] = 1;
          }
          j += 1;
        }
      }
      copy(np.begin(), np.end(), p.begin());
      i += 1;
    }
  }
  if ((p[0] == -1))
  {
    write("NO\n");
  } else
  {
    write("YES\n");
  }
  return 0;
}
