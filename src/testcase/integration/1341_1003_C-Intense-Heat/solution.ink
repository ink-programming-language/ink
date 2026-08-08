// Translated from solution.cpp.

func chmax(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    a = b;
    return 1;
  }
  return 0;
}

func chmin(a: dynamic, b: dynamic)
{
  if ((a > b))
  {
    a = b;
    return 1;
  }
  return 0;
}

var INF = 1e18;

func prt(p: dynamic)
{
  write("(", p.first, ", ", p.second, ")\n");
}

func prt(p: dynamic)
{
  write("(", get(p), ", ", get(p), ", ", get(p), ")\n");
}

func prt(p: dynamic)
{
  if (p)
  {
    write("True", cpp_char("\n"));
  } else
  {
    write("False", cpp_char("\n"));
  }
}

func prt(v: dynamic)
{
  write(cpp_char("{"));
  {
    var i = 0;
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

func prt(v: dynamic)
{
  write(cpp_char("{"));
  {
    var i = 0;
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

func prt(v: dynamic)
{
  write(cpp_char("{"));
  var c = 0;
  for (var p in v)
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

func prt(v: dynamic)
{
  write(cpp_char("{"));
  var c = 0;
  for (var p in v)
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

func prt(v: dynamic)
{
  write(cpp_char("{"));
  {
    var i = v.begin();
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

func prt(v: dynamic)
{
  write(cpp_char("{"));
  {
    var i = v.begin();
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

func main()
{
  var n: dynamic;
  var k: dynamic;
  read(n, k);
  {
    var i = 0;
    while ((i < (n)))
    {
      read(a[i]);
      i += 1;
    }
  }
  var r = cpp_construct((n + 1), 0);
  {
    var i = 0;
    while ((i < (n)))
    {
      r[(i + 1)] = (r[i] + a[i]);
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = 0;
    while ((i < (n)))
    {
      {
        var j = (i + k);
        while ((j <= n))
        {
          var now = (((r[j] - r[i])) / double((j - i)));
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
