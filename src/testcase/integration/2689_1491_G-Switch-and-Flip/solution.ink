// Translated from solution.cpp.

var N = 200005;

class note
{
  var x: dynamic;
  var y: dynamic;
}

var opt = cpp_array(N);

var t = cpp_array(N);

var a = cpp_array(N);

var n: dynamic;

var cnt: dynamic;

var vis = cpp_array(N);

var ed = cpp_array(N);

var s = cpp_array(N);

var tot: dynamic;

var siz = cpp_array(N);

func Swap(x: dynamic, y: dynamic)
{
  opt[cpp_update(tot, "++")] = [x, y];
  swap(a[x], a[y]);
}

func solve(x: dynamic)
{
  var nn = 0;
  {
    var i = a[x];
    while ((i != x))
    {
      t[cpp_update(nn, "++")] = i;
      i = a[i];
    }
  }
  {
    var i = 1;
    while ((i <= nn))
    {
      Swap(t[i], x);
      i += 1;
    }
  }
}

func merge(x: dynamic, y: dynamic)
{
  Swap(x, y);
  Swap(a[x], a[y]);
  solve(a[x]);
  solve(a[y]);
}

func main()
{
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if ((vis[i] || (a[i] == i)))
      {
        i += 1;
        continue;
      }
      s[cpp_update(cnt, "++")] = i;
      {
        var x = i;
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
    var i = 1;
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
      var i = 1;
      while ((vis[i] == cnt))
      {
        i += 1;
      }
      var v1 = s[cnt];
      var v2 = ed[cnt];
      Swap(v1, i);
      Swap(i, v2);
      Swap(v1, i);
    } else
    {
      var v1 = s[cnt];
      var v2 = a[s[cnt]];
      var vm = ed[cnt];
      Swap(v1, v2);
      Swap(v2, vm);
      solve(v1);
    }
  }
  printf("%d\n", tot);
  {
    var i = 1;
    while ((i <= tot))
    {
      printf("%d %d\n", opt[i].x, opt[i].y);
      i += 1;
    }
  }
}
