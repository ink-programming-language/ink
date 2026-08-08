// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var q: dynamic;
  read(q);
  while (cpp_update(q, "--"))
  {
    var n: dynamic;
    var m: dynamic;
    read(n, m);
    var str = cpp_array((n + 5));
    {
      var i = 0;
      while ((i < n))
      {
        read(str[i]);
        i += 1;
      }
    }
    var hor = INT_MAX;
    var ver = INT_MAX;
    var row: dynamic;
    var col: dynamic;
    {
      var i = 0;
      while ((i < n))
      {
        var cnt = 0;
        {
          var j = 0;
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
      var i = 0;
      while ((i < m))
      {
        var cnt = 0;
        {
          var j = 0;
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
      var i = 0;
      while ((i < n))
      {
        var cnt = 0;
        {
          var j = 0;
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
      var i = 0;
      while ((i < m))
      {
        var cnt = 0;
        {
          var j = 0;
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
    var ans = (hor + ver);
    {
      var i = 0;
      while ((i < row.size()))
      {
        var j: dynamic;
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
