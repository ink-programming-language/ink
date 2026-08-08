// Translated from solution.cpp.

var maxn = 100005;

var maxk = 105;

var f: dynamic;

var tmp: dynamic;

var g = cpp_array(maxn);

var t = cpp_array(maxn);

var ans: dynamic;

var n: dynamic;

var k: dynamic;

func main()
{
  scanf("%d%d", (&n), (&k));
  f.push_back(make_pair(1, 1.0));
  {
    var i = 0;
    while ((i < n))
    {
      tmp.clear();
      {
        var k = 0;
        while ((k < cpp_cast(f.size())))
        {
          var j = f[k].first;
          var p = f[k].second;
          if ((p < 1e-15))
          {
            k += 1;
            continue;
          }
          g[(i + 1)] += ((p * (((j * 1.0) / ((j + 1))))) * ((((j + 1)) / 2.0)));
          tmp.push_back(make_pair(j, (p * (((j * 1.0) / ((j + 1)))))));
          g[(i + 1)] += ((p * ((1.0 / ((j + 1))))) * j);
          tmp.push_back(make_pair((j + 1), (p * ((1.0 / ((j + 1)))))));
          k += 1;
        }
      }
      var last = tmp[0];
      f.clear();
      {
        var k = 1;
        while ((k < cpp_cast(tmp.size())))
        {
          if ((tmp[k].first == last.first))
          {
            last.second += tmp[k].second;
          } else
          {
            f.push_back(last);
            last = tmp[k];
          }
          k += 1;
        }
      }
      f.push_back(last);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      g[i] += g[(i - 1)];
      i += 1;
    }
  }
  if ((k > 1))
  {
    t[0] = log(1);
    {
      var i = 1;
      while ((i <= n))
      {
        t[0] = ((t[0] + log((k - 1))) - log(k));
        i += 1;
      }
    }
    {
      var i = 1;
      while ((i <= n))
      {
        t[i] = (((t[(i - 1)] + log(((n - i) + 1))) - log(i)) - log((k - 1)));
        i += 1;
      }
    }
    {
      var i = 1;
      while ((i <= n))
      {
        t[i] = exp(t[i]);
        i += 1;
      }
    }
  } else
  {
    t[n] = 1;
  }
  {
    var i = 0;
    while ((i <= n))
    {
      ans += (g[i] * t[i]);
      i += 1;
    }
  }
  ans *= k;
  printf("%.100lf\n", cpp_cast(ans));
  return 0;
}
