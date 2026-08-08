// Translated from solution.cpp.

var b = cpp_array(102);

var g = cpp_array(102);

func main()
{
  var n: dynamic;
  var m: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(b[i]);
      i += 1;
    }
  }
  read(m);
  {
    var i = 0;
    while ((i < m))
    {
      read(g[i]);
      i += 1;
    }
  }
  sort(b, (b + n));
  sort(g, (g + m));
  var itb: dynamic;
  var itg: dynamic;
  itb = cpp_assign(itg, "=", 0);
  var cnt = 0;
  while (((itb < n) && (itg < m)))
  {
    if ((abs((b[itb] - g[itg])) <= 1))
    {
      cnt += 1;
      itb += 1;
      itg += 1;
      continue;
    }
    if ((b[itb] < g[itg]))
    {
      itb += 1;
    } else
    {
      itg += 1;
    }
  }
  write(cnt);
  return 0;
}
