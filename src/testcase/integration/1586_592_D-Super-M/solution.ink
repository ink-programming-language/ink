// Translated from solution.cpp.

var maxn = 124000;

var tag = cpp_array(maxn);

var v1 = cpp_array(maxn);

var v2 = cpp_array(maxn);

var a = cpp_array(maxn);

var check = cpp_array(maxn);

var d: dynamic;

func Go(x: dynamic, fa: dynamic)
{
  {
    var i = 0;
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

func print(x: dynamic, fa: dynamic)
{
  printf("%d ", x);
  {
    var i = 0;
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
  var x: dynamic;
  var t: dynamic;
  func point(x: dynamic = 0, t: dynamic = 0)
  {
      this->x = cpp_construct(x);
      this->t = cpp_construct(t);
    }
}

var q: dynamic;

func bfs(x: dynamic)
{
  while ((!q.empty()))
  {
    q.pop();
  }
  memset(check, 0, cpp_sizeof((check)));
  q.push(point(x, 0));
  check[x] = 1;
  var p: dynamic;
  var t: dynamic;
  while ((!q.empty()))
  {
    p = q.top();
    q.pop();
    x = p.x;
    t = p.t;
    {
      var i = 0;
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

func getson(x: dynamic, fa: dynamic)
{
  var ans = 0;
  {
    var i = 0;
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

func main()
{
  var n: dynamic;
  var m: dynamic;
  var l: dynamic;
  var r: dynamic;
  var ansnum: dynamic;
  while ((~scanf("%d%d", (&n), (&m))))
  {
    memset(tag, 0, cpp_sizeof((tag)));
    {
      var i = 1;
      while ((i <= n))
      {
        v1[i].clear();
        v2[i].clear();
        i += 1;
      }
    }
    {
      var i = 1;
      while ((i < n))
      {
        scanf("%d%d", (&l), (&r));
        v1[l].push_back(r);
        v1[r].push_back(l);
        i += 1;
      }
    }
    {
      var i = 1;
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
