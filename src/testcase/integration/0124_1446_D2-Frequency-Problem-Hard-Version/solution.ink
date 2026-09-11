// Translated from solution.cpp.

var inf: dynamic = (1e9 + 69);

var MX: dynamic = (5e5 + 5);

var LG: dynamic = cpp_cast(log2(MX));

var mod: dynamic = (1e9 + 7);

var BLOCK: dynamic = 450;

var rng: dynamic = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());

var n: dynamic = cpp_uninitialized();

var v: dynamic = cpp_uninitialized();

var cnt: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var inv: dynamic = cpp_uninitialized();

var val: dynamic = 0;

func reset() -> dynamic
{
  q.assign((n + 1), 0);
  inv.assign((n + 1), 0);
  val = 0;
}

func push(nw: dynamic) -> dynamic
{
  inv[q[nw]] -= 1;
  q[nw] += 1;
  inv[q[nw]] += 1;
  if ((q[nw] > val))
  {
    val = q[nw];
  }
}

func pop(nw: dynamic) -> dynamic
{
  inv[q[nw]] -= 1;
  q[nw] -= 1;
  inv[q[nw]] += 1;
  if ((inv[val] == 0))
  {
    val -= 1;
  }
}

func check() -> dynamic
{
  return (inv[val] >= 2);
}

func main() -> dynamic
{
  cin.tie(0)->sync_with_stdio(0);
  read(n);
  v.resize((n + 1));
  cnt.resize((n + 1));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(v[i]);
      cnt[v[i]] += 1;
      i += 1;
    }
  }
  var modus: dynamic = cpp_uninitialized();
  var cntmx: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((cntmx < cnt[i]))
      {
        cntmx = cnt[i];
        modus.clear();
      }
      if ((cntmx == cnt[i]))
      {
        modus.push_back(i);
      }
      i += 1;
    }
  }
  if ((modus.size() > 1))
  {
    write(n, "\n");
    return 0;
  }
  var fi: dynamic = modus[0];
  if ((cnt[fi] == n))
  {
    write(0, "\n");
    return 0;
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= BLOCK))
    {
      reset();
      var lf: dynamic = 1;
      {
        var rg: dynamic = 1;
        while ((rg <= n))
        {
          push(v[rg]);
          {
            while ((val > i))
            {
              pop(v[lf]);
              lf += 1;
            }
          }
          if (check())
          {
            ans = max(ans, ((rg - lf) + 1));
          }
          rg += 1;
        }
      }
      i += 1;
    }
  }
  {
    var sc: dynamic = 1;
    while ((sc <= n))
    {
      if (((sc == fi) || (cnt[sc] < BLOCK)))
      {
        sc += 1;
        continue;
      }
      var presum: dynamic = cpp_construct(((2 * n) + 5), -1);
      presum[(n + 2)] = 0;
      var sm: dynamic = 0;
      {
        var i: dynamic = 1;
        while ((i <= n))
        {
          if ((v[i] == fi))
          {
            sm += 1;
          } else if ((v[i] == sc))
          {
            sm -= 1;
          }
          if ((presum[((sm + n) + 2)] == -1))
          {
            presum[((sm + n) + 2)] = i;
          }
          ans = max(ans, (i - presum[((sm + n) + 2)]));
          i += 1;
        }
      }
      sc += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
