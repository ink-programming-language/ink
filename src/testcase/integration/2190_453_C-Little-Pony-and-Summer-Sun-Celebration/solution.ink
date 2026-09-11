// Translated from solution.cpp.

var val: dynamic = cpp_array(200010);

var cnt: dynamic = cpp_uninitialized();

var next: dynamic = cpp_array(200010);

var head: dynamic = cpp_array(200010);

var s: dynamic = cpp_array(100010);

var ans: dynamic = cpp_array(400010);

var out: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var st: dynamic = cpp_uninitialized();

var vis: dynamic = cpp_array(100010);

func add(a: dynamic, b: dynamic) -> dynamic
{
  val[cpp_update(cnt, "++")] = b;
  next[cnt] = head[a];
  head[a] = cnt;
}

func work(x: dynamic) -> dynamic
{
  s[x] ^= 1;
  ans[cpp_update(out, "++")] = x;
}

func dfs(u: dynamic) -> dynamic
{
  work(u);
  vis[u] = true;
  {
    var i: dynamic = head[u];
    while (i)
    {
      var v: dynamic = val[i];
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

func main() -> dynamic
{
  scanf("%d%d", (&n), (&m));
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      scanf("%d%d", (&u), (&v));
      add(u, v);
      add(v, u);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
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
    var i: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i <= out))
    {
      printf("%d ", ans[i]);
      i += 1;
    }
  }
  return 0;
}
