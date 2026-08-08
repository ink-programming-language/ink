// Translated from solution.cpp.

var N = (3e5 + 5);

var MAGIC = 320;

var a = cpp_array(N);

var p = cpp_array(N);

var s = cpp_array(N);

var k: dynamic;

var now: dynamic;

var l: dynamic;

var r: dynamic;

var all: dynamic;

var at = cpp_array(N);

var atp = cpp_array(N);

var atm = cpp_array(N);

var cnt = cpp_array(N);

func add(i: dynamic, v: dynamic)
{
  cnt[at[i]] += v;
}

var ans = cpp_array(N);

func main()
{
  var n: dynamic;
  scanf("%d %lld", (&n), (&k));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (p + i));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (a + i));
      s[i] = (s[(i - 1)] + (if ((p[i] == 1)) a[i] else (-a[i])));
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= n))
    {
      all.push_back(s[i]);
      all.push_back((s[i] + k));
      all.push_back((s[i] - k));
      i += 1;
    }
  }
  sort(all.begin(), all.end());
  all.resize(distance(all.begin(), unique(all.begin(), all.end())));
  {
    var i = 0;
    while ((i <= n))
    {
      at[i] = (lower_bound(all.begin(), all.end(), s[i]) - all.begin());
      atp[i] = (lower_bound(all.begin(), all.end(), (s[i] + k)) - all.begin());
      atm[i] = (lower_bound(all.begin(), all.end(), (s[i] - k)) - all.begin());
      i += 1;
    }
  }
  var que: dynamic;
  var q: dynamic;
  scanf("%d", (&q));
  {
    var i = 1;
    while ((i <= q))
    {
      var u: dynamic;
      var v: dynamic;
      scanf("%d %d", (&u), (&v));
      que.emplace_back((u / MAGIC), v, u, i);
      i += 1;
    }
  }
  sort(que.begin(), que.end());
  l = cpp_assign(r, "=", 1);
  now = ((s[1] == k));
  add(1, 1);
  for (var it in que)
  {
    var u: dynamic;
    var v: dynamic;
    var id: dynamic;
    tie(ignore, v, u, id) = it;
    while ((r < v))
    {
      add((r + 1), 1);
      now += cnt[atm[(r + 1)]];
      if ((s[(l - 1)] == (s[(r + 1)] - k)))
      {
        now += 1;
      }
      if ((s[(r + 1)] == (s[(r + 1)] - k)))
      {
        now -= 1;
      }
      r += 1;
    }
    while ((u < l))
    {
      add((l - 1), 1);
      now += cnt[atp[(l - 2)]];
      l -= 1;
    }
    while ((v < r))
    {
      now -= cnt[atm[r]];
      if ((s[(l - 1)] == (s[r] - k)))
      {
        now -= 1;
      }
      if ((s[r] == (s[r] - k)))
      {
        now += 1;
      }
      add(r, -1);
      r -= 1;
    }
    while ((l < u))
    {
      now -= cnt[atp[(l - 1)]];
      add(l, -1);
      l += 1;
    }
    ans[id] = now;
  }
  {
    var i = 1;
    while ((i <= q))
    {
      printf("%lld\n", ans[i]);
      i += 1;
    }
  }
  return 0;
}
