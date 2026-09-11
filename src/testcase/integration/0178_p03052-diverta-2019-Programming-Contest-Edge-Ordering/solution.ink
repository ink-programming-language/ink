// Translated from solution.cpp.

var N: dynamic = 20;

var mod: dynamic = 1000000007;

func add(a: dynamic, b: dynamic, p: dynamic = mod) -> dynamic
{
  return  (((a + b) >= p)) ? ((a + b) - p) : (a + b);
}

func sub(a: dynamic, b: dynamic, p: dynamic = mod) -> dynamic
{
  return  (((a - b) < 0)) ? ((a - b) + p) : (a - b);
}

func mul(a: dynamic, b: dynamic, p: dynamic = mod) -> dynamic
{
  return ((cpp_cast(a) * b) % p);
}

func sadd(a: dynamic, b: dynamic, p: dynamic = mod) -> dynamic
{
  a = add(a, b, p);
}

func ssub(a: dynamic, b: dynamic, p: dynamic = mod) -> dynamic
{
  a = sub(a, b, p);
}

func smul(a: dynamic, b: dynamic, p: dynamic = mod) -> dynamic
{
  a = mul(a, b, p);
}

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

class side0
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
}

var a: dynamic = cpp_array(((N * N) + 9));

var to: dynamic = cpp_array((N + 9));

func into() -> dynamic
{
  scanf("%d%d", (&n), (&m));
  {
    var i: dynamic = 0;
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

var inv: dynamic = cpp_array(((N * N) + 9));

var fac: dynamic = cpp_array(((N * N) + 9));

var ifac: dynamic = cpp_array(((N * N) + 9));

func Get_inv() -> dynamic
{
  inv[1] = 1;
  fac[0] = cpp_assign(fac[1], "=", 1);
  ifac[0] = cpp_assign(ifac[1], "=", 1);
  {
    var i: dynamic = 2;
    while ((i <= m))
    {
      inv[i] = mul((mod - (mod / i)), inv[(mod % i)]);
      fac[i] = mul(fac[(i - 1)], i);
      ifac[i] = mul(ifac[(i - 1)], inv[i]);
      i += 1;
    }
  }
}

var c: dynamic = cpp_array((((1 << N)) + 9));

var ce: dynamic = cpp_array((((1 << N)) + 9));

func Get_e() -> dynamic
{
  {
    var s: dynamic = 0;
    while ((s < (1 << n)))
    {
      c[s] = (c[(s >> 1)] + ((s & 1)));
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          if (((s >> i) & 1))
          {
            var t: dynamic = (to[i] & s);
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
  var y: dynamic = cpp_uninitialized();
  var next: dynamic = cpp_uninitialized();
}

var e: dynamic = cpp_array(((N * 2) + 9));

var lin: dynamic = cpp_array((N + 9));

var cs: dynamic = cpp_uninitialized();

func Ins(x: dynamic, y: dynamic) -> dynamic
{
  e[cpp_update(cs, "++")].y = y;
  e[cs].next = lin[x];
  lin[x] = cs;
}

func Ins2(x: dynamic, y: dynamic) -> dynamic
{
  Ins(x, y);
  Ins(y, x);
}

var vis: dynamic = cpp_array((N + 9));

func Dfs_vis(k: dynamic) -> dynamic
{
  var res: dynamic = (1 << k);
  vis[k] = 1;
  {
    var i: dynamic = lin[k];
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

var num: dynamic = cpp_array((((1 << N)) + 9));

func Get_num() -> dynamic
{
  {
    var s: dynamic = 0;
    while ((s < (1 << (n - 1))))
    {
      cs = 0;
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          lin[i] = cpp_assign(vis[i], "=", 0);
          i += 1;
        }
      }
      {
        var i: dynamic = 0;
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
        var i: dynamic = 0;
        while ((i < n))
        {
          if ((!vis[i]))
          {
            var t: dynamic = Dfs_vis(i);
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
  var c: dynamic = cpp_uninitialized();
  var cnt: dynamic = cpp_uninitialized();
  var sum: dynamic = cpp_uninitialized();
}

var dp: dynamic = cpp_array((((1 << N)) + 9));

func Get_dp() -> dynamic
{
  dp[0].cnt = 1;
  dp[0].sum = 0;
  {
    var s: dynamic = 0;
    while ((s < (1 << (n - 1))))
    {
      {
        var i: dynamic = 0;
        while ((i < (n - 1)))
        {
          if (((s >> i) & 1))
          {
            i += 1;
            continue;
          }
          var delta: dynamic = (num[((((1 << (n - 1))) - 1) ^ s)] - num[(((((1 << (n - 1))) - 1) ^ s) ^ (1 << i))]);
          dp[(s | (1 << i))].c = ((dp[s].c + delta) + 1);
          var cnt: dynamic = mul(dp[s].cnt, mul(fac[(dp[s].c + delta)], ifac[dp[s].c]));
          var sum: dynamic = mul(dp[s].sum, mul(fac[((dp[s].c + delta) + 1)], ifac[(dp[s].c + 1)]));
          sadd(dp[(s | (1 << i))].cnt, cnt);
          sadd(dp[(s | (1 << i))].sum, add(sum, mul(cnt, (c[s] + 1))));
          i += 1;
        }
      }
      s += 1;
    }
  }
}

func work() -> dynamic
{
  Get_inv();
  Get_e();
  Get_num();
  Get_dp();
}

func outo() -> dynamic
{
  printf("%d\n", dp[(((1 << (n - 1))) - 1)].sum);
}

func main() -> dynamic
{
  into();
  work();
  outo();
  return 0;
}
