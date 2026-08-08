// Translated from solution.cpp.

var MOD = cpp_cast(((1e9 + 7)));

var grid = cpp_array(1005, 1005);

func main()
{
  var h: dynamic;
  var w: dynamic;
  read(h, w);
  var r = cpp_array(h);
  var c = cpp_array(w);
  {
    var i = 0;
    while ((i < h))
    {
      read(r[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < w))
    {
      read(c[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < h))
    {
      {
        var j = 0;
        while ((j < w))
        {
          grid[i][j] = cpp_char("n");
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < h))
    {
      if ((r[i] > w))
      {
        write(0, "\n");
        return 0;
      }
      {
        var j = 0;
        while ((j < r[i]))
        {
          if ((grid[i][j] == cpp_char("e")))
          {
            write(0, "\n");
            return 0;
          }
          grid[i][j] = cpp_char("f");
          j += 1;
        }
      }
      if ((grid[i][r[i]] == cpp_char("f")))
      {
        write(0, "\n");
        return 0;
      }
      grid[i][r[i]] = cpp_char("e");
      i += 1;
    }
  }
  {
    var j = 0;
    while ((j < w))
    {
      if ((c[j] > h))
      {
        write(0, "\n");
        return 0;
      }
      {
        var i = 0;
        while ((i < c[j]))
        {
          if ((grid[i][j] == cpp_char("e")))
          {
            write(0, "\n");
            return 0;
          }
          grid[i][j] = cpp_char("f");
          i += 1;
        }
      }
      if ((grid[c[j]][j] == cpp_char("f")))
      {
        write(0, "\n");
        return 0;
      }
      grid[c[j]][j] = cpp_char("e");
      j += 1;
    }
  }
  var ans = 1;
  {
    var i = 0;
    while ((i < h))
    {
      {
        var j = 0;
        while ((j < w))
        {
          if (((grid[i][j] == cpp_char("f")) || (grid[i][j] == cpp_char("e"))))
          {
            j += 1;
            continue;
          }
          ans *= 2;
          ans %= MOD;
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
