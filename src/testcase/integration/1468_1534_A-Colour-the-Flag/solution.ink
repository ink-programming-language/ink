// Translated from solution.cpp.

var ll = dynamic;

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    var m: dynamic;
    read(n, m);
    var count = 0;
    {
      var i = 0;
      while ((i < n))
      {
        {
          var j = 0;
          while ((j < m))
          {
            read(f[i][j]);
            if ((f[i][j] == cpp_char(".")))
            {
              count += 1;
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    var k = 0;
    if ((count == (n * m)))
    {
      {
        var i = 0;
        while ((i < n))
        {
          {
            var j = 0;
            while ((j < m))
            {
              if (((i == 0) && (j == 0)))
              {
                f[i][j] = cpp_char("R");
              } else if (((i - 1) >= 0))
              {
                if ((f[(i - 1)][j] == cpp_char("R")))
                {
                  f[i][j] = cpp_char("W");
                } else
                {
                  f[i][j] = cpp_char("R");
                }
              } else if (((j - 1) >= 0))
              {
                if ((f[i][(j - 1)] == cpp_char("R")))
                {
                  f[i][j] = cpp_char("W");
                } else
                {
                  f[i][j] = cpp_char("R");
                }
              }
              j += 1;
            }
          }
          i += 1;
        }
      }
    } else
    {
      {
        var z = 0;
        while ((z < 100))
        {
          {
            var i = 0;
            while ((i < n))
            {
              {
                var j = 0;
                while ((j < m))
                {
                  if (((i - 1) >= 0))
                  {
                    if ((f[(i - 1)][j] == cpp_char("R")))
                    {
                      f[i][j] = cpp_char("W");
                    } else if ((f[(i - 1)][j] == cpp_char("W")))
                    {
                      f[i][j] = cpp_char("R");
                    }
                  }
                  if (((j - 1) >= 0))
                  {
                    if ((f[i][(j - 1)] == cpp_char("R")))
                    {
                      if ((f[i][j] == cpp_char("R")))
                      {
                        k = 1;
                      }
                      f[i][j] = cpp_char("W");
                    } else if ((f[i][(j - 1)] == cpp_char("W")))
                    {
                      if ((f[i][j] == cpp_char("W")))
                      {
                        k = 1;
                      }
                      f[i][j] = cpp_char("R");
                    }
                  }
                  if (((i + 1) < n))
                  {
                    if ((f[(i + 1)][j] == cpp_char("R")))
                    {
                      if ((f[i][j] == cpp_char("R")))
                      {
                        k = 1;
                      }
                      f[i][j] = cpp_char("W");
                    } else if ((f[(i + 1)][j] == cpp_char("W")))
                    {
                      if ((f[i][j] == cpp_char("W")))
                      {
                        k = 1;
                      }
                      f[i][j] = cpp_char("R");
                    }
                  }
                  if (((j + 1) < m))
                  {
                    if ((f[i][(j + 1)] == cpp_char("R")))
                    {
                      if ((f[i][j] == cpp_char("R")))
                      {
                        k = 1;
                      }
                      f[i][j] = cpp_char("W");
                    } else if ((f[i][(j + 1)] == cpp_char("W")))
                    {
                      if ((f[i][j] == cpp_char("W")))
                      {
                        k = 1;
                      }
                      f[i][j] = cpp_char("R");
                    }
                  }
                  j += 1;
                }
              }
              i += 1;
            }
          }
          z += 1;
        }
      }
    }
    if ((k == 1))
    {
      write("NO", cpp_char("\n"));
    } else
    {
      write("YES", cpp_char("\n"));
      {
        var i = 0;
        while ((i < n))
        {
          {
            var j = 0;
            while ((j < m))
            {
              write(f[i][j]);
              j += 1;
            }
          }
          write(cpp_char("\n"));
          i += 1;
        }
      }
    }
  }
  return 0;
}
