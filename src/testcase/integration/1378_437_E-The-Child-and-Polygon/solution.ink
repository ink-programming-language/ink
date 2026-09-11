// Translated from solution.cpp.

var v: dynamic = cpp_array(300);

var O: dynamic = cpp_uninitialized();

var M: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var nr: dynamic = cpp_uninitialized();

var K: dynamic = cpp_uninitialized();

var nri: dynamic = cpp_uninitialized();

var OK: dynamic = cpp_array(300, 300);

var nx: dynamic = cpp_array(300);

var val1: dynamic = cpp_uninitialized();

var val2: dynamic = cpp_uninitialized();

var val: dynamic = cpp_uninitialized();

var A1: dynamic = cpp_uninitialized();

var dt: dynamic = cpp_uninitialized();

var A2: dynamic = cpp_uninitialized();

var B1: dynamic = cpp_uninitialized();

var B2: dynamic = cpp_uninitialized();

var C1: dynamic = cpp_uninitialized();

var C2: dynamic = cpp_uninitialized();

var D: dynamic = cpp_array(300, 300);

var S: dynamic = cpp_uninitialized();

var d1: dynamic = cpp_uninitialized();

var d2: dynamic = cpp_uninitialized();

var d3: dynamic = cpp_uninitialized();

var d4: dynamic = cpp_uninitialized();

var xm: dynamic = cpp_uninitialized();

var ym: dynamic = cpp_uninitialized();

var SP: dynamic = cpp_uninitialized();

func aba(a: dynamic) -> dynamic
{
  if ((a < 0))
  {
    return (-a);
  }
  return a;
}

func detu(ax: dynamic, ay: dynamic, bx: dynamic, by: dynamic, cx: dynamic, cy: dynamic) -> dynamic
{
  return ((((((ax * by) + (bx * cy)) + (cx * ay)) - (ay * bx)) - (by * cx)) - (cy * ax));
}

func det(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  var dt: dynamic = ((((((a.first * b.second) + (b.first * c.second)) + (c.first * a.second)) - (a.second * b.first)) - (b.second * c.first)) - (c.second * a.first));
  if ((dt > 0))
  {
    dt = 1;
  }
  if ((dt < 0))
  {
    dt = -1;
  }
  return dt;
}

func intersect(a: dynamic, b: dynamic, c: dynamic, d: dynamic) -> dynamic
{
  var d1: dynamic = det(a, b, c);
  var d2: dynamic = det(a, b, d);
  var d3: dynamic = det(c, d, a);
  var d4: dynamic = det(c, d, b);
  if (((((!d1) || (!d2)) || (!d3)) || (!d4)))
  {
    return 0;
  }
  if (((d1 == (-d2)) && (d3 == (-d4))))
  {
    return 1;
  } else
  {
    return 0;
  }
}

func inside(M: dynamic) -> dynamic
{
  var c: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((((((det(O, M, v[i]) == 0) && (v[i].first >= O.first)) && (v[i].first <= M.first)) && (v[i].second >= min(O.second, M.second))) && (v[i].second <= max(O.second, M.second))))
      {
        c = (1 - c);
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if (intersect(O, M, v[i], v[nx[i]]))
      {
        c = (1 - c);
      }
      i += 1;
    }
  }
  return c;
}

func solve(st: dynamic, dr: dynamic) -> dynamic
{
  if ((((nx[st] == dr) || (nx[nx[st]] == dr)) || (st == dr)))
  {
    return 1;
  }
  if (D[st][dr])
  {
    return D[st][dr];
  }
  {
    var i: dynamic = nx[st];
    while ((i != dr))
    {
      if ((OK[st][i] && OK[dr][i]))
      {
        D[st][dr] += ((solve(st, i) * solve(i, dr)) % 1000000007);
        if ((D[st][dr] >= 1000000007))
        {
          D[st][dr] -= 1000000007;
        }
      }
      i = nx[i];
    }
  }
  return D[st][dr];
}

func main() -> dynamic
{
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(v[i].first, v[i].second);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      nx[i] = (i + 1);
      i += 1;
    }
  }
  nx[n] = 1;
  O.first = -1000000007;
  O.second = -1000000009;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = (i + 1);
        while ((j <= n))
        {
          K = 1;
          OK[i][j] = cpp_assign(OK[j][i], "=", 1);
          if ((j == (i + 1)))
          {
            j += 1;
            continue;
          }
          if (((i == 1) && (j == n)))
          {
            j += 1;
            continue;
          }
          {
            var k: dynamic = 1;
            while ((k <= n))
            {
              if (((i == k) || (j == k)))
              {
                k += 1;
                continue;
              }
              if ((((((det(v[i], v[j], v[k]) == 0) && (v[k].first >= min(v[i].first, v[j].first))) && (v[k].first <= max(v[i].first, v[j].first))) && (v[k].second >= min(v[i].second, v[j].second))) && (v[k].second <= max(v[i].second, v[j].second))))
              {
                K = 0;
                break;
              }
              k += 1;
            }
          }
          if ((!K))
          {
            OK[i][j] = cpp_assign(OK[j][i], "=", 0);
            j += 1;
            continue;
          }
          {
            var k: dynamic = 1;
            while ((k <= n))
            {
              if (intersect(v[i], v[j], v[k], v[nx[k]]))
              {
                K = 0;
                break;
              }
              k += 1;
            }
          }
          if ((!K))
          {
            OK[i][j] = cpp_assign(OK[j][i], "=", 0);
            j += 1;
            continue;
          }
          M.first = (v[i].first + v[j].first);
          M.second = (v[i].second + v[j].second);
          M.first /= 2;
          M.second /= 2;
          O.first -= (M.first - 1);
          O.second -= M.second;
          if ((!inside(M)))
          {
            OK[i][j] = cpp_assign(OK[j][i], "=", 0);
          }
          O.first += M.first;
          O.second += M.second;
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(solve(1, n));
  return 0;
}
