// Translated from solution.cpp.

var maxn: dynamic = 100005;

var maxk: dynamic = 105;

var f: dynamic = cpp_uninitialized();

var tmp: dynamic = cpp_uninitialized();

var g: dynamic = cpp_array(maxn);

var t: dynamic = cpp_array(maxn);

var ans: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  scanf("%d%d", (&n), (&k));
  f.push_back(make_pair(1, 1.0));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      tmp.clear();
      {
        var k: dynamic = 0;
        while ((k < cpp_cast(f.size())))
        {
          var j: dynamic = f[k].first;
          var p: dynamic = f[k].second;
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
      var last: dynamic = tmp[0];
      f.clear();
      {
        var k: dynamic = 1;
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
    var i: dynamic = 1;
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
      var i: dynamic = 1;
      while ((i <= n))
      {
        t[0] = ((t[0] + log((k - 1))) - log(k));
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        t[i] = (((t[(i - 1)] + log(((n - i) + 1))) - log(i)) - log((k - 1)));
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
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
    var i: dynamic = 0;
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
