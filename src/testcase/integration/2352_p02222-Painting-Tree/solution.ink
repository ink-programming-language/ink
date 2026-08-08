// Translated from solution.cpp.

func eprintf()
{
  return cpp_expression("#include <iostream> #include");
}

func eprintf()
{
  return cpp_expression("#i");
}

func rep(i: dynamic, a: dynamic, b: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for (int i = (a), i##_len = (b); i < i##_len; ++i)");
}

func rep(i: dynamic)
{
  return cpp_expression("#include <iostream> #include <cstdio> #include <c");
}

func reprev(i: dynamic, a: dynamic, b: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for (int i = (b-1), i##_min = (a); i >= i##_min; --i)");
}

func reprev(i: dynamic)
{
  return cpp_expression("#include <iostream> #include <cstdio> #include <cstd");
}

func all(x: dynamic)
{
  return cpp_expression("#include <iostream> #i");
}

func chmax(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    a = b;
    return 1;
  }
  return 0;
}

func chmin(a: dynamic, b: dynamic)
{
  if ((b < a))
  {
    a = b;
    return 1;
  }
  return 0;
}

func gcd(a: dynamic, b: dynamic)
{
  return if (b) gcd(b, (a % b)) else a;
}

func main(argument_0: dynamic)
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  var n: dynamic;
  read(n);
  rep(i, (n - 1));
  {
    var a: dynamic;
    var b: dynamic;
    read(a, b);
    a -= 1;
    b -= 1;
    graph[a].push_back(b);
    graph[b].push_back(a);
    hen[a] += 1;
    hen[b] += 1;
  }
  used_tmp[0] = true;
  var q: dynamic;
  q.emplace(0, 0);
  while ((!q.empty()))
  {
    var p = q.top();
    q.pop();
    dist_tmp[p.second] = p.first;
    for (var i in graph[p.second])
    {
      if ((!used_tmp[i]))
      {
        used_tmp[i] = true;
        q.emplace((p.first + 1), i);
      }
    }
  }
  rep(i, n);
  eprintf("%d ", dist_tmp[i]);
  eprintf("\n");
  var u = 0;
  var u_len = 0;
  rep(i, n);
  if (chmax(u_len, dist_tmp[i]))
  {
    u = i;
  }
  used_u[u] = true;
  q.emplace(0, u);
  while ((!q.empty()))
  {
    var p = q.top();
    q.pop();
    dist_u[p.second] = p.first;
    for (var i in graph[p.second])
    {
      if ((!used_u[i]))
      {
        used_u[i] = true;
        q.emplace((p.first + 1), i);
      }
    }
  }
  rep(i, n);
  eprintf("%d ", dist_u[i]);
  eprintf("\n");
  var v = u;
  var longest = 0;
  rep(i, n);
  if (chmax(longest, dist_u[i]))
  {
    v = i;
  }
  eprintf("%d %d %d\n", u, v, longest);
  used_v[v] = true;
  q.emplace(0, v);
  while ((!q.empty()))
  {
    var p = q.top();
    q.pop();
    dist_v[p.second] = p.first;
    for (var i in graph[p.second])
    {
      if ((!used_v[i]))
      {
        used_v[i] = true;
        q.emplace((p.first + 1), i);
      }
    }
  }
  rep(i, n);
  eprintf("%d ", dist_v[i]);
  eprintf("\n");
  var m = 0;
  var flg = true;
  rep(k, 1, (n + 1));
  {
    var ok = true;
    if (((k <= 2) || (k > longest)))
    {
      write(1);
      continue;
    }
    if (((m < k) || (((m == k) && flg))))
    {
      write(1);
    } else
    {
      write(0);
    }
  }
  write("\n");
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
    if ((((hen[i] != 1) || (i == u)) || (i == v)))
    {
      continue;
    }
    chmax(m, max(dist_u[i], dist_v[i]));
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    if ((((m == max(dist_u[i], dist_v[i])) && (((m != dist_u[i]) || (m != dist_v[i]))))))
    {
      if ((((hen[i] != 1) || (i == u)) || (i == v)))
      {
        continue;
      }
      flg = false;
    }
  }
