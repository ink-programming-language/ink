// Translated from solution.cpp.

var N: dynamic = (cpp_cast(2e5) + 5);

var INF: dynamic = cpp_cast(1e9);

var mod: dynamic = (cpp_cast(1e9) + 7);

var LLINF: dynamic = cpp_cast(1e18);

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(26, 26);

var was: dynamic = cpp_array(26);

func dfs(x: dynamic) -> dynamic
{
  write(char((97 + x)));
  {
    var to: dynamic = 0;
    while ((to < 26))
    {
      if (a[x][to])
      {
        dfs(to);
      }
      to += 1;
    }
  }
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var t: dynamic = cpp_uninitialized();
      read(t);
      {
        var j: dynamic = 0;
        while ((j < t.size()))
        {
          was[(t[j] - cpp_char("a"))] = true;
          j += 1;
        }
      }
      {
        var j: dynamic = 1;
        while ((j < t.size()))
        {
          a[(t[(j - 1)] - cpp_char("a"))][(t[j] - cpp_char("a"))] = true;
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < 26))
    {
      if ((!was[i]))
      {
        i += 1;
        continue;
      }
      var ok: dynamic = true;
      {
        var j: dynamic = 0;
        while ((j < 26))
        {
          if ((a[j][i] == true))
          {
            ok = false;
          }
          j += 1;
        }
      }
      if (ok)
      {
        dfs(i);
      }
      i += 1;
    }
  }
  return 0;
}
