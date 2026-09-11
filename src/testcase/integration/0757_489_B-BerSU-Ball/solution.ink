// Translated from solution.cpp.

var b: dynamic = cpp_array(102);

var g: dynamic = cpp_array(102);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(b[i]);
      i += 1;
    }
  }
  read(m);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      read(g[i]);
      i += 1;
    }
  }
  sort(b, (b + n));
  sort(g, (g + m));
  var itb: dynamic = cpp_uninitialized();
  var itg: dynamic = cpp_uninitialized();
  itb = cpp_assign(itg, "=", 0);
  var cnt: dynamic = 0;
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
