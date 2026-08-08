// Translated from solution.cpp.

var q: dynamic;

class node
{
  var l: dynamic;
  var r: dynamic;
  var id: dynamic;
}

var p = cpp_array(100010);

var n: dynamic;

var m: dynamic;

var x: dynamic;

var y: dynamic;

func cmp(a: dynamic, b: dynamic)
{
  return ((cpp_cast(a))->l - (cpp_cast(b))->l);
}

var ans = cpp_array(100010);

func main()
{
  scanf("%d%d", (&n), (&m));
  {
    var i = 1;
    while ((i <= m))
    {
      scanf("%d%d", (&x), (&y));
      p[i].l = ((n + 1) - y);
      p[i].r = x;
      p[i].id = i;
      i += 1;
    }
  }
  qsort((p + 1), m, cpp_sizeof((node)), cmp);
  var j = 1;
  var cnt = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      while (((j <= m) && (p[j].l <= i)))
      {
        q.push(make_pair((-p[j].r), p[j].id));
        j += 1;
      }
      while ((!q.empty()))
      {
        var t = q.top();
        q.pop();
        if (((-t.first) >= i))
        {
          cnt += 1;
          ans[cnt] = t.second;
          break;
        }
      }
      i += 1;
    }
  }
  printf("%d\n", cnt);
  {
    var i = 1;
    while ((i <= cnt))
    {
      printf("%d ", ans[i]);
      i += 1;
    }
  }
}
