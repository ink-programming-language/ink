// Translated from solution.cpp.

var inf = (1e9 + 69);

var MX = (5e5 + 5);

var LG = cpp_cast(log2(MX));

var mod = (1e9 + 7);

var BLOCK = 450;

var rng = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());

var n: dynamic;

var v: dynamic;

var cnt: dynamic;

var q: dynamic;

var inv: dynamic;

var val = 0;

func reset()
{
  q.assign((n + 1), 0);
  inv.assign((n + 1), 0);
  val = 0;
}

func push(nw: dynamic)
{
  inv[q[nw]] -= 1;
  q[nw] += 1;
  inv[q[nw]] += 1;
  if ((q[nw] > val))
  {
    val = q[nw];
  }
}

func pop(nw: dynamic)
{
  inv[q[nw]] -= 1;
  q[nw] -= 1;
  inv[q[nw]] += 1;
  if ((inv[val] == 0))
  {
    val -= 1;
  }
}

func check()
{
  return (inv[val] >= 2);
}

func main()
{
  cin.tie(0)->sync_with_stdio(0);
  read(n);
  v.resize((n + 1));
  cnt.resize((n + 1));
  {
    var i = 1;
    while ((i <= n))
    {
      read(v[i]);
      cnt[v[i]] += 1;
      i += 1;
    }
  }
  var modus: dynamic;
  var cntmx = 0;
  {
    var i = 1;
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
  var fi = modus[0];
  if ((cnt[fi] == n))
  {
    write(0, "\n");
    return 0;
  }
  var ans = 0;
  {
    var i = 1;
    while ((i <= BLOCK))
    {
      reset();
      var lf = 1;
      {
        var rg = 1;
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
    var sc = 1;
    while ((sc <= n))
    {
      if (((sc == fi) || (cnt[sc] < BLOCK)))
      {
        sc += 1;
        continue;
      }
      var presum = cpp_construct(((2 * n) + 5), -1);
      presum[(n + 2)] = 0;
      var sm = 0;
      {
        var i = 1;
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
