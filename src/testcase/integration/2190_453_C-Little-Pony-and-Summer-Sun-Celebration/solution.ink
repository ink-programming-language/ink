// Translated from solution.cpp.

var val = cpp_array(200010);

var cnt: dynamic;

var next = cpp_array(200010);

var head = cpp_array(200010);

var s = cpp_array(100010);

var ans = cpp_array(400010);

var out: dynamic;

var n: dynamic;

var m: dynamic;

var st: dynamic;

var vis = cpp_array(100010);

func add(a: dynamic, b: dynamic)
{
  val[cpp_update(cnt, "++")] = b;
  next[cnt] = head[a];
  head[a] = cnt;
}

func work(x: dynamic)
{
  s[x] ^= 1;
  ans[cpp_update(out, "++")] = x;
}

func dfs(u: dynamic)
{
  work(u);
  vis[u] = true;
  {
    var i = head[u];
    while (i)
    {
      var v = val[i];
      if (vis[v])
      {
        i = next[i];
        continue;
      }
      dfs(v);
      work(u);
      if (s[v])
      {
        work(v);
        work(u);
      }
      i = next[i];
    }
  }
}

func main()
{
  scanf("%d%d", (&n), (&m));
  {
    var i = 1;
    while ((i <= m))
    {
      var u: dynamic;
      var v: dynamic;
      scanf("%d%d", (&u), (&v));
      add(u, v);
      add(v, u);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&s[i]));
      if ((s[i] == 1))
      {
        st = i;
      }
      i += 1;
    }
  }
  dfs(st);
  if (s[st])
  {
    s[st] = 0;
    out -= 1;
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if ((s[i] != 0))
      {
        printf("-1\n");
        return 0;
      }
      i += 1;
    }
  }
  printf("%d\n", out);
  {
    var i = 1;
    while ((i <= out))
    {
      printf("%d ", ans[i]);
      i += 1;
    }
  }
  return 0;
}
