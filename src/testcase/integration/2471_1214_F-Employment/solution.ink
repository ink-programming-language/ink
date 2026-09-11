// Translated from solution.cpp.

var MAXN: dynamic = 200010;

var INF64: dynamic = (1 << 60);

var m: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(MAXN);

var b: dynamic = cpp_array(MAXN);

var v2: dynamic = cpp_array(MAXN);

var ans: dynamic = cpp_array(MAXN);

var d: dynamic = cpp_array(MAXN);

func update1(l: dynamic, r: dynamic, id: dynamic, val: dynamic) -> dynamic
{
  if ((l > r))
  {
    return;
  }
  var L: dynamic = ((((l + n) - id)) % n);
  var R: dynamic = ((((r + n) - id)) % n);
  if ((L > R))
  {
    d[L] += val;
    d[n] -= val;
    d[0] += val;
    d[(R + 1)] -= val;
  } else
  {
    d[L] += val;
    d[(R + 1)] -= val;
  }
}

func update2(l: dynamic, r: dynamic, id: dynamic, val: dynamic) -> dynamic
{
  if ((l > r))
  {
    return;
  }
  var L: dynamic = ((((id - r) + n)) % n);
  var R: dynamic = ((((id - l) + n)) % n);
  if ((L > R))
  {
    d[L] += val;
    d[n] -= val;
    d[0] += val;
    d[(R + 1)] -= val;
  } else
  {
    d[L] += val;
    d[(R + 1)] -= val;
  }
}

func main() -> dynamic
{
  scanf("%d%d", (&m), (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i].first));
      a[i].second = i;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&b[i].first));
      b[i].second = i;
      i += 1;
    }
  }
  sort((a + 1), ((a + n) + 1));
  sort((b + 1), ((b + n) + 1));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      v2[i] = (2 * b[i].first);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var p1: dynamic = (upper_bound((v2 + 1), ((v2 + n) + 1), ((2 * a[i].first) - m)) - v2);
      update1(1, (p1 - 1), i, (-a[i].first));
      var p2: dynamic = (upper_bound((v2 + 1), ((v2 + n) + 1), (2 * a[i].first)) - v2);
      update1(p1, (p2 - 1), i, a[i].first);
      var p3: dynamic = (upper_bound((v2 + 1), ((v2 + n) + 1), ((2 * a[i].first) + m)) - v2);
      update1(p2, (p3 - 1), i, (-a[i].first));
      update1(p3, n, i, (a[i].first + m));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      v2[i] = (2 * a[i].first);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var p1: dynamic = (upper_bound((v2 + 1), ((v2 + n) + 1), (((2 * b[i].first) - m) - 1)) - v2);
      update2(1, (p1 - 1), i, (-b[i].first));
      var p2: dynamic = (upper_bound((v2 + 1), ((v2 + n) + 1), ((2 * b[i].first) - 1)) - v2);
      update2(p1, (p2 - 1), i, b[i].first);
      var p3: dynamic = (upper_bound((v2 + 1), ((v2 + n) + 1), (((2 * b[i].first) + m) - 1)) - v2);
      update2(p2, (p3 - 1), i, (-b[i].first));
      update2(p3, n, i, (b[i].first + m));
      i += 1;
    }
  }
  var minv: dynamic = INF64;
  var id: dynamic = -1;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((d[i] < minv))
      {
        minv = d[i];
        id = i;
      }
      d[(i + 1)] += d[i];
      i += 1;
    }
  }
  write(minv, "\n");
  {
    var i: dynamic = 1;
    var j: dynamic = (id + 1);
    while ((i <= n))
    {
      ans[a[i].second] = b[j].second;
      i += 1;
      j = ( ((j == n)) ? 1 : (j + 1));
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      printf("%d%c", ans[i], " \n"[(i == n)]);
      i += 1;
    }
  }
}
