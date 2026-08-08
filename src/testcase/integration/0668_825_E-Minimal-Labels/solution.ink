// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var a: dynamic;

var b: dynamic;

var D = cpp_array(100010);

var vis = cpp_array(100010);

var E = cpp_array(100010);

var Q: dynamic;

func main()
{
  read(n, m);
  {
    var i = 1;
    while ((i <= m))
    {
      read(a, b);
      D[a] += 1;
      E[b].push_back(a);
      i += 1;
    }
  }
  {
    var i = n;
    while ((i >= 1))
    {
      if ((D[i] == 0))
      {
        Q.push(i);
      }
      i -= 1;
    }
  }
  var now = n;
  while ((!Q.empty()))
  {
    var nd = Q.top();
    Q.pop();
    if (vis[nd])
    {
      continue;
    }
    vis[nd] = cpp_update(now, "--");
    for (var i in E[nd])
    {
      D[i] -= 1;
      if (((!D[i]) && (!vis[i])))
      {
        Q.push(i);
      }
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      write(vis[i], " ");
      i += 1;
    }
  }
}
