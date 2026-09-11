// Translated from solution.cpp.

var maxn: dynamic = 124000;

var tag: dynamic = cpp_array(maxn);

var v1: dynamic = cpp_array(maxn);

var v2: dynamic = cpp_array(maxn);

var a: dynamic = cpp_array(maxn);

var check: dynamic = cpp_array(maxn);

var d: dynamic = cpp_uninitialized();

func Go(x: dynamic, fa: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < v1[x].size()))
    {
      if ((fa == v1[x][i]))
      {
        i += 1;
        continue;
      }
      if (Go(v1[x][i], x))
      {
        v2[x].push_back(v1[x][i]);
        v2[v1[x][i]].push_back(x);
      }
      i += 1;
    }
  }
  if ((tag[x] || v2[x].size()))
  {
    return true;
  }
  return false;
}

func print(x: dynamic, fa: dynamic) -> dynamic
{
  printf("%d ", x);
  {
    var i: dynamic = 0;
    while ((i < v2[x].size()))
    {
      if ((v2[x][i] == fa))
      {
        i += 1;
        continue;
      }
      print(v2[x][i], x);
      i += 1;
    }
  }
  return;
}

class point
{
  var x: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  func point(x: dynamic = 0, t: dynamic = 0) -> dynamic
  {
      self->x = cpp_construct(x);
      self->t = cpp_construct(t);
    }
}

var q: dynamic = cpp_uninitialized();

func bfs(x: dynamic) -> dynamic
{
  while ((!q.empty()))
  {
    q.pop();
  }
  memset(check, 0, cpp_sizeof((check)));
  q.push(point(x, 0));
  check[x] = 1;
  var p: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  while ((!q.empty()))
  {
    p = q.top();
    q.pop();
    x = p.x;
    t = p.t;
    {
      var i: dynamic = 0;
      while ((i < v2[x].size()))
      {
        if ((!check[v2[x][i]]))
        {
          check[v2[x][i]] = 1;
          q.push(point(v2[x][i], (t + 1)));
        }
        i += 1;
      }
    }
  }
  d = t;
  return x;
}

func getson(x: dynamic, fa: dynamic) -> dynamic
{
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < v2[x].size()))
    {
      if ((v2[x][i] == fa))
      {
        i += 1;
        continue;
      }
      ans += getson(v2[x][i], x);
      i += 1;
    }
  }
  return (ans + 1);
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var ansnum: dynamic = cpp_uninitialized();
  while ((~scanf("%d%d", (&n), (&m))))
  {
    memset(tag, 0, cpp_sizeof((tag)));
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        v1[i].clear();
        v2[i].clear();
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i < n))
      {
        scanf("%d%d", (&l), (&r));
        v1[l].push_back(r);
        v1[r].push_back(l);
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i <= m))
      {
        scanf("%d", (&a[i]));
        tag[a[i]] = 1;
        i += 1;
      }
    }
    sort((a + 1), ((a + m) + 1));
    Go(a[1], -1);
    l = bfs(a[1]);
    r = bfs(l);
    ansnum = ((((getson(a[1], -1) - 1)) * 2) - d);
    printf("%d\n%d\n", min(l, r), ansnum);
  }
  return 0;
}
