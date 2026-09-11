// Translated from solution.cpp.

var inf: dynamic = 1e9;

func main() -> dynamic
{
  while (1)
  {
    var h: dynamic = cpp_uninitialized();
    var w: dynamic = cpp_uninitialized();
    read(h, w);
    if (((h == 0) && (w == 0)))
    {
      break;
    }
    var e: dynamic = cpp_construct((h + 2), vector((w + 2), 0));
    {
      var i: dynamic = 1;
      while ((i <= h))
      {
        {
          var j: dynamic = 1;
          while ((j <= w))
          {
            read(e[i][j]);
            j += 1;
          }
        }
        i += 1;
      }
    }
    var res: dynamic = 0;
    {
      var l: dynamic = 1;
      while ((l <= h))
      {
        {
          var u: dynamic = (l + 2);
          while ((u <= h))
          {
            {
              var r: dynamic = 1;
              while ((r <= w))
              {
                {
                  var le: dynamic = (r + 2);
                  while ((le <= w))
                  {
                    var side: dynamic = inf;
                    var inner: dynamic = 0;
                    {
                      var i: dynamic = (l + 1);
                      while ((i < u))
                      {
                        {
                          var j: dynamic = (r + 1);
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
                      var i: dynamic = l;
                      while ((i <= u))
                      {
                        side = min(side, min(e[i][r], e[i][le]));
                        i += 1;
                      }
                    }
                    {
                      var i: dynamic = r;
                      while ((i <= le))
                      {
                        side = min(side, min(e[l][i], e[u][i]));
                        i += 1;
                      }
                    }
                    if ((inner < side))
                    {
                      var cnt: dynamic = 0;
                      {
                        var i: dynamic = (l + 1);
                        while ((i < u))
                        {
                          {
                            var j: dynamic = (r + 1);
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
