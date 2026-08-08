// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  if ((n > m))
  {
    write("YES\n");
    return 0;
  }
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      a[i] %= m;
      i += 1;
    }
  }
  var p = cpp_construct(m, -1);
  var np = cpp_construct(m, -1);
  {
    var i = 0;
    while (((i < n) && (p[0] == -1)))
    {
      var v = a[i];
      copy(p.begin(), p.end(), np.begin());
      if ((np[v] == -1))
      {
        np[v] = 1;
      }
      {
        var j = 0;
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
