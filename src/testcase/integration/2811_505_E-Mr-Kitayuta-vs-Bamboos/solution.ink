// Translated from solution.cpp.

var N = (2e5 + 100);

var OO = (1e9 + 7);

var T = 22;

var M = (1e9 + 7);

var P = 6151;

var SQ = 1300;

var lg = 22;

var h = cpp_array(N);

var a = cpp_array(N);

var f = cpp_array(N);

var n: dynamic;

var m: dynamic;

var k: dynamic;

var p: dynamic;

var ct = cpp_array(N);

var mx = 0;

func check(x: dynamic)
{
  var cnt = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      if ((x >= f[i]))
      {
        i += 1;
        continue;
      }
      cnt += (((((f[i] - x) + p) - 1)) / p);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= (m + 1)))
    {
      ct[i] = 0;
      i += 1;
    }
  }
  if ((cnt > (k * ((m + 1)))))
  {
    return false;
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if ((x >= f[i]))
      {
        i += 1;
        continue;
      }
      var val = (((((f[i] - x) + p) - 1)) / p);
      var pos = 0;
      var cnt = h[i];
      while ((val && (pos <= m)))
      {
        if ((cnt >= p))
        {
          val -= 1;
          cnt -= p;
          continue;
        }
        var g = (((((p - cnt) + a[i]) - 1)) / a[i]);
        if (((g + pos) > m))
        {
          cnt += (((m - pos)) * a[i]);
          if (((val > 1) || ((max(cpp_cast(0), (cnt - p)) + a[i]) > x)))
          {
            return false;
          }
          cnt -= (((m - pos)) * a[i]);
          ct[min((x / a[i]), (m + 1))] += 1;
          break;
        }
        cnt += (g * a[i]);
        cnt -= p;
        val -= 1;
        pos += g;
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= (m + 1)))
    {
      ct[i] += ct[(i - 1)];
      if (((i * k) < ct[i]))
      {
        return false;
      }
      i += 1;
    }
  }
  return true;
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  read(n, m, k, p);
  {
    var i = 1;
    while ((i <= n))
    {
      read(h[i], a[i]);
      f[i] = (h[i] + (a[i] * m));
      mx = max(mx, f[i]);
      i += 1;
    }
  }
  m -= 1;
  var l = 0;
  var r = mx;
  while ((l < r))
  {
    var mid = (((l + r)) >> 1);
    if (check(mid))
    {
      r = mid;
    } else
    {
      l = (mid + 1);
    }
  }
  write(r, "\n");
  return 0;
}
