// Translated from solution.cpp.

var eps = 1e-12;

var maxn = (100000 + 1912);

var MX = 1e6;

var n: dynamic;

var a = cpp_array(maxn);

var p = cpp_array(((MX * 2) + 3));

var res = 0;

func ReadData()
{
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d%d", (&a[i].first), (&a[i].second));
      a[i].first += MX;
      a[i].second += MX;
      p[a[i].first].push_back(a[i].second);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= (MX * 2)))
    {
      if ((cpp_cast(p[i].size())))
      {
        sort(p[i].begin(), p[i].end());
      }
      i += 1;
    }
  }
}

func Found(x: dynamic, y: dynamic)
{
  if (((x < 0) || (x > (MX * 2))))
  {
    return false;
  }
  var it = lower_bound(p[x].begin(), p[x].end(), y);
  if (((it != p[x].end()) && (((*it)) == y)))
  {
    return true;
  }
  return false;
}

func Process()
{
  {
    var i = 0;
    while ((i <= (MX * 2)))
    {
      if ((cpp_cast(p[i].size())))
      {
        if (((cpp_cast(p[i].size())) <= 520))
        {
          {
            var fi = 0;
            while ((fi < (cpp_cast(p[i].size()))))
            {
              {
                var se = 0;
                while ((se < fi))
                {
                  var len = (p[i][fi] - p[i][se]);
                  if ((((i >= len) && Found((i - len), p[i][se])) && Found((i - len), p[i][fi])))
                  {
                    res += 1;
                  }
                  se += 1;
                }
              }
              fi += 1;
            }
          }
        } else
        {
          {
            var j = 0;
            while ((j <= (i - 1)))
            {
              var len = (i - j);
              {
                var k = 0;
                while ((k < (cpp_cast(p[j].size()))))
                {
                  if (((Found(i, p[j][k]) && Found(j, (p[j][k] + len))) && Found(i, (p[j][k] + len))))
                  {
                    res += 1;
                  }
                  k += 1;
                }
              }
              j += 1;
            }
          }
        }
      }
      i += 1;
    }
  }
  write(res, "\n");
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  ReadData();
  Process();
}
