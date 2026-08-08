// Translated from solution.cpp.

var maxn = 100005;

func sqr(a: dynamic)
{
  return (a * a);
}

func dist(a: dynamic, b: dynamic)
{
  return (sqr((a.first - b.first)) + sqr((a.second - b.second)));
}

func newval(a: dynamic, b: dynamic)
{
  var v = cpp_construct(((b.first - a.first)), ((b.second - a.second)));
  var r = v;
  r.first = v.second;
  r.second = (-v.first);
  v.first = (b.first + r.first);
  v.second = (b.second + r.second);
  return v;
}

func main()
{
  ios_base.sync_with_stdio(0);
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < cpp_cast(n)))
    {
      var a = cpp_array(4);
      var b = cpp_array(4);
      var c = cpp_array(4);
      {
        var j = 0;
        while ((j < cpp_cast(4)))
        {
          read(a[j].first, a[j].second, b[j].first, b[j].second);
          c[j] = a[j];
          j += 1;
        }
      }
      var mi = maxn;
      {
        var q = 0;
        while ((q < 4))
        {
          {
            var w = 0;
            while ((w < 4))
            {
              {
                var e = 0;
                while ((e < 4))
                {
                  {
                    var r = 0;
                    while ((r < 4))
                    {
                      var y: dynamic;
                      {
                        var p = 0;
                        while ((p < 4))
                        {
                          {
                            var u = (p + 1);
                            while ((u < 4))
                            {
                              y.push_back(dist(c[p], c[u]));
                              u += 1;
                            }
                          }
                          p += 1;
                        }
                      }
                      sort(y.begin(), y.end());
                      if (((((((y[0] == y[1]) && (y[0] == y[2])) && (y[0] == y[3])) && (y[4] == y[5])) && ((cpp_cast(2) * y[0]) == y[5])) && (y[0] != 0)))
                      {
                        mi = min(mi, (((q + w) + e) + r));
                      }
                      c[3] = newval(c[3], b[3]);
                      r += 1;
                    }
                  }
                  c[2] = newval(c[2], b[2]);
                  e += 1;
                }
              }
              c[1] = newval(c[1], b[1]);
              w += 1;
            }
          }
          c[0] = newval(c[0], b[0]);
          q += 1;
        }
      }
      if ((mi == maxn))
      {
        write(-1, "\n");
      } else
      {
        write(mi, "\n");
      }
      i += 1;
    }
  }
  return 0;
}
