// Translated from solution.cpp.

var N = 200005;

var tree = cpp_array((2 * N));

var tree1 = cpp_array((2 * N));

var e = cpp_construct(N, 0);

var cos1 = cpp_construct(N, 0);

var m: dynamic;

var plan: dynamic;

func updateTreeNode(p: dynamic, value: dynamic)
{
  var n = m;
  tree[(p + n)] = value;
  p = (p + n);
  {
    var i = p;
    while ((i > 1))
    {
      tree[(i >> 1)] = (tree[i] + tree[(i ^ 1)]);
      i >>= 1;
    }
  }
}

func query(l: dynamic, r: dynamic)
{
  var n = m;
  var res = 0;
  {
    l += n;
    r += n;
    while ((l < r))
    {
      if ((l & 1))
      {
        res += tree[cpp_update(l, "++")];
      }
      if ((r & 1))
      {
        res += tree[cpp_update(r, "--")];
      }
      l >>= 1;
      r >>= 1;
    }
  }
  return res;
}

func updateTreeNode1(p: dynamic, value: dynamic)
{
  var n = m;
  tree1[(p + n)] = value;
  p = (p + n);
  {
    var i = p;
    while ((i > 1))
    {
      tree1[(i >> 1)] = (tree1[i] + tree1[(i ^ 1)]);
      i >>= 1;
    }
  }
}

func query1(l: dynamic, r: dynamic)
{
  var n = m;
  var res = 0;
  {
    l += n;
    r += n;
    while ((l < r))
    {
      if ((l & 1))
      {
        res += tree1[cpp_update(l, "++")];
      }
      if ((r & 1))
      {
        res += tree1[cpp_update(r, "--")];
      }
      l >>= 1;
      r >>= 1;
    }
  }
  return res;
}

func solve()
{
  var n: dynamic;
  var k: dynamic;
  read(n, k, m);
  {
    var i = 0;
    while ((i < (2 * N)))
    {
      tree[i] = 0;
      tree1[i] = 0;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      var l: dynamic;
      var r: dynamic;
      var c: dynamic;
      var p: dynamic;
      read(l, r, c, p);
      plan.push_back(make_pair(make_pair(p, l), make_pair(r, c)));
      i += 1;
    }
  }
  sort(plan.begin(), plan.end());
  var d = cpp_construct((1000000 + 5));
  {
    var i = 0;
    while ((i < m))
    {
      d[(plan[i].first.second - 1)].push_back(i);
      d[plan[i].second.first].push_back(((-i) - 1));
      i += 1;
    }
  }
  var tot = 0;
  {
    var i = 0;
    while ((i < n))
    {
      for (var j in d[i])
      {
        if ((j >= 0))
        {
          e[j] += cpp_cast(plan[j].second.second);
          cos1[j] += (cpp_cast(plan[j].second.second) * cpp_cast(plan[j].first.first));
        } else
        {
          j *= -1;
          j -= 1;
          e[j] -= cpp_cast(plan[j].second.second);
          cos1[j] -= (cpp_cast(plan[j].second.second) * cpp_cast(plan[j].first.first));
        }
        updateTreeNode(j, e[j]);
        updateTreeNode1(j, cos1[j]);
      }
      if ((query(0, m) <= k))
      {
        tot += query1(0, m);
        i += 1;
        continue;
      }
      if ((query(0, 1) >= k))
      {
        tot += (cpp_cast(plan[0].first.first) * k);
        i += 1;
        continue;
      }
      var st = 0;
      var end = (m - 1);
      var w = (m - 2);
      while ((st <= end))
      {
        var mid = (((st + end)) / 2);
        if ((query(0, (mid + 1)) <= k))
        {
          w = mid;
          st = (mid + 1);
        } else
        {
          end = (mid - 1);
        }
      }
      tot += query1(0, (w + 1));
      var left = (k - query(0, (w + 1)));
      tot += (cpp_cast(plan[(w + 1)].first.first) * left);
      i += 1;
    }
  }
  write(tot, "\n");
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  srand(time(null));
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var t = 1;
  var c = 0;
  while (cpp_update(t, "--"))
  {
    solve();
    c += 1;
  }
  return 0;
}
