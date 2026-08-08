// Translated from solution.cpp.

var inf = 1e9;

func main()
{
  while (1)
  {
    var h: dynamic;
    var w: dynamic;
    read(h, w);
    if (((h == 0) && (w == 0)))
    {
      break;
    }
    var e = cpp_construct((h + 2), vector((w + 2), 0));
    {
      var i = 1;
      while ((i <= h))
      {
        {
          var j = 1;
          while ((j <= w))
          {
            read(e[i][j]);
            j += 1;
          }
        }
        i += 1;
      }
    }
    var res = 0;
    {
      var l = 1;
      while ((l <= h))
      {
        {
          var u = (l + 2);
          while ((u <= h))
          {
            {
              var r = 1;
              while ((r <= w))
              {
                {
                  var le = (r + 2);
                  while ((le <= w))
                  {
                    var side = inf;
                    var inner = 0;
                    {
                      var i = (l + 1);
                      while ((i < u))
                      {
                        {
                          var j = (r + 1);
                          while ((j < le))
                          {
                            inner = max(inner, e[i][j]);
                            j += 1;
                          }
                        }
                        i += 1;
                      }
                    }
                    {
                      var i = l;
                      while ((i <= u))
                      {
                        side = min(side, min(e[i][r], e[i][le]));
                        i += 1;
                      }
                    }
                    {
                      var i = r;
                      while ((i <= le))
                      {
                        side = min(side, min(e[l][i], e[u][i]));
                        i += 1;
                      }
                    }
                    if ((inner < side))
                    {
                      var cnt = 0;
                      {
                        var i = (l + 1);
                        while ((i < u))
                        {
                          {
                            var j = (r + 1);
                            while ((j < le))
                            {
                              cnt += (side - e[i][j]);
                              j += 1;
                            }
                          }
                          i += 1;
                        }
                      }
                      res = max(res, cnt);
                    }
                    le += 1;
                  }
                }
                r += 1;
              }
            }
            u += 1;
          }
        }
        l += 1;
      }
    }
    write(res, "\n");
  }
  return 0;
}
