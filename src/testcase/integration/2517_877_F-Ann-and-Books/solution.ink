// Translated from solution.cpp.

var N: dynamic = (3e5 + 5);

var MAGIC: dynamic = 320;

var a: dynamic = cpp_array(N);

var p: dynamic = cpp_array(N);

var s: dynamic = cpp_array(N);

var k: dynamic = cpp_uninitialized();

var now: dynamic = cpp_uninitialized();

var l: dynamic = cpp_uninitialized();

var r: dynamic = cpp_uninitialized();

var all: dynamic = cpp_uninitialized();

var at: dynamic = cpp_array(N);

var atp: dynamic = cpp_array(N);

var atm: dynamic = cpp_array(N);

var cnt: dynamic = cpp_array(N);

func add(i: dynamic, v: dynamic) -> dynamic
{
  cnt[at[i]] += v;
}

var ans: dynamic = cpp_array(N);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%d %lld", (&n), (&k));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (p + i));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (a + i));
      s[i] = (s[(i - 1)] + ( ((p[i] == 1)) ? a[i] : (-a[i])));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i <= n))
    {
      at[i] = (lower_bound(all.begin(), all.end(), s[i]) - all.begin());
      atp[i] = (lower_bound(all.begin(), all.end(), (s[i] + k)) - all.begin());
      atm[i] = (lower_bound(all.begin(), all.end(), (s[i] - k)) - all.begin());
      i += 1;
    }
  }
  var que: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  scanf("%d", (&q));
  {
    var i: dynamic = 1;
    while ((i <= q))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      scanf("%d %d", (&u), (&v));
      que.emplace_back((u / MAGIC), v, u, i);
      i += 1;
    }
  }
  sort(que.begin(), que.end());
  l = cpp_assign(r, "=", 1);
  now = ((s[1] == k));
  add(1, 1);
  for (var it: dynamic in que)
  {
    var u: dynamic = cpp_uninitialized();
    var v: dynamic = cpp_uninitialized();
    var id: dynamic = cpp_uninitialized();
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
    var i: dynamic = 1;
    while ((i <= q))
    {
      printf("%lld\n", ans[i]);
      i += 1;
    }
  }
  return 0;
}
