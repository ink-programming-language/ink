// Translated from solution.cpp.

var maxN = (2 * 100224);

class BIT
{
  var data: dynamic = cpp_array(maxN);
  func update(idx: dynamic, val: dynamic)
  {
      while ((idx < maxN))
      {
        data[idx] += val;
        idx += (idx & (-idx));
      }
    }
  func update(l: dynamic, r: dynamic, val: dynamic)
  {
      update(l, val);
      update((r + 1), (-val));
    }
  func query(idx: dynamic)
  {
      var res = 0;
      while ((idx > 0))
      {
        res += data[idx];
        idx -= (idx & (-idx));
      }
      return res;
    }
  func query(l: dynamic, r: dynamic)
  {
      return (query(r) - query(l));
    }
}

class LazyBIT
{
  var bitAdd: dynamic;
  var bitSub: dynamic;
  func update(l: dynamic, r: dynamic, val: dynamic)
  {
      bitAdd.update(l, r, val);
      bitSub.update(l, r, (((l - 1)) * val));
      bitSub.update((r + 1), (((((-r) + l) - 1)) * val));
    }
  func query(idx: dynamic)
  {
      return ((idx * bitAdd.query(idx)) - bitSub.query(idx));
    }
  func query(l: dynamic, r: dynamic)
  {
      return (query(r) - query((l - 1)));
    }
}

var parent = cpp_array(maxN);

var rnk = cpp_array(maxN);

var lfmost = cpp_array(maxN);

var rtmost = cpp_array(maxN);

var vis = cpp_array(maxN);

func make_set(v: dynamic)
{
  parent[v] = v;
  rnk[v] = 0;
  lfmost[v] = v;
  rtmost[v] = v;
}

func find_set(v: dynamic)
{
  if ((v == parent[v]))
  {
    return v;
  }
  return cpp_assign(parent[v], "=", find_set(parent[v]));
}

func union_sets(a: dynamic, b: dynamic)
{
  a = find_set(a);
  b = find_set(b);
  if ((a != b))
  {
    if ((rnk[a] < rnk[b]))
    {
      swap(a, b);
    }
    parent[b] = a;
    lfmost[a] = min(lfmost[a], lfmost[b]);
    rtmost[a] = max(rtmost[a], rtmost[b]);
    if ((rnk[a] == rnk[b]))
    {
      rnk[a] += 1;
    }
  }
}

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(null);
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(v[i]);
      i += 1;
    }
  }
  var sum = 0;
  var B: dynamic;
  {
    var i = 1;
    while ((i <= n))
    {
      B.update(i, i, sum);
      sum += i;
      i += 1;
    }
  }
  var haha: dynamic;
  {
    var i = (n - 1);
    while ((i >= 0))
    {
      var lo = 1;
      var hi = n;
      var mid: dynamic;
      var ans: dynamic;
      var val: dynamic;
      var temp: dynamic;
      while ((lo <= hi))
      {
        mid = (((lo + hi)) / 2);
        if ((vis[mid] == 1))
        {
          if (((lfmost[find_set(mid)] - 1) >= lo))
          {
            mid = (lfmost[find_set(mid)] - 1);
          } else if (((rtmost[find_set(mid)] + 1) <= hi))
          {
            mid = (rtmost[find_set(mid)] + 1);
          } else
          {
            break;
          }
        }
        val = B.query(mid, mid);
        if ((val == v[i]))
        {
          ans = mid;
          hi = (mid - 1);
        } else if ((v[i] > val))
        {
          lo = (mid + 1);
        } else if ((v[i] < val))
        {
          hi = (mid - 1);
        }
      }
      B.update(ans, n, (-ans));
      vis[ans] = 1;
      make_set(ans);
      if (((vis[ans] == 1) && (vis[(ans + 1)] == 1)))
      {
        union_sets(ans, (ans + 1));
      }
      if (((vis[ans] == 1) && (vis[(ans - 1)] == 1)))
      {
        union_sets(ans, (ans - 1));
      }
      haha.push_back(ans);
      i -= 1;
    }
  }
  reverse(haha.begin(), haha.end());
  for (var u in haha)
  {
    write(u, " ");
  }
}
