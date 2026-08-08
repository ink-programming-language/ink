// Translated from solution.cpp.

var eps = 1e-9;

var pi = acos(-1.0);

var inf = 0x3f3f3f3f;

var W = 0;

var B = 1;

var R = 2;

var Y = 3;

var N = 8;

var mod = 1000000007;

var inv = 500000004;

var l: dynamic;

var r: dynamic;

var mtx = cpp_array(N, N);

var a = cpp_array(N, N);

var st = cpp_array(4, 4);

var ts = cpp_array(N);

func check(i: dynamic, j: dynamic)
{
  if ((i == j))
  {
    return false;
  }
  if (((j == W) && (i == Y)))
  {
    return false;
  }
  if (((j == Y) && (i == W)))
  {
    return false;
  }
  if (((j == R) && (i == B)))
  {
    return false;
  }
  if (((j == B) && (i == R)))
  {
    return false;
  }
  return true;
}

func init()
{
  var tot = 0;
  memset(st, -1, cpp_sizeof((st)));
  {
    var i = 0;
    while ((i < cpp_cast((4))))
    {
      {
        var j = 0;
        while ((j < cpp_cast((4))))
        {
          if (check(i, j))
          {
            st[i][j] = tot;
            ts[cpp_update(tot, "++")] = ((i * 4) + j);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  memset(mtx, 0, cpp_sizeof((mtx)));
  {
    var i = 0;
    while ((i < cpp_cast((N))))
    {
      {
        var k = 0;
        while ((k < cpp_cast((4))))
        {
          var a = (ts[i] / 4);
          var b = (ts[i] % 4);
          if ((!check(b, k)))
          {
            k += 1;
            continue;
          }
          if ((((a == B) && (b == W)) && (k == R)))
          {
            k += 1;
            continue;
          }
          if ((((a == R) && (b == W)) && (k == B)))
          {
            k += 1;
            continue;
          }
          mtx[i][st[b][k]] = 1;
          k += 1;
        }
      }
      i += 1;
    }
  }
}

func add(dst: dynamic, a: dynamic, b: dynamic)
{
  var c = cpp_array(N, N);
  {
    var i = 0;
    while ((i < cpp_cast((N))))
    {
      {
        var j = 0;
        while ((j < cpp_cast((N))))
        {
          c[i][j] = (((a[i][j] + b[i][j])) % mod);
          j += 1;
        }
      }
      i += 1;
    }
  }
  memcpy(dst, c, cpp_sizeof((c)));
}

func mult(dst: dynamic, a: dynamic, b: dynamic)
{
  var c = cpp_array(N, N);
  memset(c, 0, cpp_sizeof((c)));
  {
    var i = 0;
    while ((i < cpp_cast((N))))
    {
      {
        var j = 0;
        while ((j < cpp_cast((N))))
        {
          {
            var k = 0;
            while ((k < cpp_cast((N))))
            {
              c[i][j] = (((c[i][j] + (cpp_cast(a[i][k]) * b[k][j]))) % mod);
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  memcpy(dst, c, cpp_sizeof((c)));
}

func power(dst: dynamic, a: dynamic, k: dynamic)
{
  var r = cpp_array(N, N);
  var t = cpp_array(N, N);
  memset(r, 0, cpp_sizeof((r)));
  memcpy(t, a, cpp_sizeof((t)));
  {
    var i = 0;
    while ((i < cpp_cast((N))))
    {
      r[i][i] = 1;
      i += 1;
    }
  }
  while (k)
  {
    if ((k & 1))
    {
      mult(r, t, r);
    }
    mult(t, t, t);
    k >>= 1;
  }
  memcpy(dst, r, cpp_sizeof((r)));
}

func calc(dst: dynamic, a: dynamic, k: dynamic)
{
  if ((k == 1))
  {
    memcpy(dst, a, ((cpp_sizeof(dynamic) * N) * N));
    return;
  }
  var x = cpp_array(N, N);
  var y = cpp_array(N, N);
  power(x, a, (k / 2));
  calc(y, a, (k / 2));
  {
    var i = 0;
    while ((i < cpp_cast((N))))
    {
      x[i][i] += 1;
      i += 1;
    }
  }
  mult(dst, x, y);
  if ((k & 1))
  {
    power(x, a, k);
    add(dst, dst, x);
  }
}

func f(n: dynamic)
{
  if ((n == 2))
  {
    return 8;
  }
  var a = cpp_array(N, N);
  var ans = 8;
  calc(a, mtx, (n - 2));
  {
    var i = 0;
    while ((i < cpp_cast((N))))
    {
      {
        var j = 0;
        while ((j < cpp_cast((N))))
        {
          ans = (((ans + a[i][j])) % mod);
          j += 1;
        }
      }
      i += 1;
    }
  }
  return ans;
}

func gao(n: dynamic)
{
  if ((n == 0))
  {
    return 0;
  }
  if ((n == 1))
  {
    return 4;
  }
  if ((n == 2))
  {
    return 8;
  }
  var ans = (f(n) + f((((n + 1)) / 2)));
  ans = ((cpp_cast(ans) * inv) % mod);
  return (((((ans % mod) + mod) + 4)) % mod);
}

func solve()
{
  var ans = (gao(r) - gao((l - 1)));
  ans = ((((ans % mod) + mod)) % mod);
  printf("%d\n", cpp_cast(ans));
}

func main()
{
  init();
  while ((scanf("%d%d", (&l), (&r)) != EOF))
  {
    solve();
  }
  return 0;
}
