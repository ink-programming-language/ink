// Translated from solution.cpp.

var linf: dynamic = (1e18 + 5);

var mod: dynamic = (cpp_cast(1e9) + 7);

var logN: dynamic = 18;

var inf: dynamic = (1e9 + 9);

var N: dynamic = (3e5 + 5);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var z: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var c: dynamic = cpp_array(N);

var p: dynamic = cpp_uninitialized();

var sum: dynamic = cpp_array(N);

var h: dynamic = cpp_array(N);

var G: dynamic = cpp_array(N);

var v: dynamic = cpp_array(N);

var ans: dynamic = cpp_uninitialized();

func prep(node: dynamic, root: dynamic) -> dynamic
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

func find(node: dynamic, root: dynamic, S: dynamic) -> dynamic
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

func dfs(node: dynamic, root: dynamic, dist: dynamic) -> dynamic
{
  var ans: dynamic = ((dist * sqrt(dist)) * c[node]);
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

func dfs2(node: dynamic, root: dynamic, dist: dynamic) -> dynamic
{
  var ans: dynamic = (sqrt(dist) * c[node]);
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

func find(node: dynamic) -> dynamic
{
  prep(node, 0);
  node = find(node, 0, (sum[node] / 2));
  h[node] = 1;
  var S: dynamic = 0;
  var all: dynamic = 0;
  ans.push_back(make_pair(dfs(node, 0, 0), node));
  var d2: dynamic = 0;
  var temp: dynamic = cpp_uninitialized();
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

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&c[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 2;
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
