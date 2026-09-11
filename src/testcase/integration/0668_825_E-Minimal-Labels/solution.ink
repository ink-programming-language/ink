// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var D: dynamic = cpp_array(100010);

var vis: dynamic = cpp_array(100010);

var E: dynamic = cpp_array(100010);

var Q: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(n, m);
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      read(a, b);
      D[a] += 1;
      E[b].push_back(a);
      i += 1;
    }
  }
  {
    var i: dynamic = n;
    while ((i >= 1))
    {
      if ((D[i] == 0))
      {
        Q.push(i);
      }
      i -= 1;
    }
  }
  var now: dynamic = n;
  while ((!Q.empty()))
  {
    var nd: dynamic = Q.top();
    Q.pop();
    if (vis[nd])
    {
      continue;
    }
    vis[nd] = cpp_update(now, "--");
    for (var i: dynamic in E[nd])
    {
      D[i] -= 1;
      if (((!D[i]) && (!vis[i])))
      {
        Q.push(i);
      }
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      write(vis[i], " ");
      i += 1;
    }
  }
}
