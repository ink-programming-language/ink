// Translated from solution.cpp.

var N: dynamic = 200005;

class note
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
}

var opt: dynamic = cpp_array(N);

var t: dynamic = cpp_array(N);

var a: dynamic = cpp_array(N);

var n: dynamic = cpp_uninitialized();

var cnt: dynamic = cpp_uninitialized();

var vis: dynamic = cpp_array(N);

var ed: dynamic = cpp_array(N);

var s: dynamic = cpp_array(N);

var tot: dynamic = cpp_uninitialized();

var siz: dynamic = cpp_array(N);

func Swap(x: dynamic, y: dynamic) -> dynamic
{
  opt[cpp_update(tot, "++")] = [x, y];
  swap(a[x], a[y]);
}

func solve(x: dynamic) -> dynamic
{
  var nn: dynamic = 0;
  {
    var i: dynamic = a[x];
    while ((i != x))
    {
      t[cpp_update(nn, "++")] = i;
      i = a[i];
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= nn))
    {
      Swap(t[i], x);
      i += 1;
    }
  }
}

func merge(x: dynamic, y: dynamic) -> dynamic
{
  Swap(x, y);
  Swap(a[x], a[y]);
  solve(a[x]);
  solve(a[y]);
}

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((vis[i] || (a[i] == i)))
      {
        i += 1;
        continue;
      }
      s[cpp_update(cnt, "++")] = i;
      {
        var x: dynamic = i;
        while ((!vis[x]))
        {
          vis[x] = i;
          ed[cnt] = x;
          siz[cnt] += 1;
          x = a[x];
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < cnt))
    {
      merge(s[i], s[(i + 1)]);
      i += 2;
    }
  }
  if ((cnt & 1))
  {
    if ((siz[cnt] == 2))
    {
      var i: dynamic = 1;
      while ((vis[i] == cnt))
      {
        i += 1;
      }
      var v1: dynamic = s[cnt];
      var v2: dynamic = ed[cnt];
      Swap(v1, i);
      Swap(i, v2);
      Swap(v1, i);
    } else
    {
      var v1: dynamic = s[cnt];
      var v2: dynamic = a[s[cnt]];
      var vm: dynamic = ed[cnt];
      Swap(v1, v2);
      Swap(v2, vm);
      solve(v1);
    }
  }
  printf("%d\n", tot);
  {
    var i: dynamic = 1;
    while ((i <= tot))
    {
      printf("%d %d\n", opt[i].x, opt[i].y);
      i += 1;
    }
  }
}
