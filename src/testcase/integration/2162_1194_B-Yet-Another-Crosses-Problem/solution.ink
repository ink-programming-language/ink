// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var q: dynamic = cpp_uninitialized();
  read(q);
  while (cpp_update(q, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    var m: dynamic = cpp_uninitialized();
    read(n, m);
    var str: dynamic = cpp_array((n + 5));
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        read(str[i]);
        i += 1;
      }
    }
    var hor: dynamic = INT_MAX;
    var ver: dynamic = INT_MAX;
    var row: dynamic = cpp_uninitialized();
    var col: dynamic = cpp_uninitialized();
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        var cnt: dynamic = 0;
        {
          var j: dynamic = 0;
          while ((j < m))
          {
            if ((str[i][j] == cpp_char(".")))
            {
              cnt += 1;
            }
            j += 1;
          }
        }
        hor = min(hor, cnt);
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < m))
      {
        var cnt: dynamic = 0;
        {
          var j: dynamic = 0;
          while ((j < n))
          {
            if ((str[j][i] == cpp_char(".")))
            {
              cnt += 1;
            }
            j += 1;
          }
        }
        ver = min(ver, cnt);
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        var cnt: dynamic = 0;
        {
          var j: dynamic = 0;
          while ((j < m))
          {
            if ((str[i][j] == cpp_char(".")))
            {
              cnt += 1;
            }
            j += 1;
          }
        }
        if ((cnt == hor))
        {
          row.push_back(i);
        }
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < m))
      {
        var cnt: dynamic = 0;
        {
          var j: dynamic = 0;
          while ((j < n))
          {
            if ((str[j][i] == cpp_char(".")))
            {
              cnt += 1;
            }
            j += 1;
          }
        }
        if ((ver == cnt))
        {
          col.push_back(i);
        }
        i += 1;
      }
    }
    var ans: dynamic = (hor + ver);
    {
      var i: dynamic = 0;
      while ((i < row.size()))
      {
        var j: dynamic = cpp_uninitialized();
        {
          j = 0;
          while ((j < col.size()))
          {
            if ((str[row[i]][col[j]] == cpp_char(".")))
            {
              ans -= 1;
              break;
            }
            j += 1;
          }
        }
        if ((j != col.size()))
        {
          break;
        }
        i += 1;
      }
    }
    write(ans);
    if (q)
    {
      write(cpp_char("\n"));
    }
  }
  return 0;
}
