// Translated from solution.cpp.

var N = (cpp_cast(2e5) + 5);

var INF = cpp_cast(1e9);

var mod = (cpp_cast(1e9) + 7);

var LLINF = cpp_cast(1e18);

var n: dynamic;

var a = cpp_array(26, 26);

var was = cpp_array(26);

func dfs(x: dynamic)
{
  write(char((97 + x)));
  {
    var to = 0;
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

func main()
{
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      var t: dynamic;
      read(t);
      {
        var j = 0;
        while ((j < t.size()))
        {
          was[(t[j] - cpp_char("a"))] = true;
          j += 1;
        }
      }
      {
        var j = 1;
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
    var i = 0;
    while ((i < 26))
    {
      if ((!was[i]))
      {
        i += 1;
        continue;
      }
      var ok = true;
      {
        var j = 0;
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
