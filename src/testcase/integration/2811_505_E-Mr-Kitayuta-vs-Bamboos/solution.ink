// Translated from solution.cpp.

var N: dynamic = (2e5 + 100);

var OO: dynamic = (1e9 + 7);

var T: dynamic = 22;

var M: dynamic = (1e9 + 7);

var P: dynamic = 6151;

var SQ: dynamic = 1300;

var lg: dynamic = 22;

var h: dynamic = cpp_array(N);

var a: dynamic = cpp_array(N);

var f: dynamic = cpp_array(N);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var p: dynamic = cpp_uninitialized();

var ct: dynamic = cpp_array(N);

var mx: dynamic = 0;

func check(x: dynamic) -> dynamic
{
  var cnt: dynamic = 0;
  {
    var i: dynamic = 1;
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
    var i: dynamic = 0;
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
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((x >= f[i]))
      {
        i += 1;
        continue;
      }
      var val: dynamic = (((((f[i] - x) + p) - 1)) / p);
      var pos: dynamic = 0;
      var cnt: dynamic = h[i];
      while ((val && (pos <= m)))
      {
        if ((cnt >= p))
        {
          val -= 1;
          cnt -= p;
          continue;
        }
        var g: dynamic = (((((p - cnt) + a[i]) - 1)) / a[i]);
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
    var i: dynamic = 1;
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

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  read(n, m, k, p);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(h[i], a[i]);
      f[i] = (h[i] + (a[i] * m));
      mx = max(mx, f[i]);
      i += 1;
    }
  }
  m -= 1;
  var l: dynamic = 0;
  var r: dynamic = mx;
  while ((l < r))
  {
    var mid: dynamic = (((l + r)) >> 1);
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
