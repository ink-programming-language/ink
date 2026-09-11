// Translated from solution.cpp.

func Max(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    a = b;
  }
}

func Min(a: dynamic, b: dynamic) -> dynamic
{
  if ((a > b))
  {
    a = b;
  }
}

func add(a: dynamic, b: dynamic) -> dynamic
{
  a += b;
  if ((a >= 1000000007))
  {
    a -= 1000000007;
  }
}

func pow(a: dynamic, b: dynamic) -> dynamic
{
  var ans: dynamic = 1;
  while (b)
  {
    if ((b & 1))
    {
      ans = ((ans * cpp_cast(a)) % 1000000007);
    }
    a = ((cpp_cast(a) * a) % 1000000007);
    b >>= 1;
  }
  return ans;
}

var ans: dynamic = cpp_array(300010);

var vis: dynamic = cpp_array(300010);

var a: dynamic = cpp_array(300010);

class MAX_tree
{
  var t: dynamic = cpp_array((300010 << 2));
  func build(p: dynamic, l: dynamic, r: dynamic) -> dynamic
  {
      t[p] = -1;
      if ((l < r))
      {
        var m: dynamic = (((l + r)) >> 1);
        build((p << 1), l, m);
        build(((p << 1) | 1), (m + 1), r);
      }
    }
  func build(p: dynamic, l: dynamic, r: dynamic, a: dynamic) -> dynamic
  {
      if ((l == r))
      {
        t[p] = a[l];
      }
      if ((l < r))
      {
        var m: dynamic = (((l + r)) >> 1);
        build((p << 1), l, m, a);
        build(((p << 1) | 1), (m + 1), r, a);
        t[p] = max(t[(p << 1)], t[((p << 1) | 1)]);
      }
    }
  func upd(p: dynamic, l: dynamic, r: dynamic, first: dynamic, v: dynamic) -> dynamic
  {
      if ((l == r))
      {
        t[p] = v;
        return;
      }
      var m: dynamic = (((l + r)) >> 1);
      if ((first <= m))
      {
        upd((p << 1), l, m, first, v);
      } else
      {
        upd(((p << 1) | 1), (m + 1), r, first, v);
      }
      t[p] = max(t[(p << 1)], t[((p << 1) | 1)]);
    }
  func query(p: dynamic, l: dynamic, r: dynamic, first: dynamic, second: dynamic) -> dynamic
  {
      if (((l >= first) && (r <= second)))
      {
        return t[p];
      }
      var m: dynamic = (((l + r)) >> 1);
      var ans: dynamic = -1;
      if ((first <= m))
      {
        ans = query((p << 1), l, m, first, second);
      }
      if ((second > m))
      {
        Max(ans, query(((p << 1) | 1), (m + 1), r, first, second));
      }
      return ans;
    }
}

var t1: dynamic = cpp_uninitialized();

class MIN_tree
{
  var t: dynamic = cpp_array((300010 << 2));
  func build(p: dynamic, l: dynamic, r: dynamic) -> dynamic
  {
      t[p] = 1000000007;
      if ((l < r))
      {
        var m: dynamic = (((l + r)) >> 1);
        build((p << 1), l, m);
        build(((p << 1) | 1), (m + 1), r);
      }
    }
  func upd(p: dynamic, l: dynamic, r: dynamic, first: dynamic, second: dynamic, v: dynamic) -> dynamic
  {
      if (((l >= first) && (r <= second)))
      {
        Min(t[p], v);
        return;
      }
      var m: dynamic = (((l + r)) >> 1);
      if ((first <= m))
      {
        upd((p << 1), l, m, first, second, v);
      }
      if ((second > m))
      {
        upd(((p << 1) | 1), (m + 1), r, first, second, v);
      }
    }
  func query(p: dynamic, l: dynamic, r: dynamic, first: dynamic) -> dynamic
  {
      if ((l == r))
      {
        return t[p];
      }
      var m: dynamic = (((l + r)) >> 1);
      var ans: dynamic = 1000000007;
      if ((first <= m))
      {
        ans = min(t[p], query((p << 1), l, m, first));
      } else
      {
        ans = min(t[p], query(((p << 1) | 1), (m + 1), r, first));
      }
      return ans;
    }
}

var t2: dynamic = cpp_uninitialized();

class Q
{
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var first: dynamic = cpp_uninitialized();
}

var p: dynamic = cpp_array(300010);

func main() -> dynamic
{
  var T: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var ca: dynamic = 0;
  var m: dynamic = cpp_uninitialized();
  var K: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  scanf("%d%d", (&n), (&m));
  t1.build(1, 1, n);
  t2.build(1, 1, n);
  var ok: dynamic = 1;
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      scanf("%d", (&K));
      var l: dynamic = cpp_uninitialized();
      var r: dynamic = cpp_uninitialized();
      var first: dynamic = cpp_uninitialized();
      if ((K == 1))
      {
        scanf("%d%d%d", (&l), (&r), (&first));
        if ((!ok))
        {
          i += 1;
          continue;
        }
        K = t1.query(1, 1, n, l, r);
        if ((K > first))
        {
          ok = 0;
          i += 1;
          continue;
        }
        t2.upd(1, 1, n, l, r, first);
        p[i] = [l, r, first];
      } else
      {
        scanf("%d%d", (&k), (&first));
        if ((!ok))
        {
          i += 1;
          continue;
        }
        if ((!vis[k]))
        {
          vis[k] = 1;
          ans[k] = t2.query(1, 1, n, k);
        }
        t1.upd(1, 1, n, k, first);
        p[i] = [k, first, -1];
      }
      i += 1;
    }
  }
  if ((!ok))
  {
    puts("NO");
    return 0;
  }
  var ss: dynamic = 0;
  K = 0;
  var bit: dynamic = [0];
  {
    var i: dynamic = 1;
    while ((i < (n + 1)))
    {
      if ((!vis[i]))
      {
        ans[i] = t2.query(1, 1, n, i);
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < (n + 1)))
    {
      if ((ans[i] < 1000000007))
      {
        ss |= ans[i];
      }
      i += 1;
    }
  }
  var q: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
    while ((i < (n + 1)))
    {
      j = i;
      while (((j <= n) && (ans[j] == ans[i])))
      {
        j += 1;
      }
      if ((ans[i] == 1000000007))
      {
        {
          var k: dynamic = i;
          while ((k < j))
          {
            q.push_back([1e9, k]);
            k += 1;
          }
        }
      } else
      {
        {
          var k: dynamic = (i + 1);
          while ((k < j))
          {
            q.push_back([ans[i], k]);
            k += 1;
          }
        }
      }
      i = (j - 1);
      i += 1;
    }
  }
  {
    var i: dynamic = (30 - 1);
    while ((i >= 0))
    {
      if ((!(((ss >> i) & 1))))
      {
        bit[i] = 1;
      }
      i -= 1;
    }
  }
  for (var o: dynamic in q)
  {
    var first: dynamic = o.second;
    var second: dynamic = o.first;
    var ss: dynamic = 0;
    {
      var j: dynamic = (30 - 1);
      while ((j >= 0))
      {
        if ((bit[j] && ((ss + ((1 << j))) <= second)))
        {
          bit[j] = 0;
          ss += (1 << j);
        }
        j -= 1;
      }
    }
    ans[first] = ss;
  }
  {
    var i: dynamic = 1;
    while ((i < (n + 1)))
    {
      a[i] = ans[i];
      i += 1;
    }
  }
  t1.build(1, 1, n, a);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      if ((p[i].first == -1))
      {
        t1.upd(1, 1, n, p[i].l, p[i].r);
      } else
      {
        K = t1.query(1, 1, n, p[i].l, p[i].r);
        if ((K != p[i].first))
        {
          return (0 * puts("NO"));
        }
      }
      i += 1;
    }
  }
  puts("YES");
  ss = 0;
  {
    var i: dynamic = 1;
    while ((i < (n + 1)))
    {
      printf("%d ", ans[i]);
      ss |= ans[i];
      i += 1;
    }
  }
  puts("");
  return 0;
}
