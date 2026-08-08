// Translated from solution.cpp.

var linf = (1e18 + 5);

var mod = (cpp_cast(1e9) + 7);

var logN = 18;

var inf = (1e9 + 9);

var N = (3e5 + 5);

var n: dynamic;

var m: dynamic;

var x: dynamic;

var y: dynamic;

var z: dynamic;

var t: dynamic;

var c = cpp_array(N);

var p: dynamic;

var sum = cpp_array(N);

var h = cpp_array(N);

var G = cpp_array(N);

var v = cpp_array(N);

var ans: dynamic;

func prep(node: dynamic, root: dynamic)
{
  sum[node] = 1;
  {
    typeof(v[node].begin()) = v[node].begin();
    while ((it != v[node].end()))
    {
      if (((!h[it->first]) && (it->first != root)))
      {
        sum[node] += prep(it->first, node);
      }
      it += 1;
    }
  }
  return sum[node];
}

func find(node: dynamic, root: dynamic, S: dynamic)
{
  {
    typeof(v[node].begin()) = v[node].begin();
    while ((it != v[node].end()))
    {
      if ((((it->first != root) && (!h[it->first])) && (sum[it->first] > S)))
      {
        return find(it->first, node, S);
      }
      it += 1;
    }
  }
  return node;
}

func dfs(node: dynamic, root: dynamic, dist: dynamic)
{
  var ans = ((dist * sqrt(dist)) * c[node]);
  {
    typeof(v[node].begin()) = v[node].begin();
    while ((it != v[node].end()))
    {
      if ((it->first != root))
      {
        ans += dfs(it->first, node, (dist + it->second));
      }
      it += 1;
    }
  }
  return ans;
}

func dfs2(node: dynamic, root: dynamic, dist: dynamic)
{
  var ans = (sqrt(dist) * c[node]);
  {
    typeof(v[node].begin()) = v[node].begin();
    while ((it != v[node].end()))
    {
      if ((it->first != root))
      {
        ans += dfs2(it->first, node, (dist + it->second));
      }
      it += 1;
    }
  }
  return ans;
}

func find(node: dynamic)
{
  prep(node, 0);
  node = find(node, 0, (sum[node] / 2));
  h[node] = 1;
  var S = 0;
  var all = 0;
  ans.push_back(make_pair(dfs(node, 0, 0), node));
  var d2 = 0;
  var temp: dynamic;
  {
    typeof(v[node].begin()) = v[node].begin();
    while ((it != v[node].end()))
    {
      if ((!h[it->first]))
      {
        d2 += dfs2(it->first, node, it->second);
      }
      it += 1;
    }
  }
  {
    typeof(v[node].begin()) = v[node].begin();
    while ((it != v[node].end()))
    {
      if (((!h[it->first]) && ((d2 - (2 * dfs2(it->first, node, it->second))) < 0)))
      {
        find(it->first);
        return;
      }
      it += 1;
    }
  }
}

func main()
{
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&c[i]));
      i += 1;
    }
  }
  {
    var i = 2;
    while ((i <= n))
    {
      scanf("%d %d %d", (&x), (&y), (&z));
      v[x].push_back(make_pair(y, z));
      v[y].push_back(make_pair(x, z));
      i += 1;
    }
  }
  find(1);
  sort(ans.begin(), ans.end());
  printf("%d %.12lf\n", ans.begin()->second, ans.begin()->first);
  return 0;
}
