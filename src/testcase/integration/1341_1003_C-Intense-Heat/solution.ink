// Translated from solution.cpp.

func chmax(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    a = b;
    return 1;
  }
  return 0;
}

func chmin(a: dynamic, b: dynamic) -> dynamic
{
  if ((a > b))
  {
    a = b;
    return 1;
  }
  return 0;
}

var INF: dynamic = 1e18;

func prt(p: dynamic) -> dynamic
{
  write("(", p.first, ", ", p.second, ")\n");
}

func prt(p: dynamic) -> dynamic
{
  write("(", get(p), ", ", get(p), ", ", get(p), ")\n");
}

func prt(p: dynamic) -> dynamic
{
  if (p)
  {
    write("True", cpp_char("\n"));
  } else
  {
    write("False", cpp_char("\n"));
  }
}

func prt(v: dynamic) -> dynamic
{
  write(cpp_char("{"));
  {
    var i: dynamic = 0;
    while ((i < v.size()))
    {
      write(v[i]);
      if ((i < (v.size() - 1)))
      {
        write(", ");
      }
      i += 1;
    }
  }
  write(cpp_char("}"), cpp_char("\n"));
}

func prt(v: dynamic) -> dynamic
{
  write(cpp_char("{"));
  {
    var i: dynamic = 0;
    while ((i < v.size()))
    {
      write(v[i]);
      if ((i < (v.size() - 1)))
      {
        write(", ");
      }
      i += 1;
    }
  }
  write(cpp_char("}"), cpp_char("\n"));
}

func prt(v: dynamic) -> dynamic
{
  write(cpp_char("{"));
  var c: dynamic = 0;
  for (var p: dynamic in v)
  {
    write(p.first, ":", p.second);
    c += 1;
    if ((c != v.size()))
    {
      write(", ");
    }
  }
  write(cpp_char("}"), cpp_char("\n"));
}

func prt(v: dynamic) -> dynamic
{
  write(cpp_char("{"));
  var c: dynamic = 0;
  for (var p: dynamic in v)
  {
    write(p.first, ":", p.second);
    c += 1;
    if ((c != v.size()))
    {
      write(", ");
    }
  }
  write(cpp_char("}"), cpp_char("\n"));
}

func prt(v: dynamic) -> dynamic
{
  write(cpp_char("{"));
  {
    var i: dynamic = v.begin();
    while ((i != v.end()))
    {
      write((*i));
      if ((i != cpp_update(v.end(), "--")))
      {
        write(", ");
      }
      i += 1;
    }
  }
  write(cpp_char("}"), cpp_char("\n"));
}

func prt(v: dynamic) -> dynamic
{
  write(cpp_char("{"));
  {
    var i: dynamic = v.begin();
    while ((i != v.end()))
    {
      write((*i));
      if ((i != cpp_update(v.end(), "--")))
      {
        write(", ");
      }
      i += 1;
    }
  }
  write(cpp_char("}"), cpp_char("\n"));
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  {
    var i: dynamic = 0;
    while ((i < (n)))
    {
      read(a[i]);
      i += 1;
    }
  }
  var r: dynamic = cpp_construct((n + 1), 0);
  {
    var i: dynamic = 0;
    while ((i < (n)))
    {
      r[(i + 1)] = (r[i] + a[i]);
      i += 1;
    }
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < (n)))
    {
      {
        var j: dynamic = (i + k);
        while ((j <= n))
        {
          var now: dynamic = (((r[j] - r[i])) / cpp_double((j - i)));
          chmax(ans, now);
          j += 1;
        }
      }
      i += 1;
    }
  }
  printf("%.16lf\n", ans);
  return 0;
}
