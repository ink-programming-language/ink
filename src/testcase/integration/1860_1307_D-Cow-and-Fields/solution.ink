// Translated from solution.cpp.

var dis_s = cpp_array(200005);

var dis_t = cpp_array(200005);

class Point
{
  var id: dynamic;
}

var b = cpp_array(200005);

var a = cpp_array(200005);

var E = cpp_array(200005);

func bfs(S: dynamic, dis: dynamic)
{
  {
    var i = 1;
    while ((i <= 200005))
    {
      dis[i] = (200005 + 1);
      i += 1;
    }
  }
  var Q: dynamic;
  Q.push(S);
  dis[S] = 0;
  while ((!Q.empty()))
  {
    var now = Q.front();
    Q.pop();
    for (var v in E[now])
    {
      if ((dis[v] > (dis[now] + 1)))
      {
        dis[v] = (dis[now] + 1);
        Q.push(v);
      }
    }
  }
}

var num = cpp_array(200005);

func cmp(A: dynamic, B: dynamic)
{
  return ((dis_t[A.id] - dis_s[A.id]) < (dis_t[B.id] - dis_s[B.id]));
}

var mxt = cpp_array(200005);

var mxs = cpp_array(200005);

func main()
{
  var n: dynamic;
  var m: dynamic;
  var k: dynamic;
  scanf("%d%d%d", (&n), (&m), (&k));
  {
    var i = 1;
    while ((i <= k))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      var x: dynamic;
      var y: dynamic;
      scanf("%d%d", (&x), (&y));
      E[x].push_back(y);
      E[y].push_back(x);
      i += 1;
    }
  }
  bfs(1, dis_s);
  bfs(n, dis_t);
  var ans = 0;
  {
    var i = 1;
    while ((i <= k))
    {
      b[i].id = a[i];
      i += 1;
    }
  }
  sort((b + 1), ((b + k) + 1), cmp);
  mxs[(k + 1)] = 0;
  mxt[0] = 0;
  {
    var i = k;
    while ((i >= 1))
    {
      mxs[i] = max(mxs[(i + 1)], dis_s[b[i].id]);
      i -= 1;
    }
  }
  {
    var i = 1;
    while ((i <= k))
    {
      mxt[i] = max(mxt[(i - 1)], dis_t[b[i].id]);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= k))
    {
      var tmp = 0;
      if ((i > 1))
      {
        tmp = max(tmp, ((dis_s[b[i].id] + mxt[(i - 1)]) + 1));
      }
      if ((i < k))
      {
        tmp = max(tmp, ((dis_t[b[i].id] + mxs[(i + 1)]) + 1));
      }
      ans = max(ans, tmp);
      i += 1;
    }
  }
  ans = min(ans, dis_s[n]);
  printf("%d\n", ans);
  return 0;
}
