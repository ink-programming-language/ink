// Translated from solution.cpp.

var dis_s: dynamic = cpp_array(200005);

var dis_t: dynamic = cpp_array(200005);

class Point
{
  var id: dynamic = cpp_uninitialized();
}

var b: dynamic = cpp_array(200005);

var a: dynamic = cpp_array(200005);

var E: dynamic = cpp_array(200005);

func bfs(S: dynamic, dis: dynamic) -> dynamic
{
  {
    var i: dynamic = 1;
    while ((i <= 200005))
    {
      dis[i] = (200005 + 1);
      i += 1;
    }
  }
  var Q: dynamic = cpp_uninitialized();
  Q.push(S);
  dis[S] = 0;
  while ((!Q.empty()))
  {
    var now: dynamic = Q.front();
    Q.pop();
    for (var v: dynamic in E[now])
    {
      if ((dis[v] > (dis[now] + 1)))
      {
        dis[v] = (dis[now] + 1);
        Q.push(v);
      }
    }
  }
}

var num: dynamic = cpp_array(200005);

func cmp(A: dynamic, B: dynamic) -> dynamic
{
  return ((dis_t[A.id] - dis_s[A.id]) < (dis_t[B.id] - dis_s[B.id]));
}

var mxt: dynamic = cpp_array(200005);

var mxs: dynamic = cpp_array(200005);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  scanf("%d%d%d", (&n), (&m), (&k));
  {
    var i: dynamic = 1;
    while ((i <= k))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      scanf("%d%d", (&x), (&y));
      E[x].push_back(y);
      E[y].push_back(x);
      i += 1;
    }
  }
  bfs(1, dis_s);
  bfs(n, dis_t);
  var ans: dynamic = 0;
  {
    var i: dynamic = 1;
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
    var i: dynamic = k;
    while ((i >= 1))
    {
      mxs[i] = max(mxs[(i + 1)], dis_s[b[i].id]);
      i -= 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= k))
    {
      mxt[i] = max(mxt[(i - 1)], dis_t[b[i].id]);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= k))
    {
      var tmp: dynamic = 0;
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
