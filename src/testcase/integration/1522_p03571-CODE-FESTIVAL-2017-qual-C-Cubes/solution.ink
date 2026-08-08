// Translated from solution.cpp.

var mod = 1000000007;

func get(lx: dynamic, ly: dynamic, ux: dynamic, uy: dynamic, a: dynamic, b: dynamic)
{
  var x: dynamic;
  var y: dynamic;
  x.push_back(lx);
  x.push_back((ux + 1));
  x.push_back(0);
  x.push_back(a);
  y.push_back(ly);
  y.push_back((uy + 1));
  y.push_back(0);
  y.push_back(b);
  sort(x.begin(), x.end());
  sort(y.begin(), y.end());
  var r = 0;
  {
    var i = 0;
    while ((i < 3))
    {
      {
        var j = 0;
        while ((j < 3))
        {
          if (((((0 <= x[i]) && (x[i] < a)) && (0 <= y[j])) && (y[j] < b)))
          {
            j += 1;
            continue;
          }
          if (((((lx <= x[i]) && (x[i] <= ux)) && (ly <= y[j])) && (y[j] <= uy)))
          {
            r += (((x[(i + 1)] - x[i])) * ((y[(j + 1)] - y[j])));
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  return (r % mod);
}

func calc(a: dynamic, b: dynamic, c: dynamic, d: dynamic)
{
  if ((c == 1))
  {
    return 0;
  }
  var v: dynamic;
  {
    var i = 0;
    while ((i <= (d + 1)))
    {
      {
        var j = -1;
        while ((j <= 2))
        {
          v.push_back(((((c * i) / a) + j) + d));
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= (d + 1)))
    {
      {
        var j = -1;
        while ((j <= 2))
        {
          v.push_back(((((c * i) / b) + j) + d));
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= (d + 1)))
    {
      {
        var j = -1;
        while ((j <= 2))
        {
          v.push_back(((((c * ((a - i))) / a) + j) + d));
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= (d + 1)))
    {
      {
        var j = -1;
        while ((j <= 2))
        {
          v.push_back(((((c * ((b - i))) / b) + j) + d));
          j += 1;
        }
      }
      i += 1;
    }
  }
  v.push_back(c);
  v.push_back((c + d));
  {
    var i = 0;
    while ((i < v.size()))
    {
      v[i] = max((d + 1), min((c + d), v[i]));
      i += 1;
    }
  }
  sort(v.begin(), v.end());
  var r = 0;
  {
    var i = 0;
    while ((i < (v.size() - 1)))
    {
      var x = ((((v[i] - d)) * a) / c);
      var y = ((((v[i] - d)) * b) / c);
      var t = 0;
      if (((0 <= v[i]) && (v[i] < c)))
      {
        t = (get((x - d), (y - d), (x + d), (y + d), a, b) % mod);
      } else
      {
        t = ((((2 * d) + 1)) * (((2 * d) + 1)));
      }
      r += (t * ((v[(i + 1)] - v[i])));
      r %= mod;
      i += 1;
    }
  }
  return r;
}

func main()
{
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  var d: dynamic;
  scanf("%lld%lld%lld%lld", (&a), (&b), (&c), (&d));
  var ans = (((((2 * d) + 1)) * (((2 * d) + 1))) * (((2 * d) + 1)));
  ans = (((ans + (((((((((a - 1) + b) - 1) + c) - 1)) * (((2 * d) + 1))) % mod) * (((2 * d) + 1))))) % mod);
  var s = ((calc(a, b, c, d) + calc(b, c, a, d)) + calc(c, a, b, d));
  {
    var i = (-d);
    while ((i <= d))
    {
      if (((0 <= i) && (i < c)))
      {
        s += get((-d), (-d), d, d, a, b);
      } else
      {
        s += ((((2 * d) + 1)) * (((2 * d) + 1)));
      }
      i += 1;
    }
  }
  s %= mod;
  printf("%lld\n", ((((ans + mod) - s)) % mod));
}
