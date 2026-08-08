// Translated from solution.cpp.

func fun()
{
}

func gcd(a: dynamic, b: dynamic)
{
  if ((b == 0))
  {
    return a;
  }
  return gcd(b, (a % b));
}

func poww(a: dynamic, b: dynamic, md: dynamic)
{
  if ((b < 0))
  {
    return 0;
  }
  if ((a == 0))
  {
    return 0;
  }
  var res = 1;
  while (b)
  {
    if ((b & 1))
    {
      res = ((((1 * res) * a)) % md);
    }
    a = ((((1 * a) * a)) % md);
    b >>= 1;
  }
  return res;
}

func divide(a: dynamic, b: dynamic, md: dynamic)
{
  var rr = (a * (poww(b, (md - 2), md)));
  rr %= md;
  return rr;
}

var size = 55;

var n: dynamic;

var m: dynamic;

var degree = cpp_array(size);

var parent = cpp_array(size);

func findp(node: dynamic)
{
  if ((parent[node] == node))
  {
    return node;
  }
  return cpp_assign(parent[node], "=", findp(parent[node]));
}

func unite(x: dynamic, y: dynamic)
{
  x = findp(x);
  y = findp(y);
  parent[y] = x;
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  fun();
  read(n, m);
  if ((n == 1))
  {
    if ((m == 1))
    {
      write("YES\n0\n");
    } else if ((m > 1))
    {
      write("NO\n");
    } else
    {
      write("YES\n1\n1 1\n");
    }
    return 0;
  }
  var hasCycle = false;
  {
    var i = 1;
    while ((i <= n))
    {
      parent[i] = i;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      var u: dynamic;
      var v: dynamic;
      read(u, v);
      degree[u] += 1;
      degree[v] += 1;
      if ((findp(u) == findp(v)))
      {
        hasCycle = true;
      } else
      {
        unite(u, v);
      }
      i += 1;
    }
  }
  var maxdegree = 0;
  var mindegree = 1e9;
  {
    var i = 1;
    while ((i <= n))
    {
      maxdegree = max(maxdegree, degree[i]);
      mindegree = min(mindegree, degree[i]);
      i += 1;
    }
  }
  var components = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      if ((parent[i] == i))
      {
        components += 1;
      }
      i += 1;
    }
  }
  if (((((maxdegree == 2) && (mindegree == 2)) && (n == m)) && (components == 1)))
  {
    write("YES\n0\n");
    return 0;
  }
  if ((((maxdegree > 2) || hasCycle) || (m >= n)))
  {
    write("NO\n");
    return 0;
  }
  write("YES\n", (n - m), cpp_char("\n"));
  {
    var i = 1;
    while (((i <= n) && ((m - 1) < n)))
    {
      {
        var j = (i + 1);
        while (((j <= n) && ((m - 1) < n)))
        {
          if ((((findp(i) != findp(j)) && (degree[i] < 2)) && (degree[j] < 2)))
          {
            degree[i] += 1;
            degree[j] += 1;
            write(i, cpp_char(" "), j, cpp_char("\n"));
            m += 1;
            unite(i, j);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var node1 = -1;
  {
    var i = 1;
    while ((i <= n))
    {
      if (((degree[i] == 1) && (node1 != -1)))
      {
        write(node1, cpp_char(" "), i, cpp_char("\n"));
      } else if ((degree[i] == 1))
      {
        node1 = i;
      }
      i += 1;
    }
  }
  return 0;
}
