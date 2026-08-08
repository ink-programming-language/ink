// Translated from solution.cpp.

var N = 2e5;

func solve()
{
  var n: dynamic;
  var p: dynamic;
  read(n, p);
  var cnt: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      var a: dynamic;
      var b: dynamic;
      read(a, b);
      a -= 1;
      b -= 1;
      cnt[make_pair(a, b)] += 1;
      cnt[make_pair(b, a)] += 1;
      e[i] = make_pair(a, b);
      g[a].push_back(b);
      g[b].push_back(a);
      i += 1;
    }
  }
  var res = 0;
  {
    var i = 0;
    while ((i < n))
    {
      r[i] = g[i].size();
      i += 1;
    }
  }
  sort(r.begin(), r.end());
  {
    var i = 0;
    while ((i < n))
    {
      var id = (lower_bound(r.begin(), r.end(), (p - r[i])) - r.begin());
      if ((id > i))
      {
        res += (n - id);
      } else
      {
        res += ((n - id) - 1);
      }
      i += 1;
    }
  }
  res /= 2;
  var b: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      if ((((((g[e[i].first].size() + g[e[i].second].size()) >= p) && (((g[e[i].first].size() + g[e[i].second].size()) - cnt[e[i]]) < p)) && (!b.count(e[i]))) && (!b.count(make_pair(e[i].second, e[i].first)))))
      {
        res -= 1;
        b.insert(e[i]);
        b.insert(make_pair(e[i].second, e[i].first));
      }
      i += 1;
    }
  }
  write(res);
}

func main()
{
  ios_base.sync_with_stdio(null);
  cin.tie(0);
  cout.tie(0);
  var t = 1;
  while (cpp_update(t, "--"))
  {
    solve();
  }
}
