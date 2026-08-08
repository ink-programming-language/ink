// Translated from solution.cpp.

var N = 20;

var mod = 1000000007;

func add(a: dynamic, b: dynamic, p: dynamic = mod)
{
  return if (((a + b) >= p)) ((a + b) - p) else (a + b);
}

func sub(a: dynamic, b: dynamic, p: dynamic = mod)
{
  return if (((a - b) < 0)) ((a - b) + p) else (a - b);
}

func mul(a: dynamic, b: dynamic, p: dynamic = mod)
{
  return ((cpp_cast(a) * b) % p);
}

func sadd(a: dynamic, b: dynamic, p: dynamic = mod)
{
  a = add(a, b, p);
}

func ssub(a: dynamic, b: dynamic, p: dynamic = mod)
{
  a = sub(a, b, p);
}

func smul(a: dynamic, b: dynamic, p: dynamic = mod)
{
  a = mul(a, b, p);
}

var n: dynamic;

var m: dynamic;

class side0
{
  var x: dynamic;
  var y: dynamic;
}

var a = cpp_array(((N * N) + 9));

var to = cpp_array((N + 9));

func into()
{
  scanf("%d%d", (&n), (&m));
  {
    var i = 0;
    while ((i < m))
    {
      scanf("%d%d", (&a[i].x), (&a[i].y));
      a[i].x -= 1;
      a[i].y -= 1;
      to[a[i].x] |= (1 << a[i].y);
      to[a[i].y] |= (1 << a[i].x);
      i += 1;
    }
  }
}

var inv = cpp_array(((N * N) + 9));

var fac = cpp_array(((N * N) + 9));

var ifac = cpp_array(((N * N) + 9));

func Get_inv()
{
  inv[1] = 1;
  fac[0] = cpp_assign(fac[1], "=", 1);
  ifac[0] = cpp_assign(ifac[1], "=", 1);
  {
    var i = 2;
    while ((i <= m))
    {
      inv[i] = mul((mod - (mod / i)), inv[(mod % i)]);
      fac[i] = mul(fac[(i - 1)], i);
      ifac[i] = mul(ifac[(i - 1)], inv[i]);
      i += 1;
    }
  }
}

var c = cpp_array((((1 << N)) + 9));

var ce = cpp_array((((1 << N)) + 9));

func Get_e()
{
  {
    var s = 0;
    while ((s < (1 << n)))
    {
      c[s] = (c[(s >> 1)] + ((s & 1)));
      {
        var i = 0;
        while ((i < n))
        {
          if (((s >> i) & 1))
          {
            var t = (to[i] & s);
            ce[s] = (ce[(s ^ (1 << i))] + c[t]);
            break;
          }
          i += 1;
        }
      }
      s += 1;
    }
  }
}

class side
{
  var y: dynamic;
  var next: dynamic;
}

var e = cpp_array(((N * 2) + 9));

var lin = cpp_array((N + 9));

var cs: dynamic;

func Ins(x: dynamic, y: dynamic)
{
  e[cpp_update(cs, "++")].y = y;
  e[cs].next = lin[x];
  lin[x] = cs;
}

func Ins2(x: dynamic, y: dynamic)
{
  Ins(x, y);
  Ins(y, x);
}

var vis = cpp_array((N + 9));

func Dfs_vis(k: dynamic)
{
  var res = (1 << k);
  vis[k] = 1;
  {
    var i = lin[k];
    while (i)
    {
      if ((!vis[e[i].y]))
      {
        res |= Dfs_vis(e[i].y);
      }
      i = e[i].next;
    }
  }
  return res;
}

var num = cpp_array((((1 << N)) + 9));

func Get_num()
{
  {
    var s = 0;
    while ((s < (1 << (n - 1))))
    {
      cs = 0;
      {
        var i = 0;
        while ((i < n))
        {
          lin[i] = cpp_assign(vis[i], "=", 0);
          i += 1;
        }
      }
      {
        var i = 0;
        while ((i < (n - 1)))
        {
          if (((s >> i) & 1))
          {
            Ins2(a[i].x, a[i].y);
          }
          i += 1;
        }
      }
      {
        var i = 0;
        while ((i < n))
        {
          if ((!vis[i]))
          {
            var t = Dfs_vis(i);
            num[s] += ((ce[t] - c[t]) + 1);
          }
          i += 1;
        }
      }
      s += 1;
    }
  }
}

class state
{
  var c: dynamic;
  var cnt: dynamic;
  var sum: dynamic;
}

var dp = cpp_array((((1 << N)) + 9));

func Get_dp()
{
  dp[0].cnt = 1;
  dp[0].sum = 0;
  {
    var s = 0;
    while ((s < (1 << (n - 1))))
    {
      {
        var i = 0;
        while ((i < (n - 1)))
        {
          if (((s >> i) & 1))
          {
            i += 1;
            continue;
          }
          var delta = (num[((((1 << (n - 1))) - 1) ^ s)] - num[(((((1 << (n - 1))) - 1) ^ s) ^ (1 << i))]);
          dp[(s | (1 << i))].c = ((dp[s].c + delta) + 1);
          var cnt = mul(dp[s].cnt, mul(fac[(dp[s].c + delta)], ifac[dp[s].c]));
          var sum = mul(dp[s].sum, mul(fac[((dp[s].c + delta) + 1)], ifac[(dp[s].c + 1)]));
          sadd(dp[(s | (1 << i))].cnt, cnt);
          sadd(dp[(s | (1 << i))].sum, add(sum, mul(cnt, (c[s] + 1))));
          i += 1;
        }
      }
      s += 1;
    }
  }
}

func work()
{
  Get_inv();
  Get_e();
  Get_num();
  Get_dp();
}

func outo()
{
  printf("%d\n", dp[(((1 << (n - 1))) - 1)].sum);
}

func main()
{
  into();
  work();
  outo();
  return 0;
}
